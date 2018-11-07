#include "b_buddy.h"

napi_value nprpl_buddy_create(napi_env env, PurpleBuddy* buddy) {
    napi_value obj;
    napi_value value;
    napi_create_object(env, &obj);
    const char* icon_filepath;
    const char* nick;
    /* name */
    napi_create_string_utf8(env, purple_buddy_get_name(buddy), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, obj, "name", value);

    /* icon */
    PurpleBuddyIcon* icon = purple_buddy_get_icon(buddy);

    if (icon) {
        icon_filepath = purple_buddy_icon_get_full_path(icon);
    } else {
        // Attempt to load it from the store.
        icon_filepath = g_build_filename(
            purple_buddy_icons_get_cache_dir(),
            purple_blist_node_get_string((PurpleBlistNode*)buddy, "buddy_icon"),
            NULL
        );
    }

    if (icon_filepath != NULL) {
        napi_create_string_utf8(env, icon_filepath, NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, obj, "icon_path", value);
    }

    /* status */
    PurpleStatus* status = purple_presence_get_active_status(
                purple_buddy_get_presence(buddy));
    if (status != NULL) {
        napi_create_string_utf8(env, purple_status_get_id(status), NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, obj, "status_id", value);

        nick = purple_status_get_attr_string(status, "nick");
        if (nick == NULL) {
            // We might have saved the nick previously
            nick = purple_blist_node_get_string((PurpleBlistNode*)buddy, "servernick");
        }
        if (nick != NULL) {
            napi_create_string_utf8(env, nick, NAPI_AUTO_LENGTH, &value);
            napi_set_named_property(env, obj, "nick", value);
        }
    }


    return obj;
}


napi_value _buddy_find(napi_env env, napi_callback_info info) {
    PurpleAccount *account;
    PurpleBuddy* buddy;
    size_t argc = 2;
    napi_value opt[2];
    napi_value n_out;
    napi_get_cb_info(env, info, &argc, opt, NULL, NULL);
    if (argc < 2) {
      napi_throw_error(env, NULL, "takes two arguments");
    }
    napi_get_value_external(env, opt[0], (void*)&account);
    char* name = napi_help_strfromval(env, opt[1]);

    buddy = purple_find_buddy(account, name);
    free(name);
    if (buddy != NULL) {
        n_out = nprpl_buddy_create(env, buddy);
    } else {
        napi_get_undefined(env, &n_out);
    }
    return n_out;
}

void buddy_bind_node(napi_env env,napi_value root) {
    napi_value namespace;
    napi_value func;
    napi_create_object(env, &namespace);

    napi_create_function(env, NULL, 0, _buddy_find, NULL, &func);
    napi_set_named_property(env, namespace, "find", func);

    napi_set_named_property(env, root, "buddy", namespace);
}
