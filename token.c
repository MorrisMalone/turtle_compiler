#include "token.h"
#include <stdlib.h>
Token_T* init_token(char* value, int type)
{
    Token_T* token = calloc(1, sizeof(struct TOKEN_STRUCT));    //calloc(items, size)
    token->value = value;
    token->type = type;

    return token;
}