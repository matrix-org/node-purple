#ifndef SIGNALLING_H_INCLUDED
#define SIGNALLING_H_INCLUDED

#include <glib.h>
#include <node_api.h>
#include <account.h>
#include <conversation.h>
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

napi_value getJsObjectForSignalEvent(napi_env env, s_signalEventData *eventData);
void signalling_push(s_signalEventData *eventData);
GSList* signalling_pop();

/* Specialist callbacks */
void handleReceivedIMMessage(PurpleAccount *account, char *sender, char *buffer, PurpleConversation *conv, PurpleMessageFlags flags, void *data);

#endif
