#ifndef EVENTLOOP_H_INCLUDED
#define EVENTLOOP_H_INCLUDED

#include <eventloop.h>
#include <glib.h>
#include <uv.h>
#include <node_api.h>

void eventLoop_get(PurpleEventLoopUiOps* opts, napi_env* env);

#endif