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
// expr     = equality
// equality = relation("==" relation | "!=" relation)*
// relation = add ("<" add | "<=" add | ">" add | ">=" add)*
// add      = mul ("+" mul | "-" mul)*
// mul      = unary( "*" unary | "/" unary)*
// unary    = ( "+" | "-" )? primary
// primary  = num | "(" expr ")"



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