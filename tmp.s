.intel_syntax noprefix
.globl main
main:
  push 4
  push 3
  push 2
  pop rdi
  pop rax
  sub rax, rdi
  push rax
  push 9
  pop rdi
  pop rax
  imul rax, rdi
  push rax
  pop rdi
  pop rax
  add rax, rdi
  push rax
  push 3
  push 4
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
