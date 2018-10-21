#include <node_api.h>
#include <glib.h>
#include <core.h>
#include <stdio.h>

napi_value _purple_core_get_version(napi_env env, napi_callback_info info) {
  char *version;
  napi_status status;
  napi_value node_version;
  version = purple_core_get_version();
  printf("String is: %s\n", version);
  printf("String is: %s\n", "A string");

  status = napi_create_string_utf8(env, version, NAPI_AUTO_LENGTH, &node_version);

  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to create return value");
  }

  return node_version;
}
