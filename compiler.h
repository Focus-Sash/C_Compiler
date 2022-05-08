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
    TK_WHILE,
    TK_FOR,
    TK_EOF,
} TokenKind;

char *token_name[9];

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
    int id; //何番目のトークンか
};

Token *consume_ident();

bool consume_return();

bool consume_if();

bool consume_else();

bool consume_while();

bool consume_for();


//このグローバル変数に、入力をトークナイズした列を格納する
Token *token;
int token_count;
int parse_count;

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
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,
    ND_BLANK, //左右の子を持つだけのノード 2分木を使ってN分木を作るために実装
    ND_NUM,
} NodeKind;

typedef struct Node Node;
typedef struct vector vector;
typedef struct cell cell;

struct cell {
    Node *stmt;
    cell *next;
};

struct vector {
    cell *head;
    cell *tail;
};

//構文木のノードを表す構造体
struct Node {
    NodeKind kind;

    Node *lhs;
    Node *rhs;

    Node *if_cond;  // kindがND_IFの場合のみ
    Node *if_true;  // kindがND_IFの場合のみ
    Node *if_false; // kindがND_IFの場合のみ

    Node *for_init;   // kindがND_FORの場合のみ
    Node *for_cond;   // kindがND_FORの場合のみ
    Node *for_upd;    // kindがND_FORの場合のみ
    Node *for_content;// kindがND_FORの場合のみ

    vector compound;  // kindがND_BLOCKの場合のみ
    int val;      // kindがND_NUMの場合のみ
    int offset;   // kindがND_LVARの場合のみ　ベースポインタからのオフセット
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);

Node *blank_node();

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

#define dump() fprintf(stderr, "%sの%d行目を実行しています\n", __FILE__, __LINE__)
