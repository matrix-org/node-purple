#include "messaging.h"
#include "helper.h"

napi_value messaging_sendIM(napi_env env, napi_callback_info info) {
    PurpleAccount* account;
    size_t argc = 3;
    napi_value opts[3];
    char* name;
    char* body;

    napi_get_cb_info(env, info, &argc, opts, NULL, NULL);
    if (argc < 3) {
        THROW(env, NULL, "sendIM takes three arguments", NULL);
    }

    // Get the account
    napi_get_value_external(env, opts[0], (void*)&account);

    if (account == NULL) {
        THROW(env, NULL, "account is null", NULL);
    }

    name = napi_help_strfromval(env, opts[1]);

    const PurpleConversation* conv = purple_find_conversation_with_account(
        PURPLE_CONV_TYPE_IM,
        name,
        account
    );

    if (conv == NULL) {
        // Create one.
        conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, account, name);
    }
    // Get the IM
    PurpleConvIm* convIm = purple_conversation_get_im_data(conv);

    body = napi_help_strfromval(env, opts[2]);

    purple_conv_im_send(convIm, body);
    free(name);
    free(body);

    return NULL;
}

napi_value messaging_sendChat(napi_env env, napi_callback_info info) {
    PurpleAccount* account;
    size_t argc = 3;
    napi_value opts[3];
    char* name;
    char* body;

    napi_get_cb_info(env, info, &argc, opts, NULL, NULL);
    if (argc < 3) {
        THROW(env, NULL, "sendIM takes three arguments", NULL);
    }

    // Get the account
    napi_get_value_external(env, opts[0], (void*)&account);

    if (account == NULL) {
        THROW(env, NULL, "account is null", NULL);
    }

    name = napi_help_strfromval(env, opts[1]);

    const PurpleConversation* conv = purple_find_conversation_with_account(
        PURPLE_CONV_TYPE_CHAT,
        name,
        account
    );

    if (conv == NULL) {
        THROW(env, NULL, "conversation not found", NULL);
    }
    // Get the chat
    PurpleConvChat* convChat = purple_conversation_get_chat_data(conv);

    body = napi_help_strfromval(env, opts[2]);

    purple_conv_chat_send(convChat, body);
    free(name);
    free(body);

    return NULL;
}

napi_value messaging_chatParams(napi_env env, napi_callback_info info) {
    PurplePlugin* plugin;
    PurpleAccount* account;
    PurpleConnection* connection;
    size_t argc = 2;
    napi_value opts[2];
    char* protocolId;
    napi_value property_array;
    struct proto_chat_entry *pce;

    napi_get_cb_info(env, info, &argc, opts, NULL, NULL);
    if (argc < 2) {
      THROW(env, NULL, "chatParams takes two arguments", NULL);
    }

    // Get the account
    napi_get_value_external(env, opts[0], (void*)&account);

    connection = purple_account_get_connection(account);
    protocolId = napi_help_strfromval(env, opts[1]);

    plugin = purple_find_prpl(protocolId);
    free(protocolId);

    napi_create_array(env, &property_array);
    GList* parts = PURPLE_PLUGIN_PROTOCOL_INFO(plugin)->chat_info(connection);
    GList* l;
    uint32_t i = 0;
    for (l = parts; l != NULL; l = l->next)
    {
        pce = l->data;
        napi_value obj;
        napi_value value;
        napi_create_object(env, &obj);

        napi_create_string_utf8(env, pce->identifier, NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, obj, "identifier", value);

        napi_create_string_utf8(env, pce->label, NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, obj, "label", value);

        napi_get_boolean(env, pce->required, &value);
        napi_set_named_property(env, obj, "required", value);

        napi_set_element(env, property_array, i, obj);
        i++;
    }

    return property_array;
}

napi_value messaging_joinChat(napi_env env, napi_callback_info info) {
    PurpleAccount* account;
    PurpleConnection* conn;
    size_t argc = 2;
    napi_value opts[2];

    napi_get_cb_info(env, info, &argc, opts, NULL, NULL);
    if (argc < 2) {
      THROW(env, NULL, "joinChat takes two arguments", NULL);
    }

    // Get the account
    napi_get_value_external(env, opts[0], (void*)&account);
    conn = purple_account_get_connection(account);

    GHashTable* components = g_hash_table_new_full(
        g_str_hash,
        g_str_equal,
        free,
        free
    );
    napi_value nComponentNames;
    napi_value jkey;
    napi_value jvalue;
    napi_valuetype type;
    char* key;
    char* value;
    u_int32_t length;
    napi_get_property_names(env, opts[1], &nComponentNames);
    napi_get_array_length(env, nComponentNames, &length);
    for(u_int32_t i = 0; i < length;i++) {
        napi_get_element(env, nComponentNames, i, &jkey);
        napi_get_property(env, opts[1], jkey, &jvalue);
        napi_typeof(env, jvalue, &type);
        if (type == napi_string) {
            // We only support strings.
            key = napi_help_strfromval(env, jkey);
            value = napi_help_strfromval(env, jvalue);
            g_hash_table_insert(components, key, value);
        }
    }
    serv_join_chat(conn, components);
    g_hash_table_remove_all(components);

    return NULL;
}

napi_value messaging_rejectChat(napi_env env, napi_callback_info info) {
    return NULL;
}

napi_value messaging_getNickForChat(napi_env env, napi_callback_info info) {
    // purple_conv_chat_cb_find
    PurpleConversationType type;
    PurpleConversation* conv;
    napi_value res;
    size_t argc = 1;
    napi_value opts[1];

    napi_get_cb_info(env, info, &argc, opts, NULL, NULL);
    if (argc < 1) {
        THROW(env, NULL, "getNickForChat takes one argument", NULL);
    }
    napi_get_value_external(env, opts[0], (void*)&conv);
    type = purple_conversation_get_type(conv);
    if (type == PURPLE_CONV_TYPE_CHAT) {
        PurpleConvChat* chat = purple_conversation_get_chat_data(conv);
        const char* nick = purple_conv_chat_get_nick(chat);
        napi_create_string_utf8(env, nick, NAPI_AUTO_LENGTH, &res);
    } else {
        THROW(env, NULL, "conversation was not PURPLE_CONV_TYPE_CHAT", NULL);
    }
    return res;
}

napi_value messaging_findConversation(napi_env env, napi_callback_info info) {
    PurpleAccount* account;
    size_t argc = 2;
    napi_value opts[2];
    char* name;

    napi_get_cb_info(env, info, &argc, opts, NULL, NULL);
    if (argc < 2) {
        THROW(env, NULL, "findConversation takes two arguments", NULL);
    }

    // Get the account
    napi_get_value_external(env, opts[0], (void*)&account);

    if (account == NULL) {
        THROW(env, NULL, "account is null", NULL);
    }

    name = napi_help_strfromval(env, opts[1]);

    PurpleConversation* conv = purple_find_conversation_with_account(
        PURPLE_CONV_TYPE_ANY,
        name,
        account
    );

    free(name);
    if (conv == NULL) {
        THROW(env, NULL, "conversation not found", NULL);
    }

    return nprpl_conv_create(env, conv);
}

void messaging_bind_node(napi_env env,napi_value root) {
    napi_value namespace;
    napi_value _func;
    napi_create_object(env, &namespace);

    napi_create_function(env, NULL, 0, messaging_sendIM, NULL, &_func);
    napi_set_named_property(env, namespace, "sendIM", _func);

    napi_create_function(env, NULL, 0, messaging_sendChat, NULL, &_func);
    napi_set_named_property(env, namespace, "sendChat", _func);

    napi_create_function(env, NULL, 0, messaging_chatParams, NULL, &_func);
    napi_set_named_property(env, namespace, "chatParams", _func);

    napi_create_function(env, NULL, 0, messaging_joinChat, NULL, &_func);
    napi_set_named_property(env, namespace, "joinChat", _func);

    napi_create_function(env, NULL, 0, messaging_rejectChat, NULL, &_func);
    napi_set_named_property(env, namespace, "rejectChat", _func);

    napi_create_function(env, NULL, 0, messaging_getNickForChat, NULL, &_func);
    napi_set_named_property(env, namespace, "getNickForChat", _func);

    napi_create_function(env, NULL, 0, messaging_findConversation, NULL, &_func);
    napi_set_named_property(env, namespace, "findConversation", _func);

    napi_set_named_property(env, root, "messaging", namespace);
}
