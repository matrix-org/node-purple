#include "helper.h"

napi_value setupPurple(napi_env env, napi_callback_info info) {
    // Does everything needed to create a purple session.
    // {
    //  debugEnabled: 1|0
    //
    // }
    size_t argc = 0;
    int debugEnabled;
    napi_value argv[1];
    napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    if (argc == 0) {
      napi_throw_error(env, NULL, "setupPurple takes a options object");
    }

    napi_get_value_int32(env, argv[0], &debugEnabled);
    purple_debug_set_enabled(enabled);


    napi_value n_undef;
    napi_get_undefined(env, &n_undef);
    return n_undef;
}
