#include "napi_helpers.h"
char* napi_help_strfromval(napi_env env, napi_value opt) {
    size_t length = 0;
    char* buffer;
    napi_get_value_string_utf8(env, opt, NULL, 0, &length);
    length++; //Null terminator
    buffer = malloc(sizeof(char) * length);
    napi_get_value_string_utf8(env, opt, buffer, length, NULL);
    return buffer;
}
