#ifndef SIGNALLING_H_INCLUDED
#define SIGNALLING_H_INCLUDED

#include <glib.h>
#include <node_api.h>
#include <account.h>
#include <conversation.h>
#include <prpl.h>
#include "bindings/b_accounts.h"

typedef struct {
    char* signal;
} s_signalCbData;

typedef struct {
    char* signal;
    gpointer data;
    /* Should free the data */
    bool freeMe;
} s_signalEventData;

typedef enum {
    ACCOUNT_GENERIC = 0,
    RECEIVED_IM_MSG = 1,
    RECEIVED_CHAT_MSG = 2,
    CHAT_INVITE = 3
} e_EventObjectType;

napi_value getJsObjectForSignalEvent(napi_env env, s_signalEventData *eventData);
void signalling_push(s_signalEventData *eventData);
GSList* signalling_pop();

/* Specialist callbacks */
void handleReceivedMessage(PurpleAccount *account, char *sender, char *buffer, PurpleConversation *conv, PurpleMessageFlags flags, void *data);
void handleInvited(PurpleAccount *account, const char *inviter, const char *room_name, const char *message, GHashTable *data);
#endif
