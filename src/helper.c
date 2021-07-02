#include "helper.h"

bool getValueFromObject(napi_env env, napi_value object, char* propStr, napi_valuetype *type, napi_value *value);

typedef struct {
  int32_t debugEnabled;
  napi_value eventFunc;
  char* userDir;
  char* pluginDir;
} s_setupPurple;

void getSetupPurpleStruct(napi_env env, napi_callback_info info, s_setupPurple* setupOpts) {
    size_t argc = 1;
    napi_value opts;
    napi_valuetype type;
    napi_value value;
    napi_get_cb_info(env, info, &argc, &opts, NULL, NULL);
    if (argc == 0) {
        THROW(env, NULL, "setupPurple takes a options object");
    }

    /* debugEnabled */
    if (getValueFromObject(env, opts, "debugEnabled", &type, &value)) {
        napi_get_value_int32(env, value, &setupOpts->debugEnabled);
    }

    if (napi_ok != napi_get_named_property(env, opts, "eventFunc", &setupOpts->eventFunc)) {
        THROW(env, NULL, "setupPurple expects eventFunc to be defined");
    }

    /* userDir */
    if (getValueFromObject(env, opts, "userDir", &type, &value) && type == napi_string) {
        setupOpts->userDir = napi_help_strfromval(env, value);
    } else {
        setupOpts->userDir = NULL;
    }

    /* pluginDir */
    if (getValueFromObject(env, opts, "pluginDir", &type, &value) && type == napi_string) {
        setupOpts->pluginDir = napi_help_strfromval(env, value);
    } else {
        setupOpts->pluginDir = NULL;
    }
}

napi_value pollEvents(napi_env env, napi_callback_info info) {
    napi_value eventArray;
    napi_value evtObj;
    napi_create_array(env, &eventArray);
    int i = 0;
    s_signalEventData *evtData;

    GSList* eventQueue = signalling_pop();

    while(eventQueue != NULL) {
        evtData = (s_signalEventData*)eventQueue->data;
        evtObj = getJsObjectForSignalEvent(env, evtData);
        napi_set_element(env, eventArray, i, evtObj);
        if (evtData->freeMe) {
            free(evtData->data);
        }
        free(eventQueue->data);
        eventQueue = signalling_pop();
        i++;
    }
    return eventArray;
}

void handlePurpleSignalCb(gpointer signalData, gpointer data) {
    s_signalCbData cbData = *(s_signalCbData*)data;
    s_signalEventData *ev = malloc(sizeof(s_signalEventData));
    ev->signal = cbData.signal;
    ev->data = signalData;
    // Don't free this, it's not ours.
    ev->freeMe = false;
    signalling_push(ev);
}

void wirePurpleSignalsIntoNode(napi_env env, napi_value eventFunc) {
    static int handle;
    s_signalCbData *cbData;
    void *conn_handle = purple_connections_get_handle();
    void *conv_handle = purple_conversations_get_handle();
    void *accounts_handle = purple_accounts_get_handle();

    cbData = malloc(sizeof(s_signalCbData));
    cbData->signal = "signing-on";
    purple_signal_connect(conn_handle, "signing-on", &handle,
                PURPLE_CALLBACK(handlePurpleSignalCb), cbData);

    cbData = malloc(sizeof(s_signalCbData));
    cbData->signal = "account-signed-on";
    purple_signal_connect(accounts_handle, "account-signed-on", &handle,
                PURPLE_CALLBACK(handlePurpleSignalCb), cbData);

    cbData = malloc(sizeof(s_signalCbData));
    cbData->signal = "account-signed-off";
    purple_signal_connect(accounts_handle, "account-signed-off", &handle,
                PURPLE_CALLBACK(handlePurpleSignalCb), cbData);

    cbData = malloc(sizeof(s_signalCbData));
    cbData->signal = "account-disabled";
    purple_signal_connect(accounts_handle, "account-disabled", &handle,
                PURPLE_CALLBACK(handlePurpleSignalCb), cbData);

    cbData = malloc(sizeof(s_signalCbData));
    cbData->signal = "account-enabled";
    purple_signal_connect_vargs(accounts_handle, "account-enabled", &handle,
                PURPLE_CALLBACK(handlePurpleSignalCb), cbData);

    cbData = malloc(sizeof(s_signalCbData));
    cbData->signal = "account-connecting";
    purple_signal_connect(accounts_handle, "account-connecting", &handle,
                PURPLE_CALLBACK(handlePurpleSignalCb), cbData);

    cbData = malloc(sizeof(s_signalCbData));
    cbData->signal = "account-connection-error";
    purple_signal_connect(accounts_handle, "account-connection-error", &handle,
                PURPLE_CALLBACK(handleAccountConnectionError), cbData);

    cbData = malloc(sizeof(s_signalCbData));
    cbData->signal = "received-im-msg";
    purple_signal_connect(conv_handle, "received-im-msg", &handle,
                PURPLE_CALLBACK(handleReceivedMessage), cbData);

    cbData = malloc(sizeof(s_signalCbData));
    cbData->signal = "received-chat-msg";
    purple_signal_connect(conv_handle, "received-chat-msg", &handle,
                PURPLE_CALLBACK(handleReceivedMessage), cbData);

    // Joined
    /*cbData = malloc(sizeof(s_signalCbData));
    cbData->signal = "chat-user-joined";
    purple_signal_connect(conv_handle, "chat-user-joined", &handle,
                PURPLE_CALLBACK(handleUserJoined), NULL);
    // Invited (other users)
    cbData->signal = "chat-invited-user";
    purple_signal_connect(conv_handle, "chat-invited-user", &handle,
                PURPLE_CALLBACK(handleUserInvited), NULL);
    // Left
    cbData->signal = "chat-user-left";
    purple_signal_connect(conv_handle, "chat-user-left", &handle,
                PURPLE_CALLBACK(handleUserLeft), NULL);*/

    purple_signal_connect(conv_handle, "chat-joined", &handle,
                PURPLE_CALLBACK(handleJoined), NULL);

    purple_signal_connect(conv_handle, "chat-invited", &handle,
                PURPLE_CALLBACK(handleInvited), NULL);
}

void _accounts_restore_current_statuses()
{
    GList *l;
    PurpleAccount *account;

    /* If we're not connected to the Internet right now, we bail on this */
    /*if (!purple_network_is_available())
    {
        purple_debug_warning("account", "Network not connected; skipping reconnect\n");
        return;
    }*/
    unsigned timeout = 0;
    for (l = purple_accounts_get_all(); l != NULL; l = l->next)
    {
        account = (PurpleAccount *)l->data;
        if (purple_account_get_enabled(account, purple_core_get_ui()) &&
            (purple_presence_is_online(account->presence)))
        {
            timeout_add(timeout, G_SOURCE_FUNC(purple_account_connect), account);
            timeout += 100;
        }
    }
}

napi_value setupPurple(napi_env env, napi_callback_info info) {
    napi_value n_undef;
    napi_get_undefined(env, &n_undef);

    s_setupPurple opts;
    PurpleConversationUiOps uiops = {
        NULL,                      /* create_conversation  */
        NULL,                      /* destroy_conversation */
        NULL,                      /* write_chat           */
        NULL,                      /* write_im             */
        NULL,                      /* write_conv           */
        NULL,                      /* chat_add_users       */
        NULL,                      /* chat_rename_user     */
        NULL,                      /* chat_remove_users    */
        NULL,                      /* chat_update_user     */
        NULL,                      /* present              */
        NULL,                      /* has_focus            */
        NULL,                      /* custom_smiley_add    */
        NULL,                      /* custom_smiley_write  */
        NULL,                      /* custom_smiley_close  */
        NULL,                      /* send_confirm         */
        NULL,
        NULL,
        NULL,
        NULL
    };
    PurpleNotifyUiOps *notifyopts = malloc(sizeof(PurpleNotifyUiOps));
    notifyopts->notify_userinfo = handleUserInfo;
    purple_notify_set_ui_ops(notifyopts);
    PurpleEventLoopUiOps *evLoopOps = eventLoop_get(&env);
    if (evLoopOps == NULL) {
        // Exception, return
        return;
    }
    purple_eventloop_set_ui_ops(evLoopOps);

    getSetupPurpleStruct(env, info, &opts);
    purple_debug_set_enabled(opts.debugEnabled);
    if (opts.userDir != NULL) {
        // Purple copies these strings
        purple_util_set_user_dir(opts.userDir);
        free(opts.userDir);
    }

    if (opts.pluginDir != NULL) {
        // Purple copies these strings
        purple_plugins_add_search_path(opts.pluginDir);
        free(opts.pluginDir);
    }

    purple_prefs_load();
    purple_set_blist(purple_blist_new());
    purple_core_init(STR_PURPLE_UI);
    // To restore all the accounts.
    _accounts_restore_current_statuses();
    // To get our buddies :3
    purple_blist_load();
    wirePurpleSignalsIntoNode(env, opts.eventFunc);
    free(opts.pluginDir);
    return n_undef;
}

/* N-API helpers */

bool getValueFromObject(napi_env env, napi_value object, char* propStr, napi_valuetype *type, napi_value *value) {
    napi_status status;
    napi_value propName;
    bool hasProperty;
    status = napi_create_string_utf8(env, propStr, NAPI_AUTO_LENGTH, &propName);
    if (status != napi_ok) {
        THROW(env, NULL, "Could not get value from object: Could not create string", false);
    }
    status = napi_has_property(env, object, propName, &hasProperty);
    if (status != napi_ok) {
        THROW(env, NULL, "Could not get value from object: Could not get property", false);
    }
    if (!hasProperty) {
        return false;
    }
    status = napi_get_property(env, object, propName, value);
    if (status != napi_ok) {
        THROW(env, NULL, "Could not get value from object: Could not get property", false);
    }
    status = napi_typeof(env, *value, type);
    if (status != napi_ok) {
        THROW(env, NULL, "Could not get value from object: Could not get type", false);
    }
    return true;
}
