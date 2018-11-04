#include "napi_helpers.h"
char* napi_help_strfromval(napi_env env, napi_value opt) {
    int length = 0;
    char* buffer;
    napi_get_value_string_utf8(env, opt, NULL, NULL, &length);
    length++; //Null terminator
    buffer = malloc(sizeof(char)* length);
    napi_get_value_string_utf8(env, opt, buffer, length, NULL);
    return buffer;
}
