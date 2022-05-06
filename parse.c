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

        //変数を表すトークン
        if ('a' <= *p && *p <= 'z') {
            int len = 0;
            char *tmp = p;
            while('a' <= *p && *p <= 'z'){
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
//program    = stmt*
//stmt       = expr ";"
//expr       = assign
//assign     = equality ("=" assign)?
//equality   = relation ("==" relation | "!=" relation)*
//relation = add ("<" add | "<=" add | ">" add | ">=" add)*
//add        = mul ("+" mul | "-" mul)*
//mul        = unary ("*" unary | "/" unary)*
//unary      = ("+" | "-")? primary
//primary    = num | ident | "(" expr ")"



//グローバル配列codeに、セミコロンで区切られたstmtの根を表すノードを格納する
//複数の文からなるプログラムを書くために、二分木ではなくN分木にする(左右の子だけでなく配列を使う)
//変数の宣言などに対応する値を返さない「void型のノード」が必要になる
void program() {
    int i = 0;
    while (!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

//それぞれ、対応する種類のノードを根とする木を構築し、根へのポインタを返す
Node *stmt() {
    Node *node = expr();
    //;は区切りの意味しかないので、expectで進めるだけにしておく
    expect(";");
    return node;
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *node = equality();
    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
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


//終端記号として変数名が加わった
Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();

    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->offset = (tok->str[0] - 'a' + 1) * 8;
        return node;
    }

    return new_node_num(expect_number());
}