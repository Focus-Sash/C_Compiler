#include "compiler.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }

    char *user_input = argv[1];

    //グローバル変数tokenに、入力された文字列の最初の文字へのポインタを与える
    token = tokenize(argv[1]);

    //全体がexprであるという前提のもと、構文木の根となるノードへのポインタをnodeに与える
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