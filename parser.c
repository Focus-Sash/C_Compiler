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

Node *blank_node() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BLANK;
    return node;
}


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
          dump();
//        expect("(");
//        Node *c1 = calloc(1, sizeof(Node));
//        c1 = expr();
//        expect(")");
//        Node *c2 = calloc(1, sizeof(Node));
//        c2 = stmt();
//
//        if (consume_else()) {
//            node->kind = ND_IF_ELSE;
//            node->ifelse_cond = c1;
//            node->ifelse_true = c2;
//            node->ifelse_false =
//            Node *e = calloc(1, sizeof(Node));
//            e->kind = ND_ELSE;
//            e->lhs = c2;
//            e->rhs = stmt();
//            node->lhs = c1;
//            node->rhs = e;
//        } else {
//            node->kind = ND_IF;
//            node->lhs = c1;
//            node->rhs = c2;
//        }
        dump();
        node->kind = ND_IF;
        expect("(");
        dump();
        node->if_cond = expr();
        expect(")");
        dump();
        node->if_true = stmt();
        dump();
        node->if_false = consume_else() ? stmt() : blank_node();
        dump();
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
        if(consume(")")) {
            node->for_upd = blank_node();
        } else {
            node->for_upd = expr();
            expect(")");
        }

        node->for_content = stmt();
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