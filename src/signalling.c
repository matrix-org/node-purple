#include "signalling.h"

static GSList *eventQueue = NULL; // s_signalEventData

typedef struct {
    PurpleAccount *account;
    char *sender;
    char *buffer;
    PurpleConversation *conv;
    PurpleMessageFlags flags;
    void *data;
} s_EventDataImMessage;

typedef struct {
    PurpleAccount *account;
    char *description;
    PurpleConnectionError type;
} s_EventDataConnectionError;

typedef struct {
    PurpleAccount *account;
    char *sender;
    char *roomName;
    char *message;
    GHashTable *inviteProps;
} s_EventDataInvited;

typedef struct {
    PurpleAccount *account;
    char* who;
    GList* items;
} e_UserInfoResponse;

typedef struct {
    char* label;
    char* value;
} e_UserInfoResponseItem;

napi_value getJsObjectForSignalEvent(napi_env env, s_signalEventData *eventData) {
    napi_value evtObj;
    napi_value value;
    napi_create_object(env, &evtObj);
    napi_create_string_utf8(env, eventData->signal, NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, evtObj, "eventName", value);
    /* This is where things get a bit messy, we want to extract information about each event */
    if (
            strcmp(eventData->signal, "account-signed-on") == 0 ||
            strcmp(eventData->signal, "account-signed-off") == 0  ||
            strcmp(eventData->signal, "account-added") == 0  ||
            strcmp(eventData->signal, "account-removed") == 0  ||
            strcmp(eventData->signal, "account-connecting") == 0  ||
            strcmp(eventData->signal, "account-created") == 0  ||
            strcmp(eventData->signal, "account-destroying") == 0
            ) {
        PurpleAccount* prplAcct = (PurpleAccount*)eventData->data;
        napi_value acct = nprpl_account_create(env, prplAcct);
        napi_set_named_property(env, evtObj, "account", acct);
    }

    if (strcmp(eventData->signal, "account-connection-error") == 0) {
        s_EventDataConnectionError msgData = *(s_EventDataConnectionError*)eventData->data;
        napi_value acct = nprpl_account_create(env, msgData.account);
        napi_set_named_property(env, evtObj, "account", acct);

        napi_create_string_utf8(env, msgData.description, NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, evtObj, "description", value);

        napi_create_uint32(env, msgData.type,&value);
        napi_set_named_property(env, evtObj, "type", value);
    }

    if (strcmp(eventData->signal, "received-im-msg") == 0 ||
        strcmp(eventData->signal, "received-chat-msg") == 0) {
        s_EventDataImMessage msgData = *(s_EventDataImMessage*)eventData->data;
        napi_value acct = nprpl_account_create(env, msgData.account);
        napi_set_named_property(env, evtObj, "account", acct);

        napi_create_string_utf8(env, msgData.buffer, NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, evtObj, "message", value);
        free(msgData.buffer);

        napi_create_string_utf8(env, msgData.sender, NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, evtObj, "sender", value);
        free(msgData.sender);
        if (msgData.conv != NULL) {
            value = nprpl_conv_create(env, msgData.conv);
            napi_set_named_property(env, evtObj, "conv", value);
        }
    }

    if(strcmp(eventData->signal, "user-info-response") == 0) {
        e_UserInfoResponse msgData = *(e_UserInfoResponse*)eventData->data;
        napi_value jkey, jvalue;
        GList* l;
        for (l = msgData.items; l != NULL; l = l->next) {
            e_UserInfoResponseItem item = *(e_UserInfoResponseItem*)l->data;
            napi_create_string_utf8(env, item.label, NAPI_AUTO_LENGTH, &jkey);
            if (item.value != NULL) {
                napi_create_string_utf8(env, item.value, NAPI_AUTO_LENGTH, &jvalue);
            } else {
                napi_get_undefined(env, &jvalue);
            }
            napi_set_property(env, evtObj, jkey, jvalue);
        }
        g_slist_free_full(msgData.items, free);

        napi_value acct = nprpl_account_create(env, msgData.account);
        napi_set_named_property(env, evtObj, "account", acct);

        napi_create_string_utf8(env, msgData.who, NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, evtObj, "who", value);
    }

    if (strcmp(eventData->signal, "chat-joined") == 0) {
        s_EventDataImMessage msgData = *(s_EventDataImMessage*)eventData->data;
        napi_value acct = nprpl_account_create(env, msgData.account);
        napi_set_named_property(env, evtObj, "account", acct);

        value = nprpl_conv_create(env, msgData.conv);
        napi_set_named_property(env, evtObj, "conv", value);
    }

    if (strcmp(eventData->signal, "chat-invite") == 0) {
        s_EventDataInvited msgData = *(s_EventDataInvited*)eventData->data;
        napi_value acct = nprpl_account_create(env, msgData.account);
        napi_set_named_property(env, evtObj, "account", acct);
        if (msgData.message != NULL) {
            napi_create_string_utf8(env, msgData.message, NAPI_AUTO_LENGTH, &value);
            napi_set_named_property(env, evtObj, "message", value);
        }

        if (msgData.roomName != NULL) {
            napi_create_string_utf8(env, msgData.roomName, NAPI_AUTO_LENGTH, &value);
            napi_set_named_property(env, evtObj, "room_name", value);
        }

        if(msgData.sender != NULL) {
            napi_create_string_utf8(env, msgData.sender, NAPI_AUTO_LENGTH, &value);
            napi_set_named_property(env, evtObj, "sender", value);
        }

        napi_create_object(env, &value);
        GHashTableIter iter;
        gpointer key, val;
        napi_value jkey, jvalue;

        g_hash_table_iter_init (&iter, msgData.inviteProps);
        struct proto_chat_entry *pce;
        while (g_hash_table_iter_next (&iter, &key, &val))
        {
            char* skey = (char*)key;
            char* sval = (char*)val;
            napi_create_string_utf8(env, skey, NAPI_AUTO_LENGTH, &jkey);
            if (sval != NULL) {
                napi_create_string_utf8(env, sval, NAPI_AUTO_LENGTH, &jvalue);
            } else {
                napi_get_undefined(env, &jvalue);
            }
            napi_set_property(env, value, jkey, jvalue);

        }
        napi_set_named_property(env, evtObj, "join_properties", value);

    }
    return evtObj;
}

void signalling_push(s_signalEventData *eventData) {
    eventQueue = g_slist_append(eventQueue, eventData);
}

GSList *signalling_pop() {
    GSList* item = eventQueue;
    if (item != NULL) {
        eventQueue = g_slist_remove_link(eventQueue, item);
    }
    return item;
}

/* Handle specific callbacks below */

void handleReceivedMessage(PurpleAccount *account, char *sender, char *buffer, PurpleConversation *conv,
                             PurpleMessageFlags flags, void *data) {
    s_signalCbData cbData = *(s_signalCbData*)data;
    s_signalEventData *ev = malloc(sizeof(s_signalEventData));
    s_EventDataImMessage *msgData = malloc(sizeof(s_EventDataImMessage));

    ev->signal = cbData.signal;
    msgData->account = account;

    msgData->buffer = malloc(strlen(buffer) + 1);
    strcpy(msgData->buffer, buffer);

    msgData->sender = malloc(strlen(sender) + 1);
    strcpy(msgData->sender, sender);

//    // TODO: Do not create a convo for chats
//    // The first message won't have a conversation, so create it.
    if (conv == NULL) {
        // This line was stolen from server.c#L624 where immediately after emitting
        // received-im-msg it creates a conversation below.
        conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, account, sender);
    }

    msgData->conv = conv;
    msgData->flags = flags;
    ev->freeMe = true;
    ev->data = msgData;
    signalling_push(ev);
}

void handleInvited(PurpleAccount *account, const char *inviter, const char *room_name, const char *message, GHashTable *data) {
    s_signalEventData *ev = malloc(sizeof(s_signalEventData));
    s_EventDataInvited *msgData = malloc(sizeof(s_EventDataInvited));
    ev->signal = "chat-invite";
    msgData->account = account;


    if (inviter != NULL) {
        msgData->sender = malloc(strlen(inviter) + 1);
        strcpy(msgData->sender, inviter);
    } else {
        msgData->sender = NULL;
    }


    if (room_name != NULL) {
        msgData->roomName = malloc(strlen(room_name) + 1);
        strcpy(msgData->roomName, room_name);
    } else {
        msgData->roomName = NULL;
    }


    if (message != NULL) {
        msgData->message = malloc(strlen(message) + 1);
        strcpy(msgData->message, message);
    } else {
        msgData->message = NULL;
    }


    // XXX: I'm not sure this is the best way to copy the table
    //      however it seems the only reliable way to do it.

    msgData->inviteProps = g_hash_table_new(g_str_hash, g_str_equal);

    GHashTableIter iter;
    gpointer key, val;
    g_hash_table_iter_init (&iter, data);

    while (g_hash_table_iter_next (&iter, &key, &val))
    {
        g_hash_table_insert(msgData->inviteProps, key, val);
    }

    ev->freeMe = true;
    ev->data = msgData;
    signalling_push(ev);
}

void handleJoined(PurpleConversation *chat) {
    s_signalEventData *ev = malloc(sizeof(s_signalEventData));
    s_EventDataImMessage *msgData = malloc(sizeof(s_EventDataImMessage));
    ev->signal = "chat-joined";
    msgData->conv = chat; //purple_conv_chat_get_conversation(chat);
    msgData->account = purple_conversation_get_account(chat);
    ev->freeMe = true;
    ev->data = msgData;
    signalling_push(ev);
}

void handleAccountConnectionError(PurpleAccount *account, PurpleConnectionError type, char* description) {
    s_signalEventData *ev = malloc(sizeof(s_signalEventData));
    s_EventDataConnectionError *msgData = malloc(sizeof(s_EventDataConnectionError));
    msgData->account = account;
    msgData->description = malloc(strlen(description) + 1);
    strcpy(msgData->description, description);
    msgData->type = type;
    ev->data = msgData;
    ev->signal = "account-connection-error";
    ev->freeMe = true;
    signalling_push(ev);
}

void handleUserInfo(PurpleConnection *gc, const char *who, PurpleNotifyUserInfo *user_info) {
    GList* entries = purple_notify_user_info_get_entries(user_info);
    if (entries == NULL) {
        return;
    }

    s_signalEventData *ev = malloc(sizeof(s_signalEventData));
    e_UserInfoResponse *msgData = malloc(sizeof(e_UserInfoResponse));

    msgData->items = NULL;
            //g_slist_copy_deep(entries, (GCopyFunc)_copy_user_info_entry, NULL);
    GList* l;
    for (l = entries; l != NULL; l = l->next) {
        PurpleNotifyUserInfoEntry *src = l->data;
        e_UserInfoResponseItem *dest;

        if (PURPLE_NOTIFY_USER_INFO_ENTRY_PAIR != purple_notify_user_info_entry_get_type(src)) {
            dest = NULL;
            continue;
        }
        dest = malloc(sizeof(e_UserInfoResponseItem));

        char* label = purple_notify_user_info_entry_get_label(src);
        dest->label = malloc(strlen(label) + 1);
        strcpy(dest->label, label);

        char* value = purple_notify_user_info_entry_get_value(src);
        dest->value = malloc(strlen(value) + 1);
        strcpy(dest->value, value);

        msgData->items = g_slist_append(msgData->items, dest);
    }
    msgData->who = malloc(strlen(who) + 1);
    msgData->account = purple_connection_get_account(gc);
    strcpy(msgData->who, who);
    ev->signal = "user-info-response";
    ev->freeMe = true;
    ev->data = msgData;
    signalling_push(ev);
}
