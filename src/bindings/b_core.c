#include "b_core.h"

napi_value _purple_core_get_version(napi_env env, napi_callback_info info) {
  const char *version;
  napi_status status;
  napi_value node_version;
  version = purple_core_get_version();

  status = napi_create_string_utf8(env, version, NAPI_AUTO_LENGTH, &node_version);

  if (status != napi_ok) {
    THROW(env, NULL, "Unable to create return value", NULL);
  }

  return node_version;
}

napi_value _purple_core_init(napi_env env, napi_callback_info info) {
  gboolean result;
  napi_value n_result;
  result = purple_core_init("matrix-bridge");

  if (napi_get_boolean(env, result, &n_result) != napi_ok) {
    THROW(env, NULL, "Unable to create return value", NULL);
  }

  return n_result;
}

napi_value _purple_core_quit(napi_env env, napi_callback_info info) {
  purple_core_quit();
  napi_value n_undef;
  napi_get_undefined(env, &n_undef);
  return n_undef;
}
