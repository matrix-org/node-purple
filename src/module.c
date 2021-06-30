#define NAPI_VERSION 3
#define NAPI_EXPERIMENTAL
#include <node_api.h>
#include <glib.h>

#include "bindings/b_core.h"
#include "bindings/b_plugins.h"
#include "bindings/b_accounts.h"
#include "bindings/b_buddy.h"
#include "bindings/b_notify.h"
#include "helper.h"
#include "messaging.h"


napi_value Init(napi_env env, napi_value exports) {
  // Future binding should expose a XXX_bind_node(env,exports) function
  // and apply their exported functions to that rather than use a header.
  // The stuff below is mostly wrong.
  napi_value _fn_purple_core_get_version;
  napi_value _fn_purple_core_init;
  napi_value _fn_purple_core_quit;
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

  napi_create_function(env, NULL, 0, setupPurple, NULL, &_func);
  napi_set_named_property(env, ns_helper, "setupPurple", _func);

  napi_create_function(env, NULL, 0, pollEvents, NULL, &_func);
  napi_set_named_property(env, ns_helper, "pollEvents", _func);

  napi_set_named_property(env, exports, "helper", ns_helper);

  /* Create plugins */
  napi_value ns_plugins;
  napi_create_object(env, &ns_plugins);

  napi_create_function(env, NULL, 0, _purple_plugins_get_protocols, NULL, &_func);
  napi_set_named_property(env, ns_plugins, "get_protocols", _func);

  napi_set_named_property(env, exports, "plugins", ns_plugins);


  // This is the right way to do it :)
  accounts_bind_node(env, exports);
  messaging_bind_node(env, exports);
  buddy_bind_node(env, exports);
  notify_bind_node(env, exports);

  return exports;
}


NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
