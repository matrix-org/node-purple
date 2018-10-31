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
    if (strcmp(eventData->signal, "received-im-msg") == 0) {
        s_EventDataImMessage msgData = *(s_EventDataImMessage*)eventData->data;
        napi_value acct = nprpl_account_create(env, msgData.account);
        napi_set_named_property(env, evtObj, "account", acct);

        napi_create_string_utf8(env, msgData.buffer, NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, evtObj, "message", value);

        napi_create_string_utf8(env, msgData.sender, NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, evtObj, "sender", value);
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
void handleReceivedIMMessage(PurpleAccount *account, char *sender, char *buffer, PurpleConversation *conv,
                             PurpleMessageFlags flags, void *data) {
    s_signalEventData *ev = malloc(sizeof(s_signalEventData));
    s_EventDataImMessage *msgData = malloc(sizeof(s_EventDataImMessage));
    ev->signal = "received-im-msg";
    msgData->account = account;

    msgData->buffer = malloc(strlen(buffer));
    strcpy(msgData->buffer, buffer);

    msgData->sender = malloc(strlen(sender));
    strcpy(msgData->sender, sender);

    msgData->conv = conv;
    msgData->flags = flags;
    ev->freeMe = true;
    ev->data = msgData;
    signalling_push(ev);
}
