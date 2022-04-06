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

bool isAtChar(char c)
{
    bool Result = (c == '@');
    return (Result);
}

bool isAlphaNum(char c)
{
    bool Result = (isAlpha(c) || isDigit(c) || isUnderscore(c) || isAtChar(c));
    return (Result);
}

bool isFirstLetterVariable(char c)
{
    bool Result = (isAlpha(c) || isAtChar(c) || isUnderscore(c));
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
    }
};

nameentry_t* findEntry(char *name)
{
    nameentry_t *Result = NULL;
    for (int i = 0; i < MAX_NAMES && name_tab[i].name != NULL; i++)
    {
        if (strcmp(name, name_tab[i].name) == 0)
        {
            Result = &name_tab[i];
            break;
        }
    }
    return (Result);
};

int getNametabLastIndex()
{
    int i = 0;
    while (name_tab[i].name != NULL)
    {
        i++;
    }

    return (i);
};


void lexer_skipWhitespace(Lexer *lexer)
{
    lexer_skipComments(lexer);

    while (isWhitespace(lexer->c))
    {
        lexer_advance(lexer);
        lexer_skipComments(lexer);
    }
};

typedef struct Token
{
    type_t type;
    srcpos_t pos;
    nameentry_t *entry;
    double value;
} Token;

void addEntry(Token *token)
{
    int index = getNametabLastIndex();
    nameentry_t *entry = &name_tab[index];
    token->entry = entry;
};

Token *initToken(srcpos_t pos)
{
    Token *token = calloc(1, sizeof(Token));
    token->type = name_any;
    token->pos = pos;
    token->entry = NULL;
    token->value = 0;
    return token;
}

Token *nextToken(Lexer *lexer)
{
    lexer_skipWhitespace(lexer);
    srcpos_t pos = {lexer->pos.line, lexer->pos.col};
    Token *token = initToken(pos);

    if (isFirstLetterVariable(lexer->c))
    {
        char* value = calloc(1, sizeof(char));
        while (isAlphaNum(lexer->c))
        {
            value = realloc(value, sizeof(char) * (strlen(value) + 1));
            value[strlen(value)] = lexer->c;
            lexer_advance(lexer);
        }

        token->entry = findEntry(value);

        if (token->entry == NULL)
        {
            token->entry = &name_tab[getNametabLastIndex()];
            token->entry->name = strdup(value);
            token->entry->type = name_any;
        }
        else
        {
            token->type = token->entry->type;
        }

        return token;
    }
    else if (isDigit(lexer->c))
    {
        token->type = oper_const;
        double value = 0;
        double frac = 0.1;
        do
        {
            value = value * 10 + (lexer->c - '0');
            lexer_advance(lexer);
        } while (isDigit(lexer->c));
        if (lexer->c == '.')
        {
            lexer_advance(lexer);
            while (isDigit(lexer->c))
            {
                value += (frac * (lexer->c - '0'));
                frac /= 10;
                lexer_advance(lexer);
            }
        }
        token->value = value;
        return token;
    }

    switch (lexer->c)
    {
    case '(':
        token->type = oper_lpar;
        break;
    case ')':
        token->type = oper_rpar;
        break;
    case ',':
        token->type = oper_sep;
        break;
    case '|':
        token->type = oper_abs;
        break;
    case '^':
        token->type = oper_pow;
        break;
    case '*':
        token->type = oper_mul;
        break;
    case '/':
        token->type = oper_div;
        break;
    case '+':
        token->type = oper_add;
        break;
    case '-':
        token->type = oper_sub;
        break;
    // case ünares, nur im syntaxbaum ?
    // see turtle-types.h line 111
    case '=':
        token->type = oper_equ;
        break;
    case '<':
        if (lexer_peek(lexer, 1) == '>') // case <>
        {
            token->type = oper_nequ;
            lexer_advance(lexer);
        }
        else if (lexer_peek(lexer, 1) == '=') // case <=
        {
            token->type = oper_lequ;
            lexer_advance(lexer);
        }
        else // case <
        {
            token->type = oper_less;
        }
        break;
    case '>':
        if (lexer_peek(lexer, 1) == '=') // case >=
        {
            token->type = oper_gequ;
            lexer_advance(lexer);
        }
        else // case >
        {
            token->type = oper_grtr;
        }
        break;
    }

    lexer_advance(lexer);

    return token;
}

void printNametab()
{
    printf("printing newly added variables/keywords in nametab:\n");

    for (int i = 70; i < MAX_NAMES && name_tab[i].name != NULL; i++)
    {
        printf("%s\n", name_tab[i].name);
    }
}

int main(int argc, const char *argv[])
{
    src_file = fopen(argv[1], "r");
    char *src;
    // copy all data from src_file to src
    fseek(src_file, 0, SEEK_END);
    long lSize = ftell(src_file);
    rewind(src_file);
    src = calloc(1, lSize + 1);
    if (1 != fread(src, lSize, 1, src_file))
        fclose(src_file), free(src), fputs("entire read fails", stderr), exit(1);
    fclose(src_file);

    // char *src = "\" this is a comment \n aasdf adddd = 1 adddd+ aasdaf - sdfdd";
    Lexer lexer;
    lexer_init(&lexer, src);

    printf("starting lexer\n");
    Token *token;

    while (lexer.c != '\0')
    {
        token = nextToken(&lexer);
        if (token->entry)
        {
            printf("type: %d, name: %s, keyw: %d\n", token->type, token->entry->name, token->entry->type);
        }
        else
        {
            printf("type: %d, value: %f\n", token->type, token->value);
        }
    }

    printNametab();
    return 0;
}