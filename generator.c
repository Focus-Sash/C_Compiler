#include "compiler.h"



//ノードを右辺値として評価する
//tokenを読みながら構文木を構築し、nodeの左右の子の値をスタックにpushし、nodeを根とする部分木の値を計算し、raxに格納するアセンブラを出力する
void gen(Node *node) {

    switch (node->kind) {
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            //この時点でスタックトップには変数のアドレスが入っている
            printf("  pop rax\n");
            //下の命令は「raxの値をアドレスとみなしてそこから値をロードしraxに保存する」
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case ND_ASSIGN:
            //ノードの左の子(変数名)を左辺値として評価する
            //スタックトップにはraxの値(変数のアドレス)が入る
            gen_lval(node->lhs);
            //右の子を右辺値として評価する
            //スタックトップにはraxの値(評価後の値)が入る
            gen(node->rhs);

            printf("  pop rdi\n");
            printf("  pop rax\n");
            //raxに格納されているアドレスにrdiの値を代入する(raxには代入しない)
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;
        case ND_RETURN:
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            return;
        case ND_IF:
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .L%d\n", counter);
            gen(node->rhs);
            printf(".L%d:\n", counter);
            counter++;
            return;
        case ND_IF_ELSE:
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lelse%d\n", counter);
            gen(node->rhs);
            printf(".Lend%d:\n", counter + 1);
            counter += 2;
            return;
        case ND_ELSE:
            gen(node->lhs);
            printf("  jmp .Lend%d\n", counter + 1);
            printf(".Lelse%d:\n", counter);
            gen(node->rhs);
            return;
        case ND_WHILE:
            printf(".Lbegin%d:\n", counter);
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lend%d\n", counter + 1);
            gen(node->rhs);
            printf("  jmp .Lbegin%d\n", counter);
            printf(".Lend%d:\n", counter + 1);
            counter += 2;
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

//ノードを左辺値として評価する
void gen_lval(Node *node) {
    if(node->kind != ND_LVAR){
        error("代入の左辺値が変数ではありません");
    }

    //ベースアドレスの番地をraxに代入
    printf("  mov rax, rbp\n");
    //オフセットの分だけraxの値を減らす(そのアドレスに変数が割り当てられる)
    printf("  sub rax, %d\n", node->offset);
    //スタックにraxに書いてあるアドレスを代入する
    printf("  push rax\n");
}