#ifndef PLUGINS_H_INCLUDED
#define PLUGINS_H_INCLUDED

#include <glib.h>
#include <plugin.h>
#include <node_api.h>

napi_value _purple_plugins_get_protocols(napi_env env, napi_callback_info info);

#endif
