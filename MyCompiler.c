#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

char *user_input;

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}


//特定の文字列が先頭に来ているとき、ポインタを進める
bool consume(char *op) {
    //int memcmp(void *p, void *q, int n)はp, qの最初のn文字を比較する
    //p > qのとき正、p = qのとき0、p < qのとき負になる
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

//特定の文字列が先頭に来ているかチェックし、ポインタを進める
void expect(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)) {
        error_at(token->str, "expected \"%s\"", op);
    }
    token = token->next;
}

//数字が先頭に来ているかチェックする
int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}


Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

//入力をトークンの列に変換する
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        //空白はスキップする
        if (isspace(*p)) {
            p++;
            continue;
        }

        //2文字の演算子
        //長い演算子から先に処理しないとバグる
        if (startswith(p, "==") || startswith(p, "!=") ||
            startswith(p, "<=") || startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        //char *strchr(char *s, int c)はsにcが含まれているかを検索する関数
        //含まれていれば最初のその文字へのポインタを、含まれていなければNULLを返す
        if (strchr("+-*/()<>", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            //long strtol(char *s, char **endptr, int base)は
            //文字列sをbase進数でlongに変換して返却する
            //変換できた最後の文字の次の文字を指すポインタをendptrに格納する(今回は現在読んでいる数字の次)
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(token->str, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

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

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}


//次のEBNFで表されるトークン列をパースする
// expr     = equality
// equality = relation("==" relation | "!=" relation)*
// relation = add ("<" add | "<=" add | ">" add | ">=" add)*
// add      = mul ("+" mul | "-" mul)*
// mul      = unary( "*" unary | "/" unary)*
// unary    = ( "+" | "-" )? primary
// primary  = num | "(" expr ")"

Node *expr();

Node *equality();

Node *relation();

Node *add();

Node *mul();

Node *unary();

Node *primary();

//それぞれ、対応する種類のノードを根とする木を構築し、根へのポインタを返す
Node *expr() {
    return equality();
}

Node *equality() {
    Node *node = relation();
    for (;;) {
        if (consume("==")) {
            node = new_node(ND_EQ, node, relation());
        } else if (consume("!=")) {
            node = new_node(ND_NE, node, relation());
        } else {
            return node;
        }
    }
}

Node *relation() {
    Node *node = add();
    for (;;) {
        if (consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if (consume("<=")) {
            node = new_node(ND_LE, node, add());
        } else if (consume(">")) {
            node = new_node(ND_LT, add(), node);
        } else if (consume(">=")) {
            node = new_node(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();
    for (;;) {
        if (consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}


Node *mul() {
    Node *node = unary();
    for (;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node *unary() {
    if (consume("+")) {
        return primary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    } else {
        return primary();
    }
}

Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    return new_node_num(expect_number());
}


//tokenを読みながら構文木を構築し、nodeの左右の子の値をスタックにpushし、nodeを根とする部分木の値を計算し、raxに格納するアセンブラを出力する
void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    // 演算子の両辺の値をpopしてrdiとraxに格納する
    printf("  pop rdi\n");
    printf("  pop rax\n");


    switch (node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            // cqoはraxの値を128ビットに拡張し、上(0000)をrdx、下(rax)をraxに格納する
            // idivはrdxとraxを合わせて128ビット整数とみなして、
            // 引数(今回はrdi)で割った値の商をraxに、余りをrdxにセットする
            break;
        case ND_EQ:
            // cmpは2つの引数の比較結果をフラグレジスタという特別なレジスタに格納する
            // seteは直前のcmpで調べた2つのレジスタの値が同じだったときに引数のレジスタに1を、異なっていたら0をセットする
            // alはraxの下位ビットの別名(seteは8ビットレジスタしか引数に取れない)
            // movzbによって、raxの上位56ビットをゼロクリアする
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NE:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LT:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NUM:
            break;
    }

    printf("  push rax\n");
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }

    user_input = argv[1];

    //グローバル変数tokenに、入力された文字列の最初の文字へのポインタを与える
    token = tokenize(argv[1]);

    //全体がexpressionであるという前提のもと、構文木の根となるノードへのポインタをnodeに与える
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    //nodeを根とする構文木を構築し、その値を計算するアセンブラを出力する
    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}