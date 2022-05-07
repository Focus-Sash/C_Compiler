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

int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
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

//変数名が先頭に来ているとき、ポインタを進める
//あとで変数名を2文字以上にするために、文字数のチェックは入れない
//演算子などではそのトークンの情報は自明だったが、変数ではオフセットの情報が必要なので、変数名の情報が入ったトークンを返す
//expect_numberと同様の実装になる
Token *consume_ident() {
    if (token->kind != TK_IDENT) {
        return NULL;
    }
    Token *res = token;
    token = token->next;
    return res;
}

bool consume_return() {
    if(token->kind != TK_RETURN) {
        return false;
    }
    token = token->next;
    return true;
}

bool consume_if() {
    if(token->kind != TK_IF) {
        return false;
    }
    token = token->next;
    return true;
}

bool consume_else() {
    if(token->kind != TK_ELSE) {
        return false;
    }
    token = token->next;
    return true;
}

bool consume_while() {
    if(token->kind != TK_WHILE) {
        return false;
    }
    token = token->next;
    return true;
}

bool consume_for() {
    if(token->kind != TK_FOR) {
        return false;
    }
    token = token->next;
    return true;
}

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next) {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
            return var;
        }
    }
    return NULL;
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
        if (strchr("+-*/()<>=;", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        if(strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }

        if(strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }

        if(strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }

        if(strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            cur = new_token(TK_FOR, cur, p, 3);
            p += 3;
            continue;
        }

        //変数を表すトークン
        if (('a' <= *p && *p <= 'z') ||
            ('A' <= *p && *p <= 'Z') ||
            (*p == '_')) {
            int len = 0;
            char *tmp = p;
            while (is_alnum(*p)) {
                len++;
                p++;
            }
            cur = new_token(TK_IDENT, cur, tmp, len);
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
