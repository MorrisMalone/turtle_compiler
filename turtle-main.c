
// Turtle-Graphics-Compiler:
// Hauptprogramm und programmweite Hilfsfunktionen
//
// Klaus Kusche 2021, 2022

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "turtle.h"

// Global: Der Sourcefile & der Programmname
FILE *src_file;
const char *prog_name;

// Prüfe ob malloc/calloc/realloc erfolgreich war:
// Fehlermeldung und Programmende wenn p gleich NULL ist
// what ... was wurde gerade angelegt?
// pos ... für welche Stelle im Source?
void mem_check(const void *p, const char *what, const srcpos_t *pos)
{
    if (p == NULL)
    {
        fprintf(stderr, "%s: Fehler beim Anlegen von Speicher für %s "
                        "(Zeile %d, Spalte %d): %s\n",
                prog_name, what, pos->line, pos->col, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

// Ausgabe eines Fehlers im Turtle-Programm an Stelle pos & Programmende
// (mit variabel vielen Parametern wie printf)
void code_error(const srcpos_t *pos, const char *format, ...)
{
    va_list arg_p;

    fprintf(stderr, "Fehler in Zeile %d, Spalte %d: ", pos->line, pos->col);
    va_start(arg_p, format);
    vfprintf(stderr, format, arg_p);
    va_end(arg_p);
    putchar('\n');
    exit(EXIT_FAILURE);
}

void error(srcpos_t *pos)
{
    printf("Error! -> line: %d, column: %d\n", pos->line, pos->col);
    exit(EXIT_FAILURE);
}

struct Lexer;

void printLexerPos(struct Lexer *lexer);
void makePathFunction(struct Lexer *lexer);

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
    if (isEndOfLine(lexer->c))
    {
        lexer->pos.line += 1;
        lexer->pos.col = 1;
    }
    else
    {
        lexer->pos.col += 1;
    }
    lexer->i += 1;
    lexer->c = lexer->src[lexer->i];
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

nameentry_t *findEntry(char *name)
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
    int i;
} Token;

void printToken(Token *token);

void addEntry(Token *token)
{
    int index = getNametabLastIndex();
    nameentry_t *entry = &name_tab[index];
    token->entry = entry;
};

Token *initToken(srcpos_t pos, int i)
{
    Token *token = calloc(1, sizeof(Token));
    token->type = name_any;
    token->pos = pos;
    token->entry = NULL;
    token->value = 0;
    token->i = i;
    return token;
}

void rewindLexer(Lexer *lexer, Token *token)
{
    lexer->pos = token->pos;
    lexer->i = token->i;
    lexer->c = lexer->src[lexer->i];
}

void printLexerPos(Lexer *lexer)
{
    printf("LEXER POS: %d, %d, i: %d\n", lexer->pos.line, lexer->pos.col, lexer->i);
}

Token *nextToken(Lexer *lexer)
{
    printf("start next Token\n");

    lexer_skipWhitespace(lexer);
    srcpos_t pos = {lexer->pos.line, lexer->pos.col};
    Token *token = initToken(pos, lexer->i);

    if (isFirstLetterVariable(lexer->c))
    {
        char *value = calloc(1, sizeof(char));
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
            if (isAtChar(value[0]))
            {
                token->entry->type = name_glob;
                token->type = name_glob;
            }
        }
        else
        {
            token->type = token->entry->type;
        }
        printToken(token);
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
        printToken(token);
        return token;
    }

    switch (lexer->c)
    {
    case '\0':
        token->type = tok_bofeof;
        return token;
        break;
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

    printToken(token);

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

// parser
treenode_t *expression(Lexer *lexer);
treenode_t *operand(Lexer *lexer);

treenode_t *startingNode;

treenode_t *initNode()
{
    treenode_t *node = calloc(1, sizeof(treenode_t));
    node->type = 0;
    node->pos = (srcpos_t){0, 0};
    node->next = NULL;
    node->son;
    return node;
}

treenode_t *comparison(Lexer *lexer);

treenode_t *comparison(Lexer *lexer)
{
    treenode_t *oper1 = expression(lexer);
    Token *comparator = nextToken(lexer);
    type_t type = comparator->type;

    treenode_t *comparisonNode = NULL;

    if (
        type == oper_equ ||
        type == oper_nequ ||
        type == oper_less ||
        type == oper_lequ ||
        type == oper_grtr ||
        type == oper_gequ
    )
    {
        comparisonNode = initNode();
        comparisonNode->type = type;
        comparisonNode->pos = comparator->pos;
        comparisonNode->son[0] = oper1;
        comparisonNode->son[1] = expression(lexer);

        return comparisonNode;
    }

    rewindLexer(lexer, comparator);
    return oper1;
}

treenode_t *condition(Lexer *lexer)
{
    Token *token = nextToken(lexer);
    bool withParenthesis = false;
    if (token->type != oper_lpar)
    {
        rewindLexer(lexer, token);
    }
    else
        withParenthesis = true;
    
    // get first operand
    treenode_t *comparison1 = comparison(lexer);
    // token = nextToken(lexer);
    // type_t type = token->type;

    // treenode_t *andOrNot = NULL;

    // if (type == oper_nequ)
    // {
    //     andOrNot = initNode();
    //     andOrNot->type = type;
    //     andOrNot->pos = token->pos;
    //     andOrNot->son[0] = condition(lexer);
    // }



    // treenode_t *oper1 = operand(lexer);
    // treenode_t *node = initNode();
    // token = nextToken(lexer);

    // switch (token->type)
    // {
    // case oper_nequ:
    // {
    //     node->type = oper_nequ;
    //     node->pos = token->pos;
    //     node->son[0] = oper1;
    //     node->son[1] = operand(lexer);
    //     break;
    // }
    // case keyw_not:
    // {
    //     node->type = keyw_not;
    //     node->son[0] = operand(lexer);
    //     token = nextToken(lexer);
    //     break;
    // }
    // default:
    //     break;
    // }

    // if (withParenthesis)
    // {
    //     // consume token
    //     Token *eaten = nextToken(lexer);
    // }

    return comparison1;
}

treenode_t *operand(Lexer *lexer)
{
    Token *token = nextToken(lexer);
    treenode_t *node = NULL;

    switch (token->type)
    {
    case oper_sub:
        node = initNode();
        node->type = oper_neg;
        node->pos = token->pos;
        node->son[0] = expression(lexer);
        break;
    case oper_const:
        node = initNode();
        node->type = oper_const;
        node->pos = token->pos;
        node->d.val = token->value;
        break;
    case oper_abs:
    {
        node = initNode();
        node->type = oper_abs;
        node->pos = token->pos;
        node->son[0] = expression(lexer);
        Token *endToken = nextToken(lexer);
        if (endToken->type != oper_abs)
            error(&endToken->pos);
        break;
    }
    case oper_lpar:
        {
            node = expression(lexer);
            Token *endToken = nextToken(lexer);
            if (endToken->type != oper_rpar)
                error(&endToken->pos);
            break;
        }
    case name_math_rand:
        node = initNode();
        node->type = oper_lpar;
        node->pos = token->pos;
        node->d.p_name = token->entry;
        // consume (
        Token *lpar = nextToken(lexer);
        if (lpar->type != oper_lpar)
            error(&lpar->pos);
        node->son[0] = expression(lexer);
        Token *sep = nextToken(lexer);
        if (sep->type != oper_sep)
            error(&sep->pos);
        node->son[1] = expression(lexer);
        Token *rpar = nextToken(lexer);
        if (rpar->type != oper_rpar)
            error(&sep->pos);
        break;
    default:
        break;
    }

    return node;
}

treenode_t *factor(Lexer *lexer)
{
    treenode_t *node = operand(lexer);
    treenode_t *nodePower = initNode();
    Token *token = nextToken(lexer);

    if (token->type == oper_pow)
    {
        nodePower->son[0] = node;
        nodePower->pos = token->pos;
        nodePower->son[1] = operand(lexer);
        return nodePower;
    }

    rewindLexer(lexer, token);
    return node;
}

treenode_t *term(Lexer *lexer)
{
    treenode_t *node = factor(lexer);
    printf("GOT FACTOR\n");
    Token *next = nextToken(lexer);
    treenode_t *prod;

    while (next->type == oper_mul || next->type == oper_div)
    {
        prod = initNode();
        prod->type = next->type;
        prod->pos = next->pos;
        prod->son[0] = node;
        prod->son[1] = operand(lexer);
        node = prod;
        next = nextToken(lexer);
    }
    rewindLexer(lexer, next);
    return node;
}

treenode_t *expression(Lexer *lexer)
{
    treenode_t *node = term(lexer);
    Token *next = nextToken(lexer);
    treenode_t *prod;

    while (next->type == oper_add || next->type == oper_sub)
    {
        prod = initNode();
        prod->type = next->type;
        prod->pos = next->pos;
        prod->son[0] = node;
        prod->son[1] = operand(lexer);
        node = prod;
        next = nextToken(lexer);
    }

    rewindLexer(lexer, next);
    printf("END EXPRESSION\n");
    return node;
}

void printToken(Token *token)
{
    if (token->entry)
    {
        printf("type: %d, name: %s, keyw: %d, pos: %d, %d, i: %d\n", token->type, token->entry->name, token->entry->type, token->pos.line, token->pos.col, token->i);
    }
    else
    {
        printf("type: %d, value: %f, pos: %d, %d, i: %d\n", token->type, token->value, token->pos.line, token->pos.col, token->i);
    }
}

treenode_t *statement(Lexer *lexer)
{
    printf("start statement\n");
    treenode_t *node = initNode();
    Token *token = nextToken(lexer);
    node->pos = token->pos;

    switch (token->type)
    {
    case keyw_end:
        return NULL;
        break;
    case keyw_walk:
    { // check if walk back or not with next token
        node->type = keyw_walk;
        Token *secondToken = nextToken(lexer);
        if (secondToken->type == keyw_back)
        {
            node->d.walk = keyw_back;
            node->son[0] = expression(lexer);
        }
        else if (secondToken->type == keyw_home || secondToken->type == keyw_mark)
        {
            node->d.walk = secondToken->type;
        }
        else
        {
            node->d.walk = keyw_walk;
            rewindLexer(lexer, secondToken);
            node->son[0] = expression(lexer);
        }
        break;
    }
    case keyw_jump:
    {
        node->type = keyw_jump;
        // check if walk back or not with next token
        Token *secondToken = nextToken(lexer);
        if (secondToken->type == keyw_back)
        {
            node->d.walk = keyw_back;
            node->son[0] = expression(lexer);
        }
        else if (secondToken->type == keyw_home || secondToken->type == keyw_mark)
        {
            node->d.walk = secondToken->type;
        }
        else
        {
            node->d.walk = keyw_walk;
            rewindLexer(lexer, secondToken);
            node->son[0] = expression(lexer);
        }
        break;
    }
    case keyw_turn:
    {
        Token *secondToken = nextToken(lexer);
        if (secondToken->type == keyw_left)
        {
            node->type = keyw_left;
            node->son[0] = expression(lexer);
        }
        else if (secondToken->type == keyw_right)
        {
            node->type = keyw_right;
            node->son[0] = expression(lexer);
        }
        else
        {
            node->type = keyw_right;
            rewindLexer(lexer, secondToken);
            node->son[0] = expression(lexer);
        }
        break;
    }
    case keyw_direction:
    {
        node->type = keyw_direction;
        printf("BEFORE EXPRESSION IN DIRECTION\n");
        node->son[0] = expression(lexer);
        break;
    }
    case keyw_color:
    {
        node->type = keyw_color;
        // red
        node->son[0] = expression(lexer);
        // consume separator
        Token *comma1 = nextToken(lexer);
        printf("after comma 1\n");
        if (comma1->type != oper_sep)
        {
            error(&comma1->pos);
        }
        // green
        node->son[1] = expression(lexer);
        Token *comma2 = nextToken(lexer);
        if (comma2->type != oper_sep)
        {
            error(&comma1->pos);
        }
        // blue
        node->son[2] = expression(lexer);
        break;
    }
    case keyw_clear:
    {
        node->type = keyw_clear;
        break;
    }
    case keyw_stop:
    {
        node->type = keyw_stop;
        break;
    }
    case keyw_finish:
    {
        node->type = keyw_finish;
        break;
    }
    case keyw_path:
    {
        printf("in path");
        Token *tokenName = nextToken(lexer);
        node->type = oper_lpar;
        node->pos = tokenName->pos;
        node->d.p_name = tokenName->entry;

        // get the arguments if any
        Token *lpar = nextToken(lexer);
        if (lpar->type == oper_lpar)
        {
            Token *rpar = nextToken(lexer);
            if (rpar->type != oper_rpar)
            {
                rewindLexer(lexer, rpar);
                int index = 0;
                printf("before expression\n");
                node->son[index] = expression(lexer);
                printf("after expression\n");
                rpar = nextToken(lexer); // comma
                while (rpar->type == oper_sep)
                {
                    index++;
                    node->son[index] = expression(lexer);
                    rpar = nextToken(lexer); // comma or right parenthesis
                }
                // no more comma, we should have the right parenthesis now
            }
        }
        else rewindLexer(lexer, lpar); // no parenthesis for path call is ok

        break;
    }
    case keyw_store:
    {
        node->type = keyw_store;
        node->son[0] = expression(lexer);
        Token *in = nextToken(lexer);
        printToken(in);
        if (in->type != keyw_in)
        {
            error(&in->pos);
        }
        Token *varName = nextToken(lexer);
        node->d.p_name = varName->entry;
        // check if varName in name_tab???
        break;
    }
    case keyw_add:
    {
        node->type = keyw_add;
        node->son[0] = expression(lexer);
        Token *to = nextToken(lexer);
        if (to->type != keyw_to)
            error(&to->pos);
        Token *varName = nextToken(lexer);
        node->d.p_name = varName->entry;
        break;
    }
    case keyw_sub:
    {
        node->type = keyw_add;
        node->son[0] = expression(lexer);
        Token *from = nextToken(lexer);
        if (from->type != keyw_from)
            error(&from->pos);
        Token *varName = nextToken(lexer);
        node->d.p_name = varName->entry;
        break;
    }
    case keyw_mul:
    {
        node->type = keyw_mul;
        node->son[0] = expression(lexer);
        Token *by = nextToken(lexer);
        if (by->type != keyw_by)
            error(&by->pos);
        Token *varName = nextToken(lexer);
        node->d.p_name = varName->entry;
        break;
    }
    case keyw_div:
    {
        node->type = keyw_div;
        node->son[0] = expression(lexer);
        Token *by = nextToken(lexer);
        if (by->type != keyw_by)
            error(&by->pos);
        Token *varName = nextToken(lexer);
        node->d.p_name = varName->entry;
        break;
    }
    case keyw_mark:
    {
        node->type = keyw_mark;
        printf("GOT MARKKKKKK\n");
        break;
    }
    case keyw_if:
    {
        node->type = keyw_if;
        node->son[0] = condition(lexer);
        Token *then = nextToken(lexer);
        if (then->type != keyw_then)
            error(&then->pos);
        node->son[1] = statement(lexer);
        treenode_t *last = node->son[1];
        Token *elseOrEnd = nextToken(lexer);

        while (elseOrEnd->type == keyw_else || elseOrEnd->type == keyw_end)
        {
            rewindLexer(lexer, elseOrEnd);
            last->next = statement(lexer);
            last = last->next;
            elseOrEnd = nextToken(lexer);
        }

        if (elseOrEnd->type == keyw_else)
        {
            node->son[2] = statement(lexer);
            Token *end = nextToken(lexer);
            treenode_t *last = node->son[2];

            while (end->type != keyw_endif)
            {
                rewindLexer(lexer, end);
                last->next = statement(lexer);
                last = last->next;
                end = nextToken(lexer);
            }
        }
        else
        {
            node->son[2] = NULL;
        }
        break;
    }
    case keyw_do:
    {
        node->type = keyw_do;
        node->son[0] = expression(lexer);
        Token *times = nextToken(lexer);
        if (times->type != keyw_times)
            error(&times->pos);
        node->son[1] = statement(lexer);
        treenode_t *last = node->son[1];
        Token *done = nextToken(lexer);
        while (done->type != keyw_done)
        {
            rewindLexer(lexer, done);
            last->next = statement(lexer);
            last = last->next;
            done = nextToken(lexer);
        }
        break;
    }
    case keyw_counter:
    {
        node->type = keyw_counter;
        Token *varName = nextToken(lexer);
        node->d.p_name = varName->entry;
        Token *from = nextToken(lexer);
        if (from->type != keyw_from)
            error(&from->pos);
        node->son[0] = expression(lexer);

        Token *toOrDownTo = nextToken(lexer);
        if (toOrDownTo->type == keyw_to)
        {
            node->son[1] = expression(lexer);
            node->son[2] = NULL;
        }
        else if (toOrDownTo->type == keyw_downto)
        {
            node->son[1] = NULL;
            node->son[2] = expression(lexer);
        }
        else
            error(&toOrDownTo->pos);

        Token *stepOrDo = nextToken(lexer);
        if (stepOrDo->type == keyw_step)
        {
            node->son[3] = expression(lexer);
            stepOrDo = nextToken(lexer);
        }
        else
        {
            node->son[3] = NULL;
        }

        if (stepOrDo->type != keyw_do)
            error(&stepOrDo->pos);
        
        node->son[4] = statement(lexer);

        treenode_t *last = node->son[4];
        Token *done = nextToken(lexer);
        while (done->type != keyw_done)
        {
            rewindLexer(lexer, done);
            last->next = statement(lexer);
            last = last->next;
            done = nextToken(lexer);
        }

        break;
    }
    case keyw_while:
    {
        node->type = keyw_while;
        node->son[0] = condition(lexer);
        Token *tokenDo = nextToken(lexer);
        node->son[1] = statement(lexer);
        treenode_t *last = node->son[1];
        Token *done = nextToken(lexer);
        while (done->type != keyw_done)
        {
            rewindLexer(lexer, done);
            last->next = statement(lexer);
            last = last->next;
            done = nextToken(lexer);
        }
        break;
    }
    case keyw_repeat:
    {
        node->type = keyw_repeat;
        node->son[1] = statement(lexer);
        treenode_t *last = node->son[1];
        Token *until = nextToken(lexer);
        while (until->type != keyw_done)
        {
            rewindLexer(lexer, until);
            last->next = statement(lexer);
            last = last->next;
            until = nextToken(lexer);
        }
        node->son[0] = condition(lexer);
        break;
    }
    default:
        break;
    }

    printf("NOOODE, type: %d, pos: %d, %d\n", node->type, node->pos.line, node->pos.col);
    return node;
}

treenode_t *statements(Lexer *lexer, type_t condition)
{
    treenode_t *node = statement(lexer);
    treenode_t *lastNode = node;
    Token *token = nextToken(lexer);
    while (token->type != condition)
    {
        rewindLexer(lexer, token);
        lastNode->next = statement(lexer);
        lastNode = lastNode->next;
    }

    return node;
}

funcdef_t *initFunction()
{
    funcdef_t *funcDef = calloc(1, sizeof(funcdef_t));
    funcDef->body = NULL;
    funcDef->ret = NULL;
    funcDef->params;

    return funcDef;
}

void makePathFunction(Lexer *lexer)
{
    Token *tokenName = nextToken(lexer);
    treenode_t *nodePath = initNode();

    tokenName->entry->type = name_path;

    funcdef_t *funcDef = initFunction();
    tokenName->entry->d.func = funcDef;

    // get the arguments if any
    Token *lpar = nextToken(lexer);

    if (lpar->type == oper_lpar)
    {
        Token *rpar = nextToken(lexer);
        if (rpar->type != oper_rpar)
        {
            int index = 0;
            printf("starting arguments\n");
            funcDef->params;
            funcDef->params[index] = rpar->entry;
            rpar = nextToken(lexer); // comma
            while (rpar->type == oper_sep)
            {
                index++;
                rpar = nextToken(lexer); // argument
                funcDef->params[index] = rpar->entry;
                rpar = nextToken(lexer); // comma or right parenthesis
            }
            // no more comma, we should have the right parenthesis now
        }
    }
    else
        rewindLexer(lexer, lpar); // no parenthesis for path call is ok
    // body -> statements
    // 1 statement minimum
    treenode_t *start = statement(lexer);
    funcDef->body = start;
    treenode_t *last = start;
    Token *endPath = nextToken(lexer);

    while (endPath->type != keyw_endpath)
    {
        last->next = statement(lexer);
        last = last->next;
        endPath = nextToken(lexer);
    }
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Aufruf: %s Programm-Datei [Zahlen ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (argc > 11)
    {
        fprintf(stderr, "Aufruf: %s Programm-Datei [Zahlen ...]\n"
                        "Höchstens 9 Zahlen!\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    src_file = fopen(argv[1], "r");
    if (src_file == NULL)
    {
        fprintf(stderr, "%s: Fehler beim Öffnen von %s zum Lesen: %s\n",
                argv[0], argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }

    prog_name = argv[0];

    //////////////////////////
    // my stuff
    //////////////////////////

    char *src;
    // copy all data from src_file to src
    fseek(src_file, 0, SEEK_END);
    long lSize = ftell(src_file);
    rewind(src_file);
    src = calloc(1, lSize + 1);
    if (1 != fread(src, lSize, 1, src_file))
        fclose(src_file), free(src), fputs("entire read fails", stderr), exit(1);
    fclose(src_file);

    Lexer lexer;
    lexer_init(&lexer, src);

    printf("starting lexer\n");

    treenode_t *start = NULL;

    Token *endToken = nextToken(&lexer);
    printf("first token read\n");
    bool endFound = false;

    if (endToken->type == keyw_path)
    {
        printf("making functino path\n");
        makePathFunction(&lexer);
        endToken = nextToken(&lexer);
    }

    printf("MADE PATH");
    
    while (endToken->type != tok_bofeof && !endFound)
    {
        start = statement(&lexer);
        treenode_t *last = start;
        endToken = nextToken(&lexer);

        while (!endFound)
        {
            rewindLexer(&lexer, endToken);
            last->next = statement(&lexer);
            last = last->next;
            endToken = nextToken(&lexer);
            if (endToken->type == keyw_end)
            {
                endFound = true;
                printf("END FOUND\n");
            }
        }
    }
    /////////////////////////////////

    evaluate(start, argc - 2, &(argv[2]));

    exit(EXIT_SUCCESS);
}