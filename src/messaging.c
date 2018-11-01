#include "messaging.h"

void messaging_sendIM(napi_env env, napi_callback_info info) {
    const PurpleAccount* account;
    size_t argc = 3;
    napi_value opts[3];
    int length = 0;
    char* name;
    char* body;

    napi_get_cb_info(env, info, &argc, opts, NULL, NULL);
    if (argc < 3) {
      napi_throw_error(env, NULL, "sendIM takes three arguments");
    }
    // Get the name (ugh this is always a PITA)
    napi_get_value_string_utf8(env, opts[1], NULL, NULL, &length);
    length++; //Null terminator
    name = malloc(sizeof(char)* length);
    napi_get_value_string_utf8(env, opts[1], name, length, NULL);

    // Get the account
    napi_get_value_external(env, opts[0], &account);

    if (account == NULL) {
        napi_throw_error(env, NULL, "account is null");
    }

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

    length = 0;
    napi_get_value_string_utf8(env, opts[2], NULL, NULL, &length);
    length++; //Null terminator
    body = malloc(sizeof(char)* length);
    napi_get_value_string_utf8(env, opts[2], body, length, NULL);

    purple_conv_im_send(convIm, body);
    free(name);
    free(body);
}
