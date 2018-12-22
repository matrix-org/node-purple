#ifndef HELPER_H_INCLUDED
#define HELPER_H_INCLUDED

#define STR_PURPLE_UI "matrix-bridge"

#include <glib.h>
#include <prefs.h>
#include <core.h>
#include <debug.h>
#include <conversation.h>
#include <eventloop.h>
#include <node_api.h>
#include "eventloop.h"
#include "signalling.h"

napi_value setupPurple(napi_env env, napi_callback_info info);
napi_value pollEvents(napi_env env, napi_callback_info info);

#endif
