#include "b_debug.h"

napi_value _purple_debug_set_enabled(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    int enabled;
    napi_value argv[1];
    napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    napi_get_value_int32(env, argv[0], &enabled);
    purple_debug_set_enabled(enabled);

    napi_value n_undef;
    napi_get_undefined(env, &n_undef);
    return n_undef;
}

// napi_value MyFunction(napi_env env, napi_callback_info info) {
//   napi_status status;
//   size_t argc = 1;
//   int number = 0;
//   napi_value argv[1];
//   status =
//
//   if (status != napi_ok) {
//     napi_throw_error(env, NULL, "Failed to parse arguments");
//   }
//
//   status = napi_get_value_int32(env, argv[0], &number);
//
//   if (status != napi_ok) {
//     napi_throw_error(env, NULL, "Invalid number was passed as argument");
//   }
//   napi_value myNumber;
//   number = number * 2;
//   status = napi_create_int32(env, number, &myNumber);
//
//   if (status != napi_ok) {
//     napi_throw_error(env, NULL, "Unable to create return value");
//   }
//
//   return myNumber;
// }
