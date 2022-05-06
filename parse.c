#include "compiler.h"

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

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