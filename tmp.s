.intel_syntax noprefix
.globl main
main:
  push 5
  push 6
  push 7
  pop rdi
  pop rax
  imul rax, rdi
  push rax
  pop rdi
  pop rax
  add rax, rdi
  push rax
  pop rax
  ret
