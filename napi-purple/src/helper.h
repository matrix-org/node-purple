#ifndef HELPER_H_INCLUDED
#define HELPER_H_INCLUDED

#include <glib.h>
#include <prefs.h>
#include <core.h>
#include <util.h>
#include <debug.h>
#include <conversation.h>
#include <eventloop.h>
#include <node_api.h>
#include "eventloop.h"

//const char* STR_PURPLE_UI = "matrix-bridge";

napi_value setupPurple(napi_env env, napi_callback_info info);

#endif
