#ifndef WILL_CORE_H_INCLUDED
#define WILL_CORE_H_INCLUDED

#include <glib.h>
#include <prefs.h>
#include <core.h>
#include <node_api.h>

napi_value _purple_core_get_version(napi_env env, napi_callback_info info);
napi_value _purple_core_init(napi_env env, napi_callback_info info);
napi_value _purple_core_quit(napi_env env, napi_callback_info info);

#endif
