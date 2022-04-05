#ifndef LEXER_H
#define LEXER_H
#include "token.h"
#include <stdio.h>
typedef struct LEXER_STRUCT
{
    char* src;                              //pointer auf src
    char c;                                 //current char
    unsigned int i;                         //index
    size_t src_size;                        //size für if-schleife
} lexer_T;

lexer_T* init_lexer(char* src);                 //initialisierung

void lexer_advance(lexer_T* lexer);

Token_T* lexer_advance_with(lexer_T* lexer, Token_T* token);

void lexer_skip_ws(lexer_T* lexer);             //whitespaces

Token_T* lexer_parse_id(lexer_T* lexer);

Token_T* lexer_next_token(lexer_T* lexer);      