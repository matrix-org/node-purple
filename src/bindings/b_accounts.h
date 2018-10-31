#ifndef ACCOUNTS_H_INCLUDED
#define ACCOUNTS_H_INCLUDED

#include <node_api.h>
#include <account.h>
#include "../helper.h"

napi_value _purple_accounts_new(napi_env env, napi_callback_info info);
napi_value _purple_accounts_find(napi_env env, napi_callback_info info);
napi_value _purple_accounts_get_enabled(napi_env env, napi_callback_info info);
napi_value _purple_accounts_set_enabled(napi_env env, napi_callback_info info);
napi_value _purple_accounts_connect(napi_env env, napi_callback_info info);
napi_value _purple_accounts_disconnect(napi_env env, napi_callback_info info);

/* Helpers */
napi_value nprpl_account_create(napi_env env, PurpleAccount *acct);

#endif
