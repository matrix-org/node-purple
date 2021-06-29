#ifndef NOTIFY_H_INCLUDED
#define NOTIFY_H_INCLUDED

#include <node_api.h>
#include <account.h>
#include <blist.h>
#include <buddyicon.h>
#include <status.h>
#include "../napi_helpers.h"

void notify_bind_node(napi_env env,napi_value root);

#endif
