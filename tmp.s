.intel_syntax noprefix
.globl main
main:
  push 10
  push 11
  pop rdi
  pop rax
  cmp rax, rdi
  setle al
  movzb rax, al
  push rax
  pop rax
  ret
