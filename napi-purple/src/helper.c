#include "helper.h"

bool getValueFromObject(napi_env env, napi_value object, char* propStr, napi_valuetype *type, napi_value *value);

typedef struct {
  int32_t debugEnabled;
  char userDir[512];
  bool userDirSet;
} s_setupPurple;

void getSetupPurpleStruct(napi_env env, napi_callback_info info, s_setupPurple *o) {
    size_t argc = 1;
    napi_status status;
    napi_value opts;
    s_setupPurple stemp;
    napi_valuetype type;
    napi_value value;
    stemp.userDirSet = false;

    napi_get_cb_info(env, info, &argc, &opts, NULL, NULL);
    if (argc == 0) {
      napi_throw_error(env, NULL, "setupPurple takes a options object");
    }

    /* debugEnabled */
    if (getValueFromObject(env, opts, "debugEnabled", &type, &value)) {
      status = napi_get_value_int32(env, value, &stemp.debugEnabled);
    }

    /* userDir */
    if (getValueFromObject(env, opts, "userDir", &type, &value)) {
      status = napi_get_value_string_utf8(env, value, (char *) &stemp.userDir, 512, NULL);
      printf("status: %d\ndir: %s\n", status, stemp.userDir);
      stemp.userDirSet = true;
    }
    *o = stemp;
}

// void handleUiInit() {
//   printf("called handleUiInit");
// }

//Does everything needed to create a purple session.
//{
// debugEnabled: 1|0
//
//}
napi_value setupPurple(napi_env env, napi_callback_info info) {
    napi_value n_undef;
    s_setupPurple opts;
    PurpleConversationUiOps uiopts = {
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
    napi_get_undefined(env, &n_undef);

    PurpleEventLoopUiOps eventLoop;
    eventLoop_get(&eventLoop, &env);

    printf("purple_eventloop_set_ui_ops()\n");
    purple_eventloop_set_ui_ops(&eventLoop);

    getSetupPurpleStruct(env, info, &opts);
    if (opts.userDirSet) {
  	  purple_util_set_user_dir(opts.userDir);
    }


    printf("purple_debug_set_enabled(%d)\n", opts.debugEnabled);
    purple_debug_set_enabled(opts.debugEnabled);

    printf("purple_conversation_set_ui_ops()\n");
    purple_conversation_set_ui_ops(&uiopts, NULL);
    printf("purple_core_init()\n");
    purple_core_init("matrix-bridge");
    return n_undef;
}

/* N-API helpers */

bool getValueFromObject(napi_env env, napi_value object, char* propStr, napi_valuetype *type, napi_value *value) {
  napi_status status;
  napi_value propName;
  bool hasProperty;
  status = napi_create_string_utf8(env, propStr, NAPI_AUTO_LENGTH, &propName);
  if (status != napi_ok) {
    printf("Status %d\n", status);
    napi_throw_error(env, NULL, "Could not get value from object: Could not create string");
  }
  status = napi_has_property(env, object, propName, &hasProperty);
  if (status != napi_ok) {
    printf("Status %d\n", status);
    napi_throw_error(env, NULL, "Could not get value from object: Could not get property");
  }
  if (!hasProperty) {
    return false;
  }
  status = napi_get_property(env, object, propName, value);
  if (status != napi_ok) {
    printf("Status %d\n", status);
    napi_throw_error(env, NULL, "Could not get value from object: Could not get property");
  }
  status = napi_typeof(env, *value, type);
  if (status != napi_ok) {
    printf("Status %d\n", status);
    napi_throw_error(env, NULL, "Could not get value from object: Could not get type");
  }
  return true;
}
