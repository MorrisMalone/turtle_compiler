#include <string.h>
#include <stdio.h>
#include "turtle-types.h"

int c = '\n';
// Zeilen- und Spaltennummer (für Fehlermeldungen)
int lineNr = 0, colNr = 0;

bool isEndOfLine(char c)
{
    bool Result = (c == '\n');
    return (Result);
}

bool isWhitespace(char c)
{
    bool Result = ((c == ' ') || (c == '\t') || (c == '\v') || (c == '\f') || IsEndOfLine(c));
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

typedef struct Token
{
    type_t type;
    srcpos_t pos;
} Token;

Token nextToken(void)
{
    Token token;
    switch (c)
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
        // if next char is '>': oper_nequ
        // if next char is '=': oper_lequ
        // not forget to move cursor position +2
        // else oper_less
        break;
    case '>':
        // if next char is '=': oper_gequ
        // not forget to move cursor postion +2
        // else oper_grtr
    }
}