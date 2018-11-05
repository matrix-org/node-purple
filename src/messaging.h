#ifndef MESSAGING_H_INCLUDED
#define MESSAGING_H_INCLUDED

#include <conversation.h>
#include <account.h>
#include <node_api.h>

void messaging_sendIM(napi_env env, napi_callback_info info);

#endif
