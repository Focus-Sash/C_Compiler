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

int is_alnum(char c);

bool at_eof();

//変数は単方向連結リストで表す

typedef struct LVar LVar;

struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
};

LVar *locals;



typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_RETURN,
    TK_IF,
    TK_ELSE,
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

Token *consume_ident();
bool consume_return();
bool consume_if();
bool consume_else();


//このグローバル変数に、入力をトークナイズした列を格納する
Token *token;

Token *new_token(TokenKind kind, Token *cur, char *str, int len);

bool startswith(char *p, char *q);

Token *tokenize(char *p);

LVar *find_lvar(Token *tok);

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_LVAR, // ローカル変数
    ND_ASSIGN, // =
    ND_RETURN,
    ND_IF,
    ND_ELSE,
    ND_IF_ELSE,
    ND_NUM,
} NodeKind;

typedef struct Node Node;

//構文木のノードを表す構造体
struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;    // kindがND_NUMの場合のみ
    int offset; // kindがND_LVARの場合のみ　ベースポインタからのオフセット
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);

Node *new_node_num(int val);

void program();

Node *stmt();

Node *expr();

Node *assign();

Node *equality();

Node *relation();

Node *add();

Node *mul();

Node *unary();

Node *primary();

void gen_lval(Node *node);

void gen(Node *node);

Node *code[100];

int counter;

#define dump() fprintf(stderr, "%d行目を実行しています\n", __LINE__)
