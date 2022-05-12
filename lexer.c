#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "turtle.h"

void error(srcpos_t *pos)
{
    printf("Error! -> line: %d, column: %d\n", pos->line, pos->col);
    exit(EXIT_FAILURE);
}

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

void rewindLexer(Lexer *lexer, srcpos_t pos)
{
    lexer->pos = pos;
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

treenode_t *condition(Lexer *lexer)
{
    Token *token = nextToken(lexer);
    bool withParenthesis = false;
    if (token->type != oper_lpar)
    {
        rewindLexer(lexer, token->pos);
    }
    else withParenthesis = true;
    token = nextToken(lexer);

    treenode_t *node = initNode();

    if (token->type == keyw_not)
    {
        node->type = keyw_not;
        node->son[0] = operand(lexer);
        token = nextToken(lexer);
    }

    return node;
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
        node->son[0] = expression(lexer);
        break;
    case oper_const:
        node = initNode();
        node->type = oper_const;
        node->d.val = token->value;
        break;
    case oper_abs:
    {
        node = initNode();
        node->type = oper_abs;
        node->son[0] = expression(lexer);
        Token *endToken = nextToken(lexer);
        if (endToken->type != oper_abs) error(&endToken->pos);
        break;
    }
    case oper_lpar:
        node = initNode();
        node = expression(lexer);
        Token *endToken = nextToken(lexer);
        if (endToken->type != oper_rpar) error(&endToken->pos);
        break;
    case name_math_rand:
        node = initNode();
        node->type = oper_lpar;
        node->d.p_name = token->entry;
        // consume (
        Token *lpar = nextToken(lexer);
        if (lpar->type != oper_lpar) error(&lpar->pos);
        node->son[0] = expression(lexer);
        Token *sep = nextToken(lexer);
        if (sep->type != oper_sep) error(&sep->pos);
        node->son[1] = expression(lexer);
        Token *rpar = nextToken(lexer);
        if (rpar->type != oper_rpar) error(&sep->pos);
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
        nodePower->son[1] = operand(lexer);
        return nodePower;
    }

    rewindLexer(lexer, token->pos);
    return node;
}

treenode_t *term(Lexer *lexer)
{
    treenode_t *node = factor(lexer);
    Token *next = nextToken(lexer);
    treenode_t *prod;

    while (next->type == oper_mul || next->type == oper_div)
    {
        prod = initNode();
        prod->type = next->type;
        prod->son[0] = node;
        prod->son[1] = operand(lexer);
        node = prod;
        next = nextToken(lexer);
    }
    rewindLexer(lexer, next->pos);
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
        prod->son[0] = node;
        prod->son[1] = operand(lexer);
        node = prod;
        next = nextToken(lexer);
    }

    rewindLexer(lexer, next->pos);
    return node;
}

treenode_t *statement(Lexer *lexer)
{
    treenode_t *node = initNode();
    Token *token = nextToken(lexer);

    switch (token->type)
    {
        case keyw_walk:
            {// check if walk back or not with next token
            node->type = keyw_walk;
            Token *secondToken = nextToken(lexer);
            if (secondToken->type == keyw_back)
            {
                node->d.walk = keyw_back;
                node->son[0] = expression(lexer);
            }
            else if (secondToken->type == keyw_home
                || secondToken->type == keyw_mark)
            {
                node->d.walk = secondToken->type;
            }
            else
            {
                node->d.walk = keyw_walk;
                rewindLexer(lexer, secondToken->pos);
                node->son[0] = expression(lexer);
            }
            break;}
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
            else if (secondToken->type == keyw_home
                || secondToken->type == keyw_mark)
            {
                node->d.walk = secondToken->type;
            }
            else
            {
                node->d.walk = keyw_jump;
                rewindLexer(lexer, secondToken->pos);
                node->son[0] = expression(lexer);
            }
            break;
            }
        case keyw_turn:
            {Token *secondToken = nextToken(lexer);
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
                rewindLexer(lexer, secondToken->pos);
                node->son[0] = expression(lexer);
            }
            break;}
        case keyw_direction:
            {node->type = keyw_direction;
            node->son[0] = expression(lexer);
            break;}
        case keyw_color:
            {node->type = keyw_color;
            // red
            node->son[0] = expression(lexer);
            // consume separator
            Token *comma1 = nextToken(lexer);
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
            break;}
        case keyw_clear:
            {node->type = keyw_clear;
            break;}
        case keyw_stop:
            {node->type = keyw_stop;
            break;}
        case keyw_finish:
            {node->type = keyw_finish;
            break;}
        case keyw_path:
            {Token *pathName = nextToken(lexer);
            // how to deal with path???
            break;}
        case keyw_store:
            {node->type = keyw_store;
            node->son[0] = expression(lexer);
            Token *in = nextToken(lexer);
            if (in->type != keyw_in)
            {
                error(&in->pos);
            }
            Token *varName = nextToken(lexer);
            node->d.p_name = varName->entry;
            // check if varName in name_tab???
            break;}
        case keyw_add:
            {node->type = keyw_add;
            node->son[0] = expression(lexer);
            Token *to = nextToken(lexer);
            if (to->type != keyw_to) error(&to->pos);
            Token *varName = nextToken(lexer);
            node->d.p_name = varName->entry;
            break;}
        case keyw_sub:
            {node->type = keyw_add;
            node->son[0] = expression(lexer);
            Token *from = nextToken(lexer);
            if (from->type != keyw_from) error(&from->pos);
            Token *varName = nextToken(lexer);
            node->d.p_name = varName->entry;
            break;}
        case keyw_mul:
            {node->type = keyw_mul;
            node->son[0] = expression(lexer);
            Token *by = nextToken(lexer);
            if (by->type != keyw_by) error(&by->pos);
            Token *varName = nextToken(lexer);
            node->d.p_name = varName->entry;
            break;}
        case keyw_div:
            {node->type = keyw_div;
            node->son[0] = expression(lexer);
            Token *by = nextToken(lexer);
            if (by->type != keyw_by) error(&by->pos);
            Token *varName = nextToken(lexer);
            node->d.p_name = varName->entry;
            break;}
        case keyw_mark:
            {node->type = keyw_mark;
            break;}
        case keyw_if:
            {node->type = keyw_if;
            node->son[0] = condition(lexer);
            Token *then = nextToken(lexer);
            if (then->type != keyw_then) error(&then->pos);
            node->son[1] = statement(lexer);
            treenode_t *last = node->son[1];
            Token *elseOrEnd = nextToken(lexer);

            while (elseOrEnd->type == keyw_else || elseOrEnd->type == keyw_end)
            {
                rewindLexer(lexer, elseOrEnd->pos);
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
                    rewindLexer(lexer, end->pos);
                    last->next = statement(lexer);
                    last = last->next;
                    end = nextToken(lexer);
                }
            }
            else
            {
                node->son[2] = NULL;
            }
            break;}
        case keyw_do:
            {node->type = keyw_do;
            node->son[0] = expression(lexer);
            Token *times = nextToken(lexer);
            if (times->type != keyw_times) error(&times->pos);
            node->son[1] = statement(lexer);
            treenode_t *last = node->son[1];
            Token *done = nextToken(lexer);
            while (done->type != keyw_done)
            {
                rewindLexer(lexer, done->pos);
                last->next = statement(lexer);
                last = last->next;
                done = nextToken(lexer);
            }
            break;}
        case keyw_counter:
            {node->type = keyw_counter;
            Token *varName = nextToken(lexer);
            node->d.p_name = varName->entry;
            Token *from = nextToken(lexer);
            if (from->type != keyw_from) error(&from->pos);
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
            else error(&toOrDownTo->pos);

            Token *stepOrDo = nextToken(lexer);
            if (stepOrDo->type == keyw_step)
            {
                node->son[3] = expression(lexer);
            }
            else if (stepOrDo->type == keyw_do)
            {
                node->son[3] = NULL;
                node->son[4] = statement(lexer);

                treenode_t *last = node->son[4];
                Token *done = nextToken(lexer);
                while (done->type != keyw_done)
                {
                    rewindLexer(lexer, done->pos);
                    last->next = statement(lexer);
                    last = last->next;
                    done = nextToken(lexer);
                }
            }
            else error(&stepOrDo->pos);
            break;}
        case keyw_while:
            {node->type = keyw_while;
            node->son[0] = condition(lexer);
            node->son[1] = statement(lexer);
            treenode_t *last = node->son[1];
            Token *done = nextToken(lexer);
            while (done->type != keyw_done)
            {
                rewindLexer(lexer, done->pos);
                last->next = statement(lexer);
                last = last->next;
                done = nextToken(lexer);
            }
            break;}
        case keyw_repeat:
            {node->type = keyw_repeat;
            node->son[1] = statement(lexer);
            treenode_t *last = node->son[1];
            Token *until = nextToken(lexer);
            while (until->type != keyw_done)
            {
                rewindLexer(lexer, until->pos);
                last->next = statement(lexer);
                last = last->next;
                until = nextToken(lexer);
            }
            node->son[0] = condition(lexer);
            break;}
        default:
            break;
    }
}

treenode_t *statements(Lexer *lexer, type_t condition)
{
    treenode_t *node = statement(lexer);
    treenode_t *lastNode = node;
    Token *token = nextToken(lexer);
    while (token->type != condition)
    {
        rewindLexer(lexer, token->pos);
        lastNode->next = statement(lexer);
        lastNode = lastNode->next;
    }
    
    return node;
}

int main(int argc, const char *argv[])
{
    src_file = fopen(argv[1], "r");
    char *src;
    // copy all data from src_file to src
    fseek(src_file, 0, SEEK_END);
    printf("%s\n", "hello world");
    long lSize = ftell(src_file);
    rewind(src_file);
    src = calloc(1, lSize + 1);
    if (1 != fread(src, lSize, 1, src_file))
        fclose(src_file), free(src), fputs("entire read fails", stderr), exit(1);
    fclose(src_file);

    Lexer lexer;
    lexer_init(&lexer, src);

    printf("starting lexer\n");

    while (lexer.c != '\0')
    {
        Token *token = nextToken(&lexer);
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