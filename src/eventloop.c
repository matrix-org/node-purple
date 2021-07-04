#include "eventloop.h"
#include "helper.h"

/*
    Horrible stiching together of libuv's loop for purple.
    This works by getting the node env's loop and adding our own
    timers and polls to it.
*/

// Structure for timers
// One uv_timer_t corresponds to one s_evLoopTimer
typedef struct {
    uv_timer_t* handle;
    GSourceFunc function;
    gpointer data;
} s_evLoopTimer;

/**
 * Structure used to map one uv_poll_t to many events.
 * When the events list is empty, this should be culled. 
 */
typedef struct {
    uv_poll_t* handle;
    int fd;
    int cond;
    // s_evLoopInputEvent
    GList* events;
} s_evLoopInput;

typedef struct {
    PurpleInputFunction func;
    gpointer user_data;
    int events;
    s_evLoopInput* parent;
} s_evLoopInputEvent;

// Global state for the eventloop.
typedef struct {
    uv_loop_t* loop;
    // fd -> s_evLoopInput
    GHashTable* inputs;
} s_evLoopState;

void call_callback(uv_timer_t* handle);

static s_evLoopState evLoopState;

/**
* Should create a callback timer with an interval measured in
* milliseconds.  The supplied @a function should be called every @a
* interval seconds until it returns @c FALSE, after which it should not
* be called again.
*
* Analogous to g_timeout_add in glib.
*
* Note: On Win32, this function may be called from a thread other than
* the libpurple thread.  You should make sure to detect this situation
* and to only call "function" from the libpurple thread.
*
* @param interval the interval in <em>milliseconds</em> between calls
*                 to @a function.
* @param data     arbitrary data to be passed to @a function at each
*                 call.
* @todo Who is responsible for freeing @a data?
*
* @return a handle for the timeout, which can be passed to
*         #timeout_remove.
*
* @see purple_timeout_add
**/
guint timeout_add (guint interval, GSourceFunc function, gpointer data) {
    s_evLoopTimer *timer = malloc(sizeof(s_evLoopTimer));
    uv_timer_t *handle = malloc(sizeof(uv_timer_t));
    uv_timer_init(evLoopState.loop, handle);
    timer->handle = handle;
    timer->function = function;
    timer->data = data;
    uv_handle_set_data(handle, timer);
    uv_timer_start(handle, call_callback, interval, 0);
    return timer;
}

/**
    * If implemented, should create a callback timer with an interval
    * measured in seconds.  Analogous to g_timeout_add_seconds in glib.
    *
    * This allows UIs to group timers for better power efficiency.  For
    * this reason, @a interval may be rounded by up to a second.
    *
    * Implementation of this UI op is optional.  If it's not implemented,
    * calls to purple_timeout_add_seconds() will be serviced by
    * #timeout_add.
    *
    * @see purple_timeout_add_seconds()
    * @since 2.1.0
    **/
guint timeout_add_seconds(guint interval, GSourceFunc function, gpointer data) {
    return timeout_add(interval*1000, function, data);
}

void on_timer_close_complete(uv_handle_t* handle)
{
    free(handle->data);
    free(handle);
}


/**
* Should remove a callback timer.  Analogous to g_source_remove in glib.
* @param handle an identifier for a timeout, as returned by
*               #timeout_add.
* @return       @c TRUE if the timeout identified by @a handle was
*               found and removed.
* @see purple_timeout_remove
*/
gboolean timeout_remove(guint handle) {
    g_return_val_if_fail(handle != NULL, false);
    s_evLoopTimer *timer = handle;
    uv_timer_stop(timer->handle);
    if (!uv_is_closing(timer->handle)) {
        uv_close(timer->handle, on_timer_close_complete);
    }
    return true;
}

void handle_input(uv_poll_t* handle, int status, int events) {
    if (status < 0) {
        printf("handle_input error status %i %s\n", status, uv_strerror(status));
        // XXX: Do we need to do anything if the status is not ok?
    } else if (status > 0) {
        // Unexpected positive status
        printf("handle_input unexpected positive status %i\n");
    }
    int closedFD = -1;
    s_evLoopInput *input = handle->data;
    GList *elem;
    s_evLoopInputEvent *inputEvent;
    for(elem = input->events; elem; elem = elem->next) {
        inputEvent = elem->data;
        if (inputEvent->events & events) {
            inputEvent->func(inputEvent->user_data, inputEvent->parent->fd, events);
        }
    }
    // if (fcntl(input->fd, F_GETFL) < 0 && errno == EBADF) {
    //     printf("FD %i closed (%i), cleaning up\n", input->fd, input->id);
    //     input_remove(handle);
    // }
}


/**
* Should add an input handler.  Analogous to g_io_add_watch_full in
* glib.
*
* @param fd        a file descriptor to watch for events
* @param cond      a bitwise OR of events on @a fd for which @a func
*                  should be called.
* @param func      a callback to fire whenever a relevant event on @a
*                  fd occurs.
* @param user_data arbitrary data to pass to @a fd.
* @return          an identifier for this input handler, which can be
*                  passed to #input_remove.
*
* @see purple_input_add
*/
guint input_add(int fd, PurpleInputCondition cond,
                PurpleInputFunction func, gpointer user_data) {
    g_return_if_fail(cond == 1 || cond == 2);
    g_return_if_fail(fd > 0);

    /**
     * There is some subtle logic to this function. LibUV can only handle
     * one poll handle per FD, but libpurple wants us to be able to support
     * multiple per FD. This means we need to manage multiple inputs for
     * a single handle ourselves.
     */

    // Create a struct to hold THIS event/func pair
    s_evLoopInputEvent *input_event = g_malloc(sizeof(uv_poll_t));
    input_event->events = cond;
    input_event->func = func;
    input_event->user_data = user_data;

    s_evLoopInput *input_handle = g_hash_table_lookup(evLoopState.inputs, &fd);
    if (input_handle == NULL) {
        printf("Creating a new input handler: %i Cond: %i\n", fd, cond);
        input_handle = g_malloc(sizeof(s_evLoopInput));
        input_handle->fd = fd;
        input_handle->handle = g_malloc(sizeof(uv_poll_t));
        input_handle->cond = cond;
        input_handle->events = NULL;
        uv_handle_set_data(input_handle->handle, input_handle);
        uv_poll_init(evLoopState.loop, input_handle->handle, fd);
        g_hash_table_insert(evLoopState.inputs, &input_handle->fd, input_handle);
    } else {
        printf("Re-using input handler: %i Cond: %i\n", fd, cond);
        // Nothing to do, except update the condition on the poll
        input_handle->cond = input_handle->cond | cond;
    }
    // This will update the handle if the cond changed.
    uv_poll_start(input_handle->handle, input_handle->cond, handle_input);
    printf("Started polling with FD %i Cond: %i\n", fd, cond);  
    input_event->parent = input_handle;
    input_handle->events = g_list_append(input_handle->events, input_event);
    return input_event;
}

/**
* Should remove an input handler.  Analogous to g_source_remove in glib.
* @param handle an identifier, as returned by #input_add.
* @return       @c TRUE if the input handler was found and removed.
* @see purple_input_remove
*/
gboolean input_remove (guint handle) {
    g_return_val_if_fail(handle != NULL, false);
    s_evLoopInputEvent *inputEvent = handle;
    s_evLoopInput *input = inputEvent->parent;
    printf("input_remove FD: %i\n", input->fd);
    if (g_list_find(input->events, inputEvent) == NULL) {
    printf("input_remove did not find event for FD: %i\n", input->fd);
        return false;
    }
    input->events = g_list_remove(input->events, inputEvent);
    free(inputEvent);
    if (g_list_length(input->events) > 0) {
        return true;
        // Do not clean up the handle yet.
    }
    uv_poll_stop(input->handle);
    free(input->handle);
    free(input);
    return true;
}

static PurpleEventLoopUiOps glib_eventloops =
{
    timeout_add,
	timeout_remove,
    input_add,
    input_remove,
    NULL,//input_get_error
    timeout_add_seconds,//timeout_add_seconds
    NULL,
    NULL,
    NULL,
};

/** End of the eventloop functions. **/
PurpleEventLoopUiOps* eventLoop_get(napi_env* env) {
    if (evLoopState.loop == NULL){
        // Initiate
        if (napi_get_uv_event_loop(*env, &evLoopState.loop) != napi_ok) {
            THROW(*env, NULL, "Could not get UV loop", NULL);
        }
        evLoopState.inputs = g_hash_table_new_full(g_int_hash, g_int_equal, NULL, NULL);
    }
    return &glib_eventloops;
}


void call_callback(uv_timer_t* handle) {
    s_evLoopTimer *timer = handle->data;
    purple_eventloop_set_ui_ops(&glib_eventloops);
    if (timer->handle == NULL) {
        return;
    }
    gboolean res = timer->function(timer->data);
    // If the function succeeds, continue
    if (!res && !uv_is_closing((uv_handle_t *)timer->handle)) {
        uv_close((uv_handle_t *)timer->handle, on_timer_close_complete);
        return;
    }
    uv_timer_again(handle);
}


/**
    * If implemented, should get the current error status for an input.
    *
    * Implementation of this UI op is optional. Implement it if the UI's
    * sockets or event loop needs to customize determination of socket
    * error status.  If unimplemented, <tt>getsockopt(2)</tt> will be used
    * instead.
    *
    * @see purple_input_get_error
    */
// int input_get_error(int fd, int *error) {

// }
