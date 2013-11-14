	.file	"WMath.cpp"
	.section	.text._Z3maplllll,"x"
.globl __Z3maplllll
__Z3maplllll:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	subl	$4, %esp
	movl	12(%ebp), %eax
	movl	20(%ebp), %esi
	movl	24(%ebp), %ebx
	subl	%esi, %ebx
	movl	8(%ebp), %ecx
	subl	%eax, %ecx
	imull	%ecx, %ebx
	movl	16(%ebp), %ecx
	subl	%eax, %ecx
	movl	%ebx, %edx
	movl	%ebx, %eax
	sarl	$31, %edx
	idivl	%ecx
	addl	%esi, %eax
	addl	$4, %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text._Z8makeWordj,"x"
.globl __Z8makeWordj
__Z8makeWordj:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	popl	%ebp
	ret
	.section	.text._Z8makeWordhh,"x"
.globl __Z8makeWordhh
__Z8makeWordhh:
	pushl	%ebp
	movl	%esp, %ebp
	movl	$0, %eax
	movb	12(%ebp), %al
	movl	$0, %edx
	movb	8(%ebp), %dl
	sall	$8, %edx
	orl	%edx, %eax
	popl	%ebp
	ret
	.section	.text._Z6randoml,"x"
.globl __Z6randoml
__Z6randoml:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	movl	$0, %ecx
	testl	%ebx, %ebx
	je	L9
	call	_random
	movl	%eax, %edx
	sarl	$31, %edx
	idivl	%ebx
	movl	%edx, %ecx
L9:
	movl	%ecx, %eax
	addl	$4, %esp
	popl	%ebx
	popl	%ebp
	ret
	.section	.text._Z6randomll,"x"
.globl __Z6randomll
__Z6randomll:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	cmpl	12(%ebp), %ebx
	jge	L12
	subl	$12, %esp
	movl	12(%ebp), %eax
	subl	%ebx, %eax
	pushl	%eax
	call	__Z6randoml
	addl	%eax, %ebx
	addl	$16, %esp
L12:
	movl	%ebx, %eax
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._Z10randomSeedj,"x"
.globl __Z10randomSeedj
__Z10randomSeedj:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	je	L16
	subl	$12, %esp
	pushl	%eax
	call	_srandom
	addl	$16, %esp
L16:
	leave
	ret
	.ident	"GCC: (GNU) 4.3.2"
