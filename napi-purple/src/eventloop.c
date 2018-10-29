#include "eventloop.h"
/***
 Horrible stiching together of libuv's loop for purple.
***/

#define TIMER_CAPACITY 5000

typedef struct {
    uv_timer_t* handle;
    GSourceFunc function;
    gpointer data;
    uint32_t timerSlot;
} s_evLoopTimer;

typedef struct {
    uint stateN;
    uv_loop_t* loop;
    s_evLoopTimer timers[TIMER_CAPACITY];
    uint32_t timerSlot;
    uv_mutex_t* timerAddMutex;
} s_evLoopState;

static s_evLoopState evLoopState;

void call_callback(uv_timer_t* handle) {
    s_evLoopTimer *timer = handle->data;
    printf("call_callback called timer # %d\n", timer->timerSlot);
    if (!timer->function(timer->data)) {
        printf("Stopped timer #%d", timer->timerSlot);
        uv_timer_stop(timer->handle);
        return;
    }
    printf("Continuing with timer #%d", timer->timerSlot);
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
    uv_mutex_lock(&evLoopState.timerAddMutex);
    printf("timeout_add: interval %d\n", interval);
    uv_timer_t handle;
    uv_timer_init(evLoopState.loop, &handle);
    uint32_t slot = evLoopState.timerSlot;
    s_evLoopTimer timer = {
        &handle,
        function,
        data
    };
    handle.data = &timer;
    evLoopState.timers[slot] = timer;
    evLoopState.timerSlot++;
    if(evLoopState.timerSlot > TIMER_CAPACITY - 1) {
        evLoopState.timerSlot = 0;
    }
    int v = uv_timer_start(&handle, call_callback, interval, interval);
    printf("timeout_add: added timer #%d out:%d\n", evLoopState.timerSlot, v);
    uv_mutex_unlock(&evLoopState.timerAddMutex);
    return slot;
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
    printf("Magic number; %d\n", evLoopState.stateN);
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
    printf("Magic number; %d\n", evLoopState.stateN);
}

/**
* Should remove an input handler.  Analogous to g_source_remove in glib.
* @param handle an identifier, as returned by #input_add.
* @return       @c TRUE if the input handler was found and removed.
* @see purple_input_remove
*/
gboolean input_remove (guint handle) {
    printf("Magic number; %d\n", evLoopState.stateN);
}


static PurpleEventLoopUiOps glib_eventloops =
{
    timeout_add,
	timeout_remove,
    input_add,
    input_remove,
    NULL,//input_get_error
    NULL,//timeout_add_seconds
    NULL,
    NULL,
    NULL,
};

/** End of the eventloop functions. **/
void eventLoop_get(PurpleEventLoopUiOps* opts, napi_env* env) {
    *opts = glib_eventloops;
    uv_loop_t* loop;
    evLoopState.stateN = 52;
    uv_mutex_init(&evLoopState.timerAddMutex);
    if (napi_get_uv_event_loop(*env, &loop) != napi_ok){
        napi_throw_error(*env, NULL, "Could not get UV loop");
    }
    evLoopState.loop = loop;
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
//guint timeout_add_seconds (guint interval, GSourceFunc function,
//	                             gpointer data) {

//}
