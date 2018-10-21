#include <node_api.h>
#include <glib.h>
#include <core.h>
#include <stdio.h>

#include "bindings/core.c"


napi_value Init(napi_env env, napi_value exports) {
  napi_value _fn_purple_core_get_version;
  napi_status status;


  napi_create_function(env, NULL, 0, _purple_core_get_version, NULL, &_fn_purple_core_get_version);
  napi_set_named_property(env, exports, "purple_core_get_version", _fn_purple_core_get_version);

  return exports;
}


// napi_value MyFunction(napi_env env, napi_callback_info info) {
//   napi_status status;
//   size_t argc = 1;
//   int number = 0;
//   napi_value argv[1];
//   status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
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


NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
