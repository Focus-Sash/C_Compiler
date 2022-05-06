#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
char *user_input;

bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
    //EOFの場合は0
};

//このグローバル変数に、入力をトークナイズした列を格納する
Token *token;

Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
Token *tokenize(char *p);

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_NUM,
} NodeKind;

typedef struct Node Node;

//構文木のノードを表す構造体
struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();

Node *equality();
Node *relation();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *expr();
Node *equality();
Node *relation();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void gen(Node *node);
