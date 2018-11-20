#include "b_notify.h"

void get_user_info(napi_env env, napi_callback_info info) {
    PurpleAccount *account;
    size_t argc = 2;
    napi_value opt[2];
    napi_value n_out;
    napi_get_cb_info(env, info, &argc, opt, NULL, NULL);
    if (argc < 2) {
      napi_throw_error(env, NULL, "takes two arguments");
    }
    napi_get_value_external(env, opt[0], (void*)&account);
    PurpleConnection* conn = purple_account_get_connection(account);
    char* who = napi_help_strfromval(env, opt[1]);
    serv_get_info(conn, who);
}

void notify_bind_node(napi_env env,napi_value root) {
    napi_value namespace;
    napi_value func;
    napi_create_object(env, &namespace);

    napi_create_function(env, NULL, 0, get_user_info, NULL, &func);
    napi_set_named_property(env, namespace, "get_user_info", func);

    napi_set_named_property(env, root, "notify", namespace);
}
