section .text

global main

main:
	push rbp

	mov rbp, rsp

	sub rsp, 16

	mov rax, 1

	cmp rax, 0

	jbe Label1

	mov rax, 10

	mov rdi, rax

	call хуй

	mov [rbp + (-8)], rax

Label1:
	mov rax, 0

	leave

	ret

хуй:
	push rbp

	mov rbp, rsp

	sub rsp, 16

	mov rax, 10

	mov rbx, rax

	mov rax, 10

	imul rbx

	leave

	ret

