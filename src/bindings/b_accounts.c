#include "b_accounts.h"

// TODO: Taken from libpurple/account.c
typedef struct
{
	PurplePrefType type;

	char *ui;

	union
	{
		int integer;
		char *string;
		gboolean boolean;

	} value;

} PurpleAccountSetting;

napi_status account_settings_to_object(napi_env env, napi_value value, GHashTable *hashTable) {
    napi_status status = napi_generic_failure;
    GHashTableIter iter;
    gpointer key, val;
    napi_value jkey, jvalue;

    g_hash_table_iter_init (&iter, hashTable);
    while (g_hash_table_iter_next (&iter, &key, &val))
    {
        char* skey = (char*)key;
        PurpleAccountSetting* sval = (PurpleAccountSetting*) val;

        if (sval->type == PURPLE_PREF_STRING) {
            printf("sValue: %s\n", sval->value.string);
            status = napi_create_string_utf8(env, sval->value.string, NAPI_AUTO_LENGTH, &jvalue);
            if (status != napi_ok) return status;
        } else if (sval->type == PURPLE_PREF_BOOLEAN) {
            printf("bValue: %i\n", sval->value.boolean);
            status = napi_get_boolean(env, sval->value.boolean, &jvalue);
            if (status != napi_ok) return status;
        } else if (sval->type == PURPLE_PREF_INT) {
            printf("iValue: %i\n", sval->value.integer);
            status = napi_create_int32(env, sval->value.integer, &jvalue);
            if (status != napi_ok) return status;
        } else {
            printf("Undefined");
            status = napi_get_undefined(env, &jvalue);
            if (status != napi_ok) return status;
        }

        printf("Set: %s\n", skey);
        status = napi_create_string_utf8(env, skey, NAPI_AUTO_LENGTH, &jkey);
        if (status != napi_ok) return status;
        status = napi_set_property(env, value, jkey, jvalue);
        if (status != napi_ok) return status;
    }

    return status;
}

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
    /* settings */
    if (acct->settings != NULL) {
        napi_create_object(env, &value);
        account_settings_to_object(env, value, acct->settings);
        napi_set_named_property(env, obj, "settings", value);
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
        return NULL;
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
        return NULL;
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

/**
 * Configure an account with a given object, calling purple_account_set_* for
 * string, int and boolean value types.
 * This will throw if the account does not exist, or if the configuration object was invalid.
 * This operation MAY partially succeed.
 */
napi_value _purple_account_configure(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value opts[2];
    PurpleAccount *account;

    if (napi_get_cb_info(env, info, &argc, opts, NULL, NULL) != napi_ok) {
        napi_throw_error(env, NULL, "napi_get_cb_info failed");
        return NULL;
    }
    if (argc < 2) {
        napi_throw_error(env, NULL, "get_enabled takes two arguments");
        return NULL;
    }

    napi_get_value_external(env, opts[0], (void*)&account);

    napi_value config_object = opts[1];

    napi_value nComponentNames;
    napi_value jkey;
    napi_value jvalue;
    napi_valuetype type;
    char* key;

    unsigned int length;
    if (napi_get_property_names(env, config_object, &nComponentNames) != napi_ok ||
        napi_get_array_length(env, nComponentNames, &length) != napi_ok) {
      napi_throw_error(env, NULL, "passed config option not an object");
      return NULL;
    }
    for (unsigned int i = 0; i < length; i++) {
        napi_get_element(env, nComponentNames, i, &jkey);
        napi_get_property(env, config_object, jkey, &jvalue);
        napi_typeof(env, jvalue, &type);
        key = napi_help_strfromval(env, jkey);
        char error[256];
        switch (type)
        {
            case napi_string:
                key = napi_help_strfromval(env, jkey);
                char* svalue = napi_help_strfromval(env, jvalue);
                if (strcmp(key, "password") == 0) {
                    purple_account_set_password(account, svalue);
                } else if (strcmp(key, "username") == 0) {
                    purple_account_set_username(account, svalue);
                } else {
                    purple_account_set_string(account, key, svalue);
                }
                break;
            case napi_bigint:
                key = napi_help_strfromval(env, jkey);
                int ivalue;
                // Technically this could be larger, but it's highly unlikely we'd need larger.
                if (napi_get_value_int32(env, jvalue, &ivalue) == napi_ok) {
                    purple_account_set_int(account, key, ivalue);
                } else {
                    napi_throw_error(env, NULL, "Could not cooerce bitint value into int32");
                    return NULL;
                }
                break;
            case napi_boolean:
                key = napi_help_strfromval(env, jkey);
                gboolean bvalue;
                if (napi_get_value_bool(env, jvalue, &bvalue) == napi_ok) {
                    purple_account_set_bool(account, key, bvalue);
                } else {
                    napi_throw_error(env, NULL, "Could not cooerce JS boolean value into boolean");
                    return NULL;
                }
                break;
            default:
                sprintf(error, "Cannot handle type for %s", key);
                napi_throw_error(env, NULL, error);
                return NULL;
        }
        free(key);
    }

    return NULL;
}

napi_value _purple_accounts_find(napi_env env, napi_callback_info info) {
    napi_value n_out;
    size_t argc = 2;
    napi_value opts[2];

    napi_get_cb_info(env, info, &argc, opts, NULL, NULL);
    if (argc == 0) {
        napi_throw_error(env, NULL, "find takes two arguments");
        return NULL;
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

napi_value _purple_accounts_get_all(napi_env env, napi_callback_info info) {
    napi_value account_array;
    napi_create_array(env, &account_array);
    GList* accounts = purple_accounts_get_all();
    GList* l;
    uint32_t i = 0;
    for (l = accounts; l != NULL; l = l->next)
    {
        PurpleAccount *account = (PurpleAccount*)l->data;
        napi_value obj = nprpl_account_create(env, account);
        napi_set_element(env, account_array, i, obj);
        i++;
    }
    return account_array;
}

napi_value _purple_accounts_get_enabled(napi_env env, napi_callback_info info) {
    napi_value n_out;
    size_t argc = 1;
    napi_value opt;
    PurpleAccount *account;
    napi_get_cb_info(env, info, &argc, &opt, NULL, NULL);
    if (argc < 1) {
        napi_throw_error(env, NULL, "get_enabled takes one argument");
        return NULL;
    }

    napi_get_value_external(env, opt, (void*)&account);
    gboolean enabled = purple_account_get_enabled(account, STR_PURPLE_UI);
    napi_get_boolean(env, enabled, &n_out);
    return n_out;
}

napi_value _purple_accounts_set_enabled(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value opts[2];
    PurpleAccount *account;
    napi_get_cb_info(env, info, &argc, opts, NULL, NULL);
    if (argc < 2) {
        napi_throw_error(env, NULL, "set_enabled takes two arguments");
        return NULL;
    }

    napi_get_value_external(env, opts[0], (void*)&account);
    gboolean enable;
    napi_get_value_bool(env, opts[1], (void*)&enable);
    purple_account_set_enabled(account, STR_PURPLE_UI, enable);

    return NULL;
}

napi_value _purple_accounts_connect(napi_env env, napi_callback_info info) {
    PurpleAccount *account = __getacct(env, info);
    purple_account_connect(account);

    return NULL;
}

napi_value _purple_accounts_disconnect(napi_env env, napi_callback_info info) {
    PurpleAccount *account = __getacct(env, info);
    purple_account_disconnect(account);

    return NULL;
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

napi_value _purple_account_set_status(napi_env env, napi_callback_info info) {
    PurpleAccount *account;
    char* id;
    bool active;
    size_t argc = 3;
    napi_value opt[3];
    napi_get_cb_info(env, info, &argc, opt, NULL, NULL);
    if (argc < 3) {
        napi_throw_error(env, NULL, "takes three arguments");
        return NULL;
    }

    napi_get_value_external(env, opt[0], (void*)&account);
    id = napi_help_strfromval(env, opt[1]);
    napi_get_value_bool(env, opt[2], &active);

    purple_account_set_status(account, id, active, NULL);

    return NULL;
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

    napi_create_function(env, "get_all", NAPI_AUTO_LENGTH, _purple_accounts_get_all, NULL, &_func);
    napi_set_named_property(env, ns_accounts, "get_all", _func);

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

    napi_set_named_property(env, root, "accounts", ns_accounts);
}
