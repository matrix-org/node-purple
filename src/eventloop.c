#include "eventloop.h"
/***
 Horrible stiching together of libuv's loop for purple.
***/

typedef struct {
    uint32_t id;
    uv_timer_t* handle;
    GSourceFunc function;
    gpointer data;
} s_evLoopTimer;

typedef struct {
    uint32_t id;
    uv_poll_t* handle;
    int fd;
    uint32_t* openPollCounter;
    int operations;
    PurpleInputFunction func;
    gpointer user_data;
} s_evLoopInput;

typedef struct {
    uv_loop_t* loop;
    GList *timers; /* s_evLoopTimer */
    uint32_t timerId;
    GList *inputs; /* s_evLoopInput */
    uint32_t inputId;
} s_evLoopState;

void call_callback(uv_timer_t* handle);

static s_evLoopState evLoopState;

gint findTimerById(gconstpointer item, gconstpointer id) {
    s_evLoopTimer timer = *(s_evLoopTimer*)item;
    if (timer.id == *(uint32_t*)id) {
        return 0;
    } else {
        return 1;
    }
}

gint findInputById(gconstpointer item, gconstpointer id) {
    s_evLoopInput input = *(s_evLoopInput*)item;
    if (input.id == *(uint32_t*)id) {
        return 0;
    } else {
        return 1;
    }
}

gint findInputByFd(gconstpointer item, gconstpointer fd) {
    s_evLoopInput input = *(s_evLoopInput*)item;
    if (input.fd == *(int*)fd) {
        return 0;
    } else {
        return 1;
    }
}

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
    uv_timer_t *handle = malloc(sizeof(uv_timer_t));
    s_evLoopTimer *timer = malloc(sizeof(s_evLoopTimer));
    uint32_t id = evLoopState.timerId;
    uv_timer_init(evLoopState.loop, handle);
    timer->handle = handle;
    timer->function = function;
    timer->data = data;
    timer->id = id;
    handle->data = timer;
    evLoopState.timers = g_list_append(evLoopState.timers, timer);
    uv_timer_start(handle, call_callback, interval, 0);
    evLoopState.timerId++;
    return id;
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

/**
* Should remove a callback timer.  Analogous to g_source_remove in glib.
* @param handle an identifier for a timeout, as returned by
*               #timeout_add.
* @return       @c TRUE if the timeout identified by @a handle was
*               found and removed.
* @see purple_timeout_remove
*/
gboolean timeout_remove(guint handle) {
    GList* timerListItem = g_list_find_custom(evLoopState.timers, &handle, findTimerById);
    if (timerListItem == NULL) {
        return false;
    }
    s_evLoopTimer *timer = (s_evLoopTimer*)timerListItem->data;
    if (timer->handle == NULL) {
        return false;
    }
    uv_timer_stop(timer->handle);
    evLoopState.timers = g_list_remove(evLoopState.timers, timer);
    printf("free(timer)");
    free(timer);
    return true;
}

void handle_input(uv_poll_t* handle, int status, int events) {
    GList *l;
    if (status != 0) {
        printf("handle_input reported a not ok status: %s", uv_err_name(status));
    }
    for (l = evLoopState.inputs; l != NULL; l = l->next)
    {
        s_evLoopInput *input = (s_evLoopInput*)l->data;
        if (input->handle != handle) {
            continue;
        }
        if (input->operations & 1 && events & 1) {
            // Read
            input->func(input->user_data, input->fd, events);
        }
        if (input->operations & 2 && events & 2) {
            // Write
            input->func(input->user_data, input->fd, events);
        }
    }
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

    uv_poll_t *poll_handle;
    uint32_t *pollsOpen;

    // Iterate over existing inputs to see if a handle exists.
    GList* inputListItem = g_list_find_custom(evLoopState.inputs, &fd, findInputByFd);
    if (inputListItem == NULL) {
        // We don't have an input handler for that FD yet.
        poll_handle = malloc(sizeof(uv_poll_t));
        uv_poll_init(evLoopState.loop, poll_handle, fd);
        pollsOpen = malloc(sizeof(uint32_t));
        *pollsOpen = 0;
    } else {
        s_evLoopInput *previous_handle = (s_evLoopInput*)inputListItem->data;
        poll_handle = previous_handle->handle;
        pollsOpen = previous_handle->openPollCounter;
    }

    // Create the input handle.
    s_evLoopInput *input_handle = malloc(sizeof(s_evLoopInput));
    input_handle = malloc(sizeof(s_evLoopInput));
    input_handle->fd = fd;
    input_handle->handle = poll_handle;
    input_handle->operations = cond;
    input_handle->user_data = user_data;
    input_handle->func = func;
    input_handle->id = evLoopState.inputId;
    (*pollsOpen)++;
    input_handle->openPollCounter = pollsOpen;
    evLoopState.inputId++;

    // Insert the handler.
    evLoopState.inputs = g_list_append(evLoopState.inputs, input_handle);

    // enum has 1=readable, 2=writable, which coincidentally matches libpurple's PurpleInputCondition...
    uv_poll_start(poll_handle, (int)cond, handle_input);
    return evLoopState.inputId - 1;
}

/**
* Should remove an input handler.  Analogous to g_source_remove in glib.
* @param handle an identifier, as returned by #input_add.
* @return       @c TRUE if the input handler was found and removed.
* @see purple_input_remove
*/
gboolean input_remove (guint handle) {
    GList* timerListItem = g_list_find_custom(evLoopState.inputs, &handle, findInputById);
    if (timerListItem == NULL) {
        // We don't have an input handler for that FD.
        return false;
    }
    s_evLoopInput *input = (s_evLoopInput*)timerListItem->data;
    (*input->openPollCounter)--;
    evLoopState.inputs = g_list_remove(evLoopState.inputs, input);
    printf("Removing a input for %d", input->fd);
    if (*input->openPollCounter > 0) {
        printf("free(input);");
        free(input);
        return true;
    }
    uv_poll_stop(input->handle);
    // XXX: This might be the wrong thing to do here, so commented out.
    //      Hopefully a libuv/c veteran could clear this up, pun not intended :p.
    //free(input->handle);
    //free(input);
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
    uv_loop_t* loop;
    if (evLoopState.loop == NULL && napi_get_uv_event_loop(*env, &loop) != napi_ok){
        napi_throw_error(*env, NULL, "Could not get UV loop");
    }
    evLoopState.loop = loop;
    return &glib_eventloops;
}


void call_callback(uv_timer_t* handle) {
    s_evLoopTimer *timer = handle->data;
    purple_eventloop_set_ui_ops(&glib_eventloops);
    if (timer->handle == NULL) {
        return;
    }
    gboolean res = timer->function(timer->data);
    if (!res) {
        printf("free(handle->data);");
        free(handle->data);
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

