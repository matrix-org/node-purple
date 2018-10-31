#include "b_accounts.h"

napi_value _purple_accounts_new(napi_env env, napi_callback_info info) {
    napi_value n_undef;
    napi_get_undefined(env, &n_undef);
    size_t argc = 2;
    napi_value opts[2];

    napi_get_cb_info(env, info, &argc, &opts, NULL, NULL);
    if (argc == 0) {
      napi_throw_error(env, NULL, "setupPurple takes a options object");
    }

    size_t length;

    napi_get_value_string_utf8(env, opts[0], NULL, NULL, &length);
    length++; //Null terminator
    char* name = malloc(sizeof(char)* length);
    napi_get_value_string_utf8(env, opts[0], name, length, NULL);

    napi_get_value_string_utf8(env, opts[1], NULL, NULL, &length);
    length++; //Null terminator
    char* prpl = malloc(sizeof(char)* length);
    napi_get_value_string_utf8(env, opts[1], prpl, length, NULL);

    PurpleEventLoopUiOps *setEventLoop = purple_eventloop_get_ui_ops();
    PurpleAccount *account = purple_account_new(name, prpl);

    free(name);
    free(prpl);

    return n_undef;
}
