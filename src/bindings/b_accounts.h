#ifndef ACCOUNTS_H_INCLUDED
#define ACCOUNTS_H_INCLUDED

#include <node_api.h>
#include <account.h>
#include <status.h>
#include <prpl.h>
#include "../helper.h"

napi_value _purple_accounts_new(napi_env env, napi_callback_info info);
napi_value _purple_accounts_find(napi_env env, napi_callback_info info);
napi_value _purple_accounts_get_enabled(napi_env env, napi_callback_info info);
napi_value _purple_accounts_set_enabled(napi_env env, napi_callback_info info);
void _purple_accounts_connect(napi_env env, napi_callback_info info);
void _purple_accounts_disconnect(napi_env env, napi_callback_info info);
napi_value _purple_account_is_connected(napi_env env, napi_callback_info info);
napi_value _purple_account_is_connecting(napi_env env, napi_callback_info info);
napi_value _purple_account_is_disconnected(napi_env env, napi_callback_info info);
napi_value _purple_account_get_status_types(napi_env env, napi_callback_info info);
void _purple_account_set_status(napi_env env, napi_callback_info info);

/* Helpers */
napi_value nprpl_account_create(napi_env env, PurpleAccount *acct);

#endif
