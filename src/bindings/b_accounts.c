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

napi_value create_object_from_statustype(napi_env env, PurpleStatusType* statustype) {
    napi_value obj;
    napi_value value;
    napi_create_object(env, &obj);

    /* id */
    napi_create_string_utf8(env, purple_status_type_get_id(statustype), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, obj, "id", value);

    /* name */
    napi_create_string_utf8(env, purple_status_type_get_name(statustype), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, obj, "name", value);

    /* saveable */
    napi_get_boolean(env, purple_status_type_is_saveable(statustype), &value);
    napi_set_named_property(env, obj, "saveable", value);

    /* user_settable */
    napi_get_boolean(env, purple_status_type_is_user_settable(statustype), &value);
    napi_set_named_property(env, obj, "user_settable", value);

    /* independent */
    napi_get_boolean(env, purple_status_type_is_independent(statustype), &value);
    napi_set_named_property(env, obj, "independent", value);

    return obj;
}

PurpleAccount* __getacct(napi_env env, napi_callback_info info) {
    PurpleAccount *account;
    size_t argc = 1;
    napi_value opt;
    napi_get_cb_info(env, info, &argc, &opt, NULL, NULL);
    if (argc < 1) {
      napi_throw_error(env, NULL, "takes one argument");
    }
    napi_get_value_external(env, opt, (void*)&account);
    return account;
}

napi_value _purple_accounts_new(napi_env env, napi_callback_info info) {
    napi_value n_out;
    size_t argc = 3;
    napi_value opts[2];

    napi_get_cb_info(env, info, &argc, opts, NULL, NULL);
    if (argc == 0) {
      napi_throw_error(env, NULL, "new takes two arguments");
    }

    char* name = napi_help_strfromval(env, opts[0]);
    char* prpl = napi_help_strfromval(env, opts[1]);

    PurpleAccount *account = purple_account_new(name, prpl);
    if (account != NULL) {
        purple_accounts_add(account);
    }
    if (argc == 3) {
        char* password = napi_help_strfromval(env, opts[2]);
        purple_account_set_remember_password(account, TRUE);
        purple_account_set_password(account, password);
        free(password);
    }
    n_out = nprpl_account_create(env, account);

    free(name);
    free(prpl);

    return n_out;
}


napi_value _purple_accounts_find(napi_env env, napi_callback_info info) {
    napi_value n_out;
    size_t argc = 2;
    napi_value opts[2];

    napi_get_cb_info(env, info, &argc, opts, NULL, NULL);
    if (argc == 0) {
      napi_throw_error(env, NULL, "find takes two arguments");
    }

    char* name = napi_help_strfromval(env, opts[0]);
    char* prpl = napi_help_strfromval(env, opts[1]);

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

    napi_get_value_external(env, opt, (void*)&account);
    gboolean enabled = purple_account_get_enabled(account, STR_PURPLE_UI);
    napi_get_boolean(env, enabled, &n_out);
    return n_out;
}

void _purple_accounts_set_enabled(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value opts[2];
    PurpleAccount *account;
    napi_get_cb_info(env, info, &argc, opts, NULL, NULL);
    if (argc < 2) {
      napi_throw_error(env, NULL, "set_enabled takes two arguments");
    }

    napi_get_value_external(env, opts[0], (void*)&account);
    gboolean enable;
    napi_get_value_bool(env, opts[1], (void*)&enable);
    purple_account_set_enabled(account, STR_PURPLE_UI, enable);
}

void _purple_accounts_connect(napi_env env, napi_callback_info info) {
    PurpleAccount *account = __getacct(env, info);
    purple_account_connect(account);
}

void _purple_accounts_disconnect(napi_env env, napi_callback_info info) {
    PurpleAccount *account = __getacct(env, info);
    purple_account_disconnect(account);
}

napi_value _purple_account_is_connected(napi_env env, napi_callback_info info) {
    PurpleAccount *account = __getacct(env, info);
    bool res = purple_account_is_connected(account);
    napi_value jres;
    napi_get_boolean(env, res, &jres);
    return jres;
}

napi_value _purple_account_is_connecting(napi_env env, napi_callback_info info) {
    PurpleAccount *account = __getacct(env, info);
    bool res = purple_account_is_connecting(account);
    napi_value jres;
    napi_get_boolean(env, res, &jres);
    return jres;
}

napi_value _purple_account_is_disconnected(napi_env env, napi_callback_info info) {
    PurpleAccount *account = __getacct(env, info);
    bool res = purple_account_is_disconnected(account);
    napi_value jres;
    napi_get_boolean(env, res, &jres);
    return jres;
}

napi_value _purple_account_get_status_types(napi_env env, napi_callback_info info) {
    napi_value status_array;
    GList* status_types;
    PurpleAccount *account = __getacct(env, info);
    napi_create_array(env, &status_array);

    status_types = purple_account_get_status_types(account);
    GList* l;
    uint32_t i = 0;
    for (l = status_types; l != NULL; l = l->next)
    {
        PurpleStatusType *type = (PurpleStatusType*)l->data;
        napi_value obj = create_object_from_statustype(env, type);
        napi_set_element(env, status_array, i, obj);
        i++;
    }
    return status_array;
}

void _purple_account_set_status(napi_env env, napi_callback_info info) {
    PurpleAccount *account;
    char* id;
    bool active;
    size_t argc = 3;
    napi_value opt[3];
    napi_get_cb_info(env, info, &argc, opt, NULL, NULL);
    if (argc < 3) {
      napi_throw_error(env, NULL, "takes three arguments");
    }

    napi_get_value_external(env, opt[0], (void*)&account);
    id = napi_help_strfromval(env, opt[1]);
    napi_get_value_bool(env, opt[2], &active);

    purple_account_set_status(account, id, active, NULL);
}

void accounts_bind_node(napi_env env, napi_value root) {
    napi_value ns_accounts;
    napi_value _func;
    napi_create_object(env, &ns_accounts);

    napi_create_function(env, "new", NAPI_AUTO_LENGTH, _purple_accounts_new, NULL, &_func);
    napi_set_named_property(env, ns_accounts, "new", _func);

    napi_create_function(env, "configure", NAPI_AUTO_LENGTH, _purple_account_configure, NULL, &_func);
    napi_set_named_property(env, ns_accounts, "configure", _func);

    napi_create_function(env, "find", NAPI_AUTO_LENGTH, _purple_accounts_find, NULL, &_func);
    napi_set_named_property(env, ns_accounts, "find", _func);

    napi_create_function(env, "get_enabled", NAPI_AUTO_LENGTH, _purple_accounts_get_enabled, NULL, &_func);
    napi_set_named_property(env, ns_accounts, "get_enabled", _func);

    napi_create_function(env, "set_enabled", NAPI_AUTO_LENGTH, _purple_accounts_set_enabled, NULL, &_func);
    napi_set_named_property(env, ns_accounts, "set_enabled", _func);

    napi_create_function(env, "connect", NAPI_AUTO_LENGTH, _purple_accounts_connect, NULL, &_func);
    napi_set_named_property(env, ns_accounts, "connect", _func);

    napi_create_function(env, "disconnect", NAPI_AUTO_LENGTH, _purple_accounts_disconnect, NULL, &_func);
    napi_set_named_property(env, ns_accounts, "disconnect", _func);

    napi_create_function(env, "is_connected", NAPI_AUTO_LENGTH, _purple_account_is_connected, NULL, &_func);
    napi_set_named_property(env, ns_accounts, "is_connected", _func);

    napi_create_function(env, "is_connecting", NAPI_AUTO_LENGTH, _purple_account_is_connecting, NULL, &_func);
    napi_set_named_property(env, ns_accounts, "is_connecting", _func);

    napi_create_function(env, "is_disconnected", NAPI_AUTO_LENGTH, _purple_account_is_disconnected, NULL, &_func);
    napi_set_named_property(env, ns_accounts, "is_disconnected", _func);

    napi_create_function(env, "get_status_types", NAPI_AUTO_LENGTH, _purple_account_get_status_types, NULL, &_func);
    napi_set_named_property(env, ns_accounts, "get_status_types", _func);

    napi_create_function(env, "set_status", NAPI_AUTO_LENGTH, _purple_account_set_status, NULL, &_func);
    napi_set_named_property(env, ns_accounts, "set_status", _func);

    napi_set_named_property(env, root, "buddy", ns_accounts);
}