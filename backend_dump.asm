	push rbp

	mov rbp, rsp

	mov [rbp + (-8)], rax

	mov [rbp + (-16)], rax

	mov [rbp + (-24)], rax

	mov [rbp + (-8)], rax

	mov rax, 0

	leave

	ret

	push rbp

	mov rbp, rsp

	leave

	ret

	push rbp

	mov rbp, rsp

	mov [rbp + (-24)], rax

	mov rax, 1

	leave

	ret

	push rbp

	mov rbp, rsp

	mov [rbp + (-32)], rax

	mov [rbp + (-40)], rax

	mov [rbp + (-48)], rax

	mov rax, 2

	leave

	ret

