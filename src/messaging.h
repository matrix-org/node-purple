#ifndef MESSAGING_H_INCLUDED
#define MESSAGING_H_INCLUDED

#include <conversation.h>
#include <account.h>
#include <connection.h>
#include <server.h>
#include <node_api.h>
#include <prpl.h>
#include "napi_helpers.h"

void messaging_bind_node(napi_env env,napi_value root);
#endif
