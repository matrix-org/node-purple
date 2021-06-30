#ifndef ACCOUNTS_H_INCLUDED
#define ACCOUNTS_H_INCLUDED

#include <node_api.h>
#include <account.h>
#include <status.h>
#include <prpl.h>
#include "../helper.h"
#include "../napi_helpers.h"

napi_value nprpl_account_create(napi_env env, PurpleAccount *acct);
void accounts_bind_node(napi_env env,napi_value root);

#endif
