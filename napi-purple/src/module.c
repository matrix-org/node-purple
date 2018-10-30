#define NAPI_VERSION 3
#include <node_api.h>
#include <glib.h>

#include "bindings/b_core.h"
#include "bindings/b_plugins.h"
#include "helper.h"


napi_value Init(napi_env env, napi_value exports) {
  napi_value _fn_purple_core_get_version;
  napi_value _fn_purple_core_init;
  napi_value _fn_purple_core_quit;
  napi_value _fn_setupPurple;
  napi_value _func;

  /* Create core */
  napi_value ns_core;
  napi_create_object(env, &ns_core);

  napi_create_function(env, NULL, 0, _purple_core_get_version, NULL, &_fn_purple_core_get_version);
  napi_set_named_property(env, ns_core, "get_version", _fn_purple_core_get_version);

  napi_create_function(env, NULL, 0, _purple_core_init, NULL, &_fn_purple_core_init);
  napi_set_named_property(env, ns_core, "init", _fn_purple_core_init);

  napi_create_function(env, NULL, 0, _purple_core_quit, NULL, &_fn_purple_core_quit);
  napi_set_named_property(env, ns_core, "quit", _fn_purple_core_quit);

  napi_set_named_property(env, exports, "core", ns_core);

  /* Create debug */
  napi_value ns_debug;
  napi_create_object(env, &ns_debug);

  napi_set_named_property(env, exports, "debug", ns_debug);

  /* Create helper */
  napi_value ns_helper;
  napi_create_object(env, &ns_helper);

  napi_create_function(env, NULL, 0, setupPurple, NULL, &_fn_setupPurple);
  napi_set_named_property(env, ns_helper, "setupPurple", _fn_setupPurple);

  napi_set_named_property(env, exports, "helper", ns_helper);

  /* Create plugins */
  napi_value ns_plugins;
  napi_create_object(env, &ns_plugins);

  napi_create_function(env, NULL, 0, _purple_plugins_get_protocols, NULL, &_func);
  napi_set_named_property(env, ns_plugins, "get_protocols", _func);

  napi_set_named_property(env, exports, "plugins", ns_plugins);

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
