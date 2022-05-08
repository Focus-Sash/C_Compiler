#include "header.h"

void parse_log() {
    fprintf(stderr, "  Consuming token #%d of type %s.\n", token->id, token_name[token->kind]);
    return;
}

//特定の文字列が先頭に来ているとき、ポインタを進める
bool consume(char *op) {
    //int memcmp(void *p, void *q, int n)はp, qの最初のn文字を比較する
    //p > qのとき正、p = qのとき0、p < qのとき負になる
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)) {
        return false;
    }
    parse_log();
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
    parse_log();
    token = token->next;
    return res;
}

bool consume_return() {
    if(token->kind != TK_RETURN) {
        return false;
    }
    parse_log();
    token = token->next;
    return true;
}

bool consume_if() {
    if(token->kind != TK_IF) {
        return false;
    }
    parse_log();
    token = token->next;
    return true;
}

bool consume_else() {
    if(token->kind != TK_ELSE) {
        return false;
    }
    parse_log();
    token = token->next;
    return true;
}

bool consume_while() {
    if(token->kind != TK_WHILE) {
        return false;
    }
    parse_log();
    token = token->next;
    return true;
}

bool consume_for() {
    if(token->kind != TK_FOR) {
        return false;
    }
    parse_log();
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
    parse_log();
    token = token->next;
}

//数字が先頭に来ているかチェックする
int expect_number() {
    if (token->kind != TK_NUM) {
//        error_at(token->str, "数ではありません");
        dump();
        exit(1);
    }
    int val = token->val;
    parse_log();
    token = token->next;
    return val;
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

Node *blank_node() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BLANK;
    return node;
}

char *token_name[9] = {
    "TK_RESERVED",
    "TK_IDENT",
    "TK_NUM",
    "TK_RETURN",
    "TK_IF",
    "TK_ELSE",
    "TK_WHILE",
    "TK_FOR",
    "TK_EOF",
};

char *node_name[17] = {
    "ND_ADD",
    "ND_SUB",
    "ND_MUL",
    "ND_DIV",
    "ND_EQ",
    "ND_NE",
    "ND_LT",
    "ND_LE",
    "ND_LVAR", // ローカル変数
    "ND_ASSIGN", // =
    "ND_RETURN",
    "ND_IF",
    "ND_WHILE",
    "ND_FOR",
    "ND_BLOCK",
    "ND_BLANK", //左右の子を持つだけのノード 2分木を使ってN分木を作るために実装
    "ND_NUM",
};


//次のEBNFで表されるトークン列をパースする
//program    = stmt*
//stmt       = expr ";"
//           | "{" stmt* "}"
//           | "if" "(" expr ")" stmt ("else" stmt)? | "return" expr ";"
//           | "while" "( expr ") stmt
//           | "for" "(" expr? ";" expr? ";" expr? ")" stmt
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
    fprintf(stderr, "Reading stmt.\n");
    Node *node;
    node = calloc(1, sizeof(Node));


    if (consume_return()) {
        node->kind = ND_RETURN;
        node->lhs = expr();
        if (at_eof()) {
            fprintf(stderr, "expected \"%c\"", ';');
            exit(1);
        }
        //;は区切りの意味しかないので、expectで進める
        expect(";");
    } else if (consume_if()) {

        node->kind = ND_IF;
        expect("(");
        node->if_cond = expr();
        expect(")");
        node->if_true = stmt();
        node->if_false = consume_else() ? stmt() : blank_node();

    } else if (consume_while()) {

        expect("(");
        node->kind = ND_WHILE;
        node->lhs = expr();
        expect(")");
        node->rhs = stmt();

    } else if (consume_for()) {
        expect("(");
        node->kind = ND_FOR;

        // for文の初期化
        if (consume(";")) {
            node->for_init = blank_node();
        } else {
            node->for_init = expr();
            expect(";");
        }

        // for文の条件
        if (consume(";")) {
            node->for_cond = blank_node();
        } else {
            node->for_cond = expr();
            expect(";");
        }

        // for文の更新式
        if (consume(")")) {
            node->for_upd = blank_node();
        } else {
            node->for_upd = expr();
            expect(")");
        }

        node->for_content = stmt();
    } else if(consume("{")) {
        int tmp = 0;
        node->kind = ND_BLOCK;
        vector v;
        v.head = NULL;
        v.tail = NULL;

        while(!consume("}")) {
            tmp++;
            cell *s = calloc(1, sizeof(cell));
            s->next = NULL;
            s->stmt = stmt();


            if(v.head == NULL) {
                v.head = s;
                v.tail = s;
            } else {
                v.tail->next = s;
                v.tail = s;
            }

            if(tmp == 10) {
                fprintf(stderr, "複文が閉じていません\n");
                exit(1);
            }
        }

        node->compound = v;


    } else {
        node = expr();
        if (at_eof()) {
            fprintf(stderr, "expected \"%c\"", ';');
            exit(1);
        }
        //;は区切りの意味しかないので、expectで進める
        expect(";");
    }

    fprintf(stderr, "Created node of type %s.\n", node_name[node->kind]);
    return node;
}

Node *expr() {
    fprintf(stderr, "Reading expr.\n");
    return assign();
}

Node *assign() {
    fprintf(stderr, "Reading assign.\n");
    Node *node = equality();
    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }

    return node;
}

Node *equality() {
    fprintf(stderr, "Reading equality.\n");
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
    fprintf(stderr, "Reading relation.\n");
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
    fprintf(stderr, "Reading add.\n");
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
    fprintf(stderr, "Reading mul.\n");
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
    fprintf(stderr, "Reading unary.\n");
    if (consume("+")) {
        return primary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    } else {
        return primary();
    }
}


Node *primary() {
    fprintf(stderr, "Reading primary.\n");
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();

    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);

        if (lvar) {
            node->offset = lvar->offset;
        } else if (locals) {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = 8;
            node->offset = lvar->offset;
            locals = lvar;
        }
        return node;
    }


    return new_node_num(expect_number());
}