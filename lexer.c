#include "lexer.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


lexer_T* init lexer(char* src)
{
    lexer_T* lexer = calloc(1, sizeof(struct LEXER_STRUCT));                //calloc(items, size)
    lexer-> src = src;
    lexer-> i = 0;
    lexer-> src_size = strlen(src);
    lexer-> c = src [lexer->i];

    return lexer;
}


void lexer_advance(lexer_T* lexer)
{
    if (lexer -> i < lexer->src_size && lexer->c != '\0'; lexer->i += 1)    //solange kleiner als size und nicht am ende -> incr
    {
        lexer-> c = lexer->src[lexer->i];
    }
};

Token_T* lexer_advance_with(lexer_parse_id(lexer))
{
    lexer_advance(lexer);
    return token;
}


void lexer_skip_ws(lexer_T* lexer)
{
    while (lexer->c == ' ' || lexer->c == '\t'|| lexer->c =='\n')           //any ws?
    {
        lexer_advance(lexer);
    }
}

Token_T* lexer_parse_id(lexer_T* lexer)
{

    char* value = calloc(1, sizeof(char));
    while(isalnum(lexer->c))
    {
        value = realloc(value, (strlen(value)+2) * sizeof(char));
        strcat(value, (char[]){lexer->c, 0};
        lexer_advance(lexer);
    }

    return init_token(value, TOKEN_ID)
}

Token_T* lexer_next_token(lexer_T* lexer)
{
    while (lexer->c != '\0')
    {
        if(isalpha(lexer->c))                   //is buchstabe?
            return lexer_advance_with(lexer, lexer_parse_id(lexer));
    }

    return init_token(0, Token_EOF);
}