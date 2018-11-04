#define NAPI_VERSION 3
#define NAPI_EXPERIMENTAL
#include <node_api.h>
#include <glib.h>

#include "bindings/b_core.h"
#include "bindings/b_plugins.h"
#include "bindings/b_accounts.h"
#include "bindings/b_buddy.h"
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

  /* Create accounts */
  napi_value ns_accounts;
  napi_create_object(env, &ns_accounts);

  napi_create_function(env, NULL, 0, _purple_accounts_new, NULL, &_func);
  napi_set_named_property(env, ns_accounts, "new", _func);

  napi_create_function(env, NULL, 0, _purple_accounts_find, NULL, &_func);
  napi_set_named_property(env, ns_accounts, "find", _func);

  napi_create_function(env, NULL, 0, _purple_accounts_get_enabled, NULL, &_func);
  napi_set_named_property(env, ns_accounts, "get_enabled", _func);

  napi_create_function(env, NULL, 0, _purple_accounts_set_enabled, NULL, &_func);
  napi_set_named_property(env, ns_accounts, "set_enabled", _func);

  napi_create_function(env, NULL, 0, _purple_accounts_connect, NULL, &_func);
  napi_set_named_property(env, ns_accounts, "connect", _func);
  napi_create_function(env, NULL, 0, _purple_accounts_disconnect, NULL, &_func);
  napi_set_named_property(env, ns_accounts, "disconnect", _func);
  napi_create_function(env, NULL, 0, _purple_account_is_connected, NULL, &_func);
  napi_set_named_property(env, ns_accounts, "is_connected", _func);
  napi_create_function(env, NULL, 0, _purple_account_is_connecting, NULL, &_func);
  napi_set_named_property(env, ns_accounts, "is_connecting", _func);
  napi_create_function(env, NULL, 0, _purple_account_is_disconnected, NULL, &_func);
  napi_set_named_property(env, ns_accounts, "is_disconnected", _func);
  napi_create_function(env, NULL, 0, _purple_account_get_status_types, NULL, &_func);
  napi_set_named_property(env, ns_accounts, "get_status_types", _func);
  napi_create_function(env, NULL, 0, _purple_account_set_status, NULL, &_func);
  napi_set_named_property(env, ns_accounts, "set_status", _func);

  napi_set_named_property(env, exports, "accounts", ns_accounts);

  /* Create messaging */
  napi_value ns_messaging;
  napi_create_object(env, &ns_messaging);

  napi_create_function(env, NULL, 0, messaging_sendIM, NULL, &_func);
  napi_set_named_property(env, ns_messaging, "sendIM", _func);

  napi_set_named_property(env, exports, "messaging", ns_messaging);

  // This is the right way to do it :)
  buddy_bind_node(env, exports);

  return exports;
}


NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
