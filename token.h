#ifndef TOKEN_DEF
#define TOKEN_DEF
typedef struct TOKEN_STRUCT
{
    char* value;
    enum
    {
        Token_ID,
        Token_assign,
        Token_paranL,
        Token_paranR,
        Token_braceL,
        Token_braceR,
        Token_colon,
        Token_comma,
        Token_semicolon,
        Token_lessth,
        Token_greaterth,
        Token_ArrowR,
        Token_EOF,
    } type;
} Token_T;

Token_T* init_token(char* value, int type);
#endif