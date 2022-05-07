#include "compiler.h"


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
//stmt       = expr ";" | "if" "(" expr ")" stmt | "return" expr ";"
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
    Node *node;
    if (consume_return()) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        if (at_eof()) {
            fprintf(stderr, "expected \"%c\"", ';');
            exit(1);
        }
        //;は区切りの意味しかないので、expectで進める
        expect(";");
    } else if (consume_if()) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->lhs = expr();
        expect(")");
        node->rhs = stmt();
    } else {
        node = expr();
        if (at_eof()) {
            fprintf(stderr, "expected \"%c\"", ';');
            exit(1);
        }
        //;は区切りの意味しかないので、expectで進める
        expect(";");
    }

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