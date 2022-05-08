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



bool at_eof() {
    return token->kind == TK_EOF;
}


Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    tok->id = token_count;
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

    fprintf(stderr,"\nToken List\n");

    while (*p) {
        //空白はスキップする
        if (isspace(*p)) {
            p++;
            continue;
        }

        token_count++;

        //2文字の演算子
        //長い演算子から先に処理しないとバグる
        if (startswith(p, "==") || startswith(p, "!=") ||
            startswith(p, "<=") || startswith(p, ">=")) {
            fprintf(stderr,"#%d : %c\n", token_count, *p);
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        //char *strchr(char *s, int c)はsにcが含まれているかを検索する関数
        //含まれていれば最初のその文字へのポインタを、含まれていなければNULLを返す
        if (strchr("+-*/()<>=;{}", *p)) {
            fprintf(stderr,"#%d : %c\n", token_count, *p);
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            fprintf(stderr,"#%d : return\n", token_count);
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        if(strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            fprintf(stderr,"#%d : if\n", token_count);
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }

        if(strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            fprintf(stderr,"#%d : else\n", token_count);
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }

        if(strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            fprintf(stderr,"#%d : while\n", token_count);
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }

        if(strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            fprintf(stderr,"#%d : for\n", token_count);
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
            fprintf(stderr,"#%d :", token_count);
            while (is_alnum(*p)) {
                len++;
                fprintf(stderr, "%c", *p);
                p++;
            }
            fprintf(stderr, "\n");
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
            fprintf(stderr,"#%d : %d\n", token_count, cur->val);
            cur->len = p - q;
            continue;
        }

        fprintf(stderr, "トークナイズできません\n");
        exit(1);
    }

    new_token(TK_EOF, cur, p, 0);

    fprintf(stderr, "\nInput successfully tokenized.\n\n");

    return head.next;
}
