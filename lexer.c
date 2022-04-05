#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "turtle.h"

bool isEndOfLine(char c)
{
    bool Result = (c == '\n');
    return (Result);
}

bool isWhitespace(char c)
{
    bool Result = ((c == ' ') || (c == '\t') || (c == '\v') || (c == '\f') || isEndOfLine(c));
    return (Result);
}

bool isAlpha(char c)
{
    bool Result = (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')));
    return (Result);
}

bool isDigit(char c)
{
    bool Result = ((c >= '0') && (c <= '9'));
    return (Result);
}

bool isUnderscore(char c)
{
    bool Result = (c == '_');
    return (Result);
}

bool isAlphaNum(char c)
{
    bool Result = (isAlpha(c) || isDigit(c) || isUnderscore(c));
    return (Result);
}

typedef struct Lexer
{
    char *src;      // src file
    char c;         // current char
    unsigned int i; // current index (pos in src)
    srcpos_t pos;
} Lexer;

void lexer_init(Lexer *lexer, char *src)
{
    lexer->src = src;
    lexer->i = 0;
    lexer->c = src[lexer->i];
    lexer->pos.line = 1;
    lexer->pos.col = 1;
};

void lexer_advance(Lexer *lexer)
{
    lexer->i++;
    lexer->c = lexer->src[lexer->i];
    if (isEndOfLine(lexer->c))
    {
        lexer->pos.line++;
        lexer->pos.col = 1;
    }
    else
    {
        lexer->pos.col++;
    }
};

char lexer_peek(Lexer *lexer, int offset)
{
    return (lexer->src[lexer->i + offset]);
};

void lexer_skipComments(Lexer *lexer)
{
    while (lexer->c == '"')
    {
        while (lexer->c != '\n')
        {
            lexer_advance(lexer);
        }
        lexer_advance(lexer);
    }
};

void lexer_skipWhitespace(Lexer *lexer)
{
    lexer_skipComments(lexer);

    while (isWhitespace(lexer->c))
    {
        lexer_advance(lexer);
    }
};

typedef struct Token
{
    type_t type;
    srcpos_t pos;
    char *value;
} Token;

Token nextToken(Lexer *lexer)
{
    lexer_skipWhitespace(lexer);
    Token token;
    srcpos_t pos = {lexer->pos.line, lexer->pos.col};
    token.pos = pos;
    char* value = calloc(1, sizeof(char));
    token.value = value;

    if (isAlpha(lexer->c))
    {
        token.type = name_any;
        token.pos = lexer->pos;
        while (isAlphaNum(lexer->c))
        {
            value = realloc(value, sizeof(char) * (strlen(value) + 1));
            value[strlen(value)] = lexer->c;
            lexer_advance(lexer);
        }

        return token;
    }
    else if (isDigit(lexer->c))
    {
        token.type = name_any;
        token.pos = lexer->pos;
        while (isDigit(lexer->c))
        {
            value = realloc(value, sizeof(char) * (strlen(value) + 1));
            value[strlen(value)] = lexer->c;
            lexer_advance(lexer);
        }
        return token;
    }

    switch (lexer->c)
    {
    case '(':
        token.type = oper_lpar;
        break;
    case ')':
        token.type = oper_rpar;
        break;
    case ',':
        token.type = oper_sep;
        break;
    case '|':
        token.type = oper_abs;
        break;
    case '^':
        token.type = oper_pow;
        break;
    case '*':
        token.type = oper_mul;
        break;
    case '/':
        token.type = oper_div;
        break;
    case '+':
        token.type = oper_add;
        break;
    case '-':
        token.type = oper_sub;
        break;
    // case ünares, nur im syntaxbaum ?
    // see turtle-types.h line 111
    case '=':
        token.type = oper_equ;
        break;
    case '<':
        if (lexer_peek(lexer, 1) == '>') // case <>
        {
            token.type = oper_nequ;
            lexer_advance(lexer);
        }
        else if (lexer_peek(lexer, 1) == '=') // case <=
        {
            token.type = oper_lequ;
            lexer_advance(lexer);
        }
        else // case <
        {
            token.type = oper_less;
        }
        break;
    case '>':
        if (lexer_peek(lexer, 1) == '=') // case >=
        {
            token.type = oper_gequ;
            lexer_advance(lexer);
        }
        else // case >
        {
            token.type = oper_grtr;
        }
        break;
    }

    lexer_advance(lexer);

    return token;
}

int main(int argc, const char *argv[])
{
    char *src = "\" this is a comment \n aasdf = 1 + 23333 (),|   ^ * / + - = <> < <= > >=";
    Lexer lexer;
    lexer_init(&lexer, src);
    Token token;

    printf("%s\n", name_tab[0].name);

    while (lexer.c != '\0')
    {
        token = nextToken(&lexer);
        printf("type: %d, value: %s\n", token.type, token.value);
    }
    return 0;
}