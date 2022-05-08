#include "header.h"

int main(int argc, char **argv) {

    char *user_input = read_file(argv[1]);

    fprintf(stderr, "%s\n", user_input);

    //グローバル変数tokenに、入力された文字列の最初の文字へのポインタを与える
    token = tokenize(user_input);

    // codeにNodeの列を保存する
    program();

    fprintf(stderr, "\nTokens successfully parsed.\n\nGenerating code.\n\n");

    //アセンブリの前半部分
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    //プロローグ
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    for (int i = 0; code[i]; i++) {
        gen(code[i]);
        printf("  pop rax\n");
    }

    // エピローグ
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}