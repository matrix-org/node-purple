#include "b_accounts.h"

napi_value nprpl_account_create(napi_env env, PurpleAccount *acct){
    napi_value obj;
    napi_value value;
    napi_create_object(env, &obj);
    /* username */
    napi_create_string_utf8(env, acct->username, NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, obj, "username", value);
    /* alias */
    if (acct->alias != NULL) {
        napi_create_string_utf8(env, acct->alias, NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, obj, "alias", value);
    }

    /* password */
    if (acct->password != NULL) {
        napi_create_string_utf8(env, acct->password, NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, obj, "password", value);
    }
    /* user_info */
    if (acct->user_info != NULL) {
        napi_create_string_utf8(env, acct->user_info, NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, obj, "user_info", value);
    }
    /* buddy_icon_path */
    if (acct->buddy_icon_path != NULL) {
        napi_create_string_utf8(env, acct->buddy_icon_path, NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, obj, "buddy_icon_path", value);
    }
    /* protocol_id */
    napi_create_string_utf8(env, acct->protocol_id, NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, obj, "protocol_id", value);

    /* handle */
    napi_create_external(env, acct, NULL, NULL, &value);
    napi_set_named_property(env, obj, "handle", value);

    return obj;
}

napi_value _purple_accounts_new(napi_env env, napi_callback_info info) {
    napi_value n_out;
    size_t argc = 2;
    napi_value opts[2];

    napi_get_cb_info(env, info, &argc, &opts, NULL, NULL);
    if (argc == 0) {
      napi_throw_error(env, NULL, "new takes two arguments");
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

    PurpleAccount *account = purple_account_new(name, prpl);
    n_out = nprpl_account_create(env, account);
    free(name);
    free(prpl);

    return n_out;
}


napi_value _purple_accounts_find(napi_env env, napi_callback_info info) {
    napi_value n_out;
    size_t argc = 2;
    napi_value opts[2];

    napi_get_cb_info(env, info, &argc, &opts, NULL, NULL);
    if (argc == 0) {
      napi_throw_error(env, NULL, "find takes two arguments");
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

    PurpleAccount *account = purple_accounts_find(name, prpl);
    if (account == false) {
        napi_get_undefined(env, &n_out);
    } else {
        n_out = nprpl_account_create(env, account);
    }
    free(name);
    free(prpl);

    return n_out;
}

napi_value _purple_accounts_get_enabled(napi_env env, napi_callback_info info) {
    napi_value n_out;
    size_t argc = 1;
    napi_value opt;
    PurpleAccount *account;
    napi_get_cb_info(env, info, &argc, &opt, NULL, NULL);
    if (argc < 1) {
      napi_throw_error(env, NULL, "get_enabled takes one argument");
    }

    napi_get_value_external(env, opt, &account);
    gboolean enabled = purple_account_get_enabled(account, STR_PURPLE_UI);
    napi_get_boolean(env, enabled, &n_out);
    return n_out;
}

napi_value _purple_accounts_set_enabled(napi_env env, napi_callback_info info) {
    napi_value n_out;
    size_t argc = 2;
    napi_value opts[2];
    PurpleAccount *account;
    napi_get_cb_info(env, info, &argc, &opts, NULL, NULL);
    if (argc < 2) {
      napi_throw_error(env, NULL, "set_enabled takes two arguments");
    }

    napi_get_value_external(env, opts[0], &account);
    gboolean enable;
    napi_get_value_bool(env, opts[1], &enable);
    purple_account_set_enabled(account, STR_PURPLE_UI, enable);
    return n_out;
}
