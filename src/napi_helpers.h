#ifndef NAPI_HELPERS_H_INCLUDED
#define NAPI_HELPERS_H_INCLUDED

#include <node_api.h>
#include <stdlib.h>
#include <conversation.h>

char* napi_help_strfromval(napi_env env, napi_value opt);
napi_value nprpl_conv_create(napi_env env, PurpleConversation *conv);

#endif
