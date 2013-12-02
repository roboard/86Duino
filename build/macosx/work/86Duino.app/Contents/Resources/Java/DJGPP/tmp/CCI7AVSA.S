	.file	"WString.cpp"
	.section	.text._ZNK6String16equalsIgnoreCaseERKS_,"x"
	.p2align 1
.globl __ZNK6String16equalsIgnoreCaseERKS_
__ZNK6String16equalsIgnoreCaseERKS_:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %edx
	movl	12(%ebp), %ecx
	cmpl	%ecx, %edx
	je	L2
	movl	8(%edx), %eax
	cmpl	8(%ecx), %eax
	jne	L3
	testl	%eax, %eax
	je	L2
	movl	(%edx), %edx
	movl	%edx, -16(%ebp)
	movl	(%ecx), %edi
	movl	$0, %ecx
	movl	$___dj_ctype_tolower, %esi
	jmp	L4
L5:
	movsbl	(%edi,%ecx),%eax
	movb	1(%esi,%eax), %dl
	incl	%ecx
	movsbl	%bl,%eax
	cmpb	%dl, 1(%esi,%eax)
	jne	L3
L4:
	movl	-16(%ebp), %eax
	movb	(%eax,%ecx), %bl
	testb	%bl, %bl
	jne	L5
L2:
	movb	$1, %al
	jmp	L6
L3:
	movb	$0, %al
L6:
	addl	$4, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZN6String9setCharAtEjc,"x"
	.p2align 1
.globl __ZN6String9setCharAtEjc
__ZN6String9setCharAtEjc:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	movl	12(%ebp), %ecx
	movb	16(%ebp), %dl
	cmpl	%ecx, 8(%eax)
	jbe	L10
	movl	(%eax), %eax
	movb	%dl, (%eax,%ecx)
L10:
	popl	%ebp
	ret
	.section	.text._ZN6StringixEj,"x"
	.p2align 1
.globl __ZN6StringixEj
__ZN6StringixEj:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %edx
	movl	12(%ebp), %ecx
	cmpl	%ecx, 8(%edx)
	jbe	L12
	movl	(%edx), %eax
	testl	%eax, %eax
	jne	L13
L12:
	movb	$0, __ZZN6StringixEjE19dummy_writable_char
	movl	$__ZZN6StringixEjE19dummy_writable_char, %eax
	jmp	L14
L13:
	addl	%ecx, %eax
L14:
	popl	%ebp
	ret
	.section	.text._ZNK6StringixEj,"x"
	.p2align 1
.globl __ZNK6StringixEj
__ZNK6StringixEj:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %edx
	movl	12(%ebp), %ecx
	cmpl	%ecx, 8(%edx)
	jbe	L17
	movl	(%edx), %eax
	testl	%eax, %eax
	je	L17
	movb	(%eax,%ecx), %al
	jmp	L18
L17:
	movb	$0, %al
L18:
	popl	%ebp
	ret
	.section	.text._ZNK6String6charAtEj,"x"
	.p2align 1
.globl __ZNK6String6charAtEj
__ZNK6String6charAtEj:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	__ZNK6StringixEj
	addl	$8, %esp
	leave
	ret
	.section	.text._ZN6String7replaceEcc,"x"
	.p2align 1
.globl __ZN6String7replaceEcc
__ZN6String7replaceEcc:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	movb	12(%ebp), %cl
	movb	16(%ebp), %bl
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	testl	%eax, %eax
	je	L26
	movb	(%eax), %dl
	testb	%dl, %dl
	je	L26
L27:
	cmpb	%cl, %dl
	jne	L24
	movb	%bl, (%eax)
L24:
	incl	%eax
	movb	(%eax), %dl
	testb	%dl, %dl
	jne	L27
L26:
	popl	%ebx
	popl	%ebp
	ret
	.section	.text._ZN6String11toLowerCaseEv,"x"
	.p2align 1
.globl __ZN6String11toLowerCaseEv
__ZN6String11toLowerCaseEv:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	movl	8(%ebp), %eax
	movl	(%eax), %edx
	testl	%edx, %edx
	je	L32
	movb	(%edx), %al
	testb	%al, %al
	je	L32
	movl	$___dj_ctype_tolower, %ecx
L33:
	movsbl	%al,%eax
	movb	1(%ecx,%eax), %al
	movb	%al, (%edx)
	incl	%edx
	movb	(%edx), %al
	testb	%al, %al
	jne	L33
L32:
	popl	%ebx
	popl	%ebp
	ret
	.section	.text._ZN6String11toUpperCaseEv,"x"
	.p2align 1
.globl __ZN6String11toUpperCaseEv
__ZN6String11toUpperCaseEv:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	movl	8(%ebp), %eax
	movl	(%eax), %edx
	testl	%edx, %edx
	je	L38
	movb	(%edx), %al
	testb	%al, %al
	je	L38
	movl	$___dj_ctype_toupper, %ecx
L39:
	movsbl	%al,%eax
	movb	1(%ecx,%eax), %al
	movb	%al, (%edx)
	incl	%edx
	movb	(%edx), %al
	testb	%al, %al
	jne	L39
L38:
	popl	%ebx
	popl	%ebp
	ret
	.section	.text._ZNK6String5toIntEv,"x"
	.p2align 1
.globl __ZNK6String5toIntEv
__ZNK6String5toIntEv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	movl	(%eax), %edx
	movl	$0, %eax
	testl	%edx, %edx
	je	L43
	subl	$12, %esp
	pushl	%edx
	call	_atol
	addl	$16, %esp
L43:
	leave
	ret
	.section	.text._ZN6String4trimEv,"x"
	.p2align 1
.globl __ZN6String4trimEv
__ZN6String4trimEv:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %edi
	movl	(%edi), %edx
	testl	%edx, %edx
	je	L54
	movl	8(%edi), %esi
	testl	%esi, %esi
	je	L54
	movsbl	(%edx),%eax
	movl	%edx, %ecx
	movl	$___dj_ctype_flags, %ebx
	testb	$1, ___dj_ctype_flags+3(%eax,%eax)
	je	L48
L56:
	incl	%ecx
	movsbl	(%ecx),%eax
	testb	$1, 3(%ebx,%eax,2)
	jne	L56
L48:
	leal	-1(%edx,%esi), %edx
	movsbl	(%edx),%eax
	testb	$1, ___dj_ctype_flags+3(%eax,%eax)
	je	L50
	cmpl	%ecx, %edx
	jb	L50
	movl	$___dj_ctype_flags, %ebx
L55:
	decl	%edx
	movsbl	(%edx),%eax
	testb	$1, 3(%ebx,%eax,2)
	je	L50
	cmpl	%ecx, %edx
	jae	L55
L50:
	leal	1(%edx), %eax
	subl	%ecx, %eax
	movl	%eax, 8(%edi)
	movl	(%edi), %edx
	cmpl	%ecx, %edx
	jae	L53
	subl	$4, %esp
	pushl	%eax
	pushl	%ecx
	pushl	%edx
	call	_memcpy
	addl	$16, %esp
L53:
	movl	(%edi), %edx
	movl	8(%edi), %eax
	movb	$0, (%edx,%eax)
L54:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZNK6String11lastIndexOfERKS_j,"x"
	.p2align 1
.globl __ZNK6String11lastIndexOfERKS_j
__ZNK6String11lastIndexOfERKS_j:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	12(%ebp), %edx
	movl	8(%edx), %eax
	testl	%eax, %eax
	je	L60
	movl	8(%ebp), %ebx
	movl	8(%ebx), %ecx
	testl	%ecx, %ecx
	je	L60
	cmpl	%ecx, %eax
	ja	L60
	cmpl	16(%ebp), %ecx
	ja	L61
	decl	%ecx
	movl	%ecx, 16(%ebp)
L61:
	movl	8(%ebp), %eax
	movl	(%eax), %esi
	movl	$-1, %ebx
	movl	16(%ebp), %eax
	addl	%esi, %eax
	movl	%eax, -16(%ebp)
	jb	L63
	movl	(%edx), %edi
	movl	%esi, %eax
	movl	$-1, %ebx
L65:
	subl	$8, %esp
	pushl	%edi
	pushl	%eax
	call	_strstr
	addl	$16, %esp
	testl	%eax, %eax
	je	L63
	movl	%eax, %edx
	subl	%esi, %edx
	cmpl	%edx, 16(%ebp)
	jb	L64
	movl	%edx, %ebx
L64:
	incl	%eax
	cmpl	%eax, -16(%ebp)
	jae	L65
	jmp	L63
L60:
	movl	$-1, %ebx
L63:
	movl	%ebx, %eax
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZNK6String11lastIndexOfERKS_,"x"
	.p2align 1
.globl __ZNK6String11lastIndexOfERKS_
__ZNK6String11lastIndexOfERKS_:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	8(%ebp), %ecx
	movl	12(%ebp), %edx
	movl	8(%ecx), %eax
	subl	8(%edx), %eax
	pushl	%eax
	pushl	%edx
	pushl	%ecx
	call	__ZNK6String11lastIndexOfERKS_j
	leave
	ret
	.section	.text._ZNK6String7indexOfERKS_j,"x"
	.p2align 1
.globl __ZNK6String7indexOfERKS_j
__ZNK6String7indexOfERKS_j:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %edx
	movl	16(%ebp), %ecx
	movl	$-1, %eax
	cmpl	%ecx, 8(%edx)
	jbe	L72
	movl	(%edx), %ebx
	subl	$8, %esp
	movl	12(%ebp), %eax
	pushl	(%eax)
	leal	(%ebx,%ecx), %eax
	pushl	%eax
	call	_strstr
	addl	$16, %esp
	movl	%eax, %edx
	movl	$-1, %eax
	testl	%edx, %edx
	je	L72
	movl	%edx, %eax
	subl	%ebx, %eax
L72:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZNK6String7indexOfERKS_,"x"
	.p2align 1
.globl __ZNK6String7indexOfERKS_
__ZNK6String7indexOfERKS_:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	$0
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	__ZNK6String7indexOfERKS_j
	addl	$16, %esp
	leave
	ret
	.section	.text._ZNK6String11lastIndexOfEcj,"x"
	.p2align 1
.globl __ZNK6String11lastIndexOfEcj
__ZNK6String11lastIndexOfEcj:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %edi
	movl	16(%ebp), %edx
	movb	12(%ebp), %cl
	movl	$-1, %eax
	cmpl	%edx, 8(%edi)
	jbe	L79
	leal	1(%edx), %esi
	movl	%esi, %eax
	addl	(%edi), %eax
	movb	(%eax), %dl
	movb	%dl, -13(%ebp)
	movb	$0, (%eax)
	movl	(%edi), %ebx
	subl	$8, %esp
	movsbl	%cl,%eax
	pushl	%eax
	pushl	%ebx
	call	_strrchr
	addl	$16, %esp
	movl	%eax, %edx
	movb	-13(%ebp), %al
	movb	%al, (%ebx,%esi)
	movl	$-1, %eax
	testl	%edx, %edx
	je	L79
	movl	%edx, %eax
	subl	(%edi), %eax
L79:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZNK6String11lastIndexOfEc,"x"
	.p2align 1
.globl __ZNK6String11lastIndexOfEc
__ZNK6String11lastIndexOfEc:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	8(%ebp), %edx
	movl	8(%edx), %eax
	decl	%eax
	pushl	%eax
	movsbl	12(%ebp),%eax
	pushl	%eax
	pushl	%edx
	call	__ZNK6String11lastIndexOfEcj
	leave
	ret
	.section	.text._ZNK6String7indexOfEcj,"x"
	.p2align 1
.globl __ZNK6String7indexOfEcj
__ZNK6String7indexOfEcj:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %ecx
	movl	16(%ebp), %ebx
	movb	12(%ebp), %dl
	movl	$-1, %eax
	cmpl	%ebx, 8(%ecx)
	jbe	L86
	movl	(%ecx), %esi
	subl	$8, %esp
	movsbl	%dl,%eax
	pushl	%eax
	leal	(%esi,%ebx), %eax
	pushl	%eax
	call	_strchr
	addl	$16, %esp
	movl	%eax, %edx
	movl	$-1, %eax
	testl	%edx, %edx
	je	L86
	movl	%edx, %eax
	subl	%esi, %eax
L86:
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text._ZNK6String7indexOfEc,"x"
	.p2align 1
.globl __ZNK6String7indexOfEc
__ZNK6String7indexOfEc:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	$0
	movsbl	12(%ebp),%eax
	pushl	%eax
	pushl	8(%ebp)
	call	__ZNK6String7indexOfEcj
	addl	$16, %esp
	leave
	ret
	.section	.text._ZNK6String8getBytesEPhjj,"x"
	.p2align 1
.globl __ZNK6String8getBytesEPhjj
__ZNK6String8getBytesEPhjj:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	12(%ebp), %esi
	movl	16(%ebp), %edx
	testl	%edx, %edx
	je	L95
	testl	%esi, %esi
	je	L95
	movl	8(%ebp), %ecx
	movl	8(%ecx), %eax
	cmpl	20(%ebp), %eax
	ja	L93
	movb	$0, (%esi)
	jmp	L95
L93:
	decl	%edx
	movl	%eax, %ebx
	subl	20(%ebp), %ebx
	cmpl	%edx, %ebx
	jbe	L94
	movl	%edx, %ebx
L94:
	subl	$4, %esp
	pushl	%ebx
	movl	20(%ebp), %eax
	movl	8(%ebp), %edx
	addl	(%edx), %eax
	pushl	%eax
	pushl	%esi
	call	_strncpy
	movb	$0, (%esi,%ebx)
	addl	$16, %esp
L95:
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text._ZNK6String8endsWithERKS_,"x"
	.p2align 1
.globl __ZNK6String8endsWithERKS_
__ZNK6String8endsWithERKS_:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %edx
	movl	12(%ebp), %esi
	movl	8(%edx), %ecx
	movl	8(%esi), %eax
	cmpl	%eax, %ecx
	jb	L97
	movl	(%edx), %ebx
	testl	%ebx, %ebx
	je	L97
	movl	(%esi), %edx
	testl	%edx, %edx
	je	L97
	subl	$8, %esp
	pushl	%edx
	subl	%eax, %ecx
	leal	(%ebx,%ecx), %eax
	pushl	%eax
	call	_strcmp
	addl	$16, %esp
	testl	%eax, %eax
	sete	%al
	jmp	L98
L97:
	movb	$0, %al
L98:
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text._ZNK6String6equalsEPKc,"x"
	.p2align 1
.globl __ZNK6String6equalsEPKc
__ZNK6String6equalsEPKc:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	movl	12(%ebp), %edx
	cmpl	$0, 8(%eax)
	jne	L101
	movb	$1, %al
	testl	%edx, %edx
	je	L104
	cmpb	$0, (%edx)
	sete	%al
	jmp	L104
L101:
	testl	%edx, %edx
	jne	L105
	movl	(%eax), %eax
	cmpb	$0, (%eax)
	sete	%al
	jmp	L104
L105:
	subl	$8, %esp
	pushl	%edx
	pushl	(%eax)
	call	_strcmp
	addl	$16, %esp
	testl	%eax, %eax
	sete	%al
L104:
	leave
	ret
	.section	.text._ZNK6String9compareToERKS_,"x"
	.p2align 1
.globl __ZNK6String9compareToERKS_
__ZNK6String9compareToERKS_:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %ecx
	movl	12(%ebp), %edx
	movl	(%ecx), %eax
	testl	%eax, %eax
	je	L108
	movl	(%edx), %edx
	testl	%edx, %edx
	jne	L115
	jmp	L116
L108:
	movl	(%edx), %eax
	testl	%eax, %eax
	je	L111
	cmpl	$0, 8(%edx)
	je	L111
	movb	(%eax), %al
	andl	$255, %eax
	negl	%eax
	jmp	L112
L113:
	movb	(%eax), %al
	andl	$255, %eax
	jmp	L112
L115:
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_strcmp
	addl	$16, %esp
	jmp	L112
L111:
	movl	$0, %eax
L112:
	leave
	ret
L116:
	cmpl	$0, 8(%ecx)
	jne	L113
	jmp	L111
	.section	.text._ZNK6StringgeERKS_,"x"
	.p2align 1
.globl __ZNK6StringgeERKS_
__ZNK6StringgeERKS_:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	__ZNK6String9compareToERKS_
	addl	$16, %esp
	notl	%eax
	shrl	$31, %eax
	leave
	ret
	.section	.text._ZNK6StringleERKS_,"x"
	.p2align 1
.globl __ZNK6StringleERKS_
__ZNK6StringleERKS_:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	__ZNK6String9compareToERKS_
	addl	$16, %esp
	testl	%eax, %eax
	setle	%al
	leave
	ret
	.section	.text._ZNK6StringgtERKS_,"x"
	.p2align 1
.globl __ZNK6StringgtERKS_
__ZNK6StringgtERKS_:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	__ZNK6String9compareToERKS_
	addl	$16, %esp
	testl	%eax, %eax
	setg	%al
	leave
	ret
	.section	.text._ZNK6StringltERKS_,"x"
	.p2align 1
.globl __ZNK6StringltERKS_
__ZNK6StringltERKS_:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	__ZNK6String9compareToERKS_
	addl	$16, %esp
	shrl	$31, %eax
	leave
	ret
	.section	.text._ZNK6String6equalsERKS_,"x"
	.p2align 1
.globl __ZNK6String6equalsERKS_
__ZNK6String6equalsERKS_:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	movl	12(%ebp), %ecx
	movl	8(%ebx), %eax
	movb	$0, %dl
	cmpl	8(%ecx), %eax
	jne	L127
	subl	$8, %esp
	pushl	%ecx
	pushl	%ebx
	call	__ZNK6String9compareToERKS_
	addl	$16, %esp
	testl	%eax, %eax
	sete	%dl
L127:
	movb	%dl, %al
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZNK6String10startsWithERKS_j,"x"
	.p2align 1
.globl __ZNK6String10startsWithERKS_j
__ZNK6String10startsWithERKS_j:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %edx
	movl	12(%ebp), %ebx
	movl	8(%ebx), %ecx
	movl	8(%edx), %eax
	subl	%ecx, %eax
	cmpl	16(%ebp), %eax
	jb	L130
	movl	(%edx), %eax
	testl	%eax, %eax
	je	L130
	movl	(%ebx), %edx
	testl	%edx, %edx
	je	L130
	subl	$4, %esp
	pushl	%ecx
	pushl	%edx
	addl	16(%ebp), %eax
	pushl	%eax
	call	_strncmp
	addl	$16, %esp
	testl	%eax, %eax
	sete	%al
	jmp	L131
L130:
	movb	$0, %al
L131:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZNK6String10startsWithERKS_,"x"
	.p2align 1
.globl __ZNK6String10startsWithERKS_
__ZNK6String10startsWithERKS_:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	movl	12(%ebp), %ecx
	movl	8(%ebx), %eax
	movb	$0, %dl
	cmpl	8(%ecx), %eax
	jb	L135
	subl	$4, %esp
	pushl	$0
	pushl	%ecx
	pushl	%ebx
	call	__ZNK6String10startsWithERKS_j
	addl	$16, %esp
	movb	%al, %dl
L135:
	movb	%dl, %al
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZN6String12changeBufferEj,"x"
	.p2align 1
.globl __ZN6String12changeBufferEj
__ZN6String12changeBufferEj:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %ebx
	movl	12(%ebp), %esi
	subl	$8, %esp
	leal	1(%esi), %eax
	pushl	%eax
	pushl	(%ebx)
	call	_realloc
	movl	%eax, %edx
	addl	$16, %esp
	movb	$0, %al
	testl	%edx, %edx
	je	L139
	movl	%edx, (%ebx)
	movl	%esi, 4(%ebx)
	movb	$1, %al
L139:
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text._ZN6String7replaceERKS_S1_,"x"
	.p2align 1
.globl __ZN6String7replaceERKS_S1_
__ZN6String7replaceERKS_S1_:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$28, %esp
	movl	8(%ebp), %eax
	movl	8(%eax), %eax
	movl	%eax, -20(%ebp)
	testl	%eax, %eax
	je	L153
	movl	12(%ebp), %edx
	movl	8(%edx), %edi
	testl	%edi, %edi
	je	L153
	movl	16(%ebp), %ecx
	movl	8(%ecx), %eax
	subl	%edi, %eax
	movl	%eax, -28(%ebp)
	movl	8(%ebp), %edx
	movl	(%edx), %esi
	testl	%eax, %eax
	je	L156
	jmp	L158
L145:
	subl	$4, %esp
	movl	16(%ebp), %ecx
	pushl	8(%ecx)
	pushl	(%ecx)
	pushl	%ebx
	call	_memcpy
	movl	%ebx, %esi
	movl	16(%ebp), %eax
	addl	8(%eax), %esi
	addl	$16, %esp
L156:
	subl	$8, %esp
	movl	12(%ebp), %edx
	pushl	(%edx)
	pushl	%esi
	call	_strstr
	addl	$16, %esp
	movl	%eax, %ebx
	testl	%eax, %eax
	jne	L145
	jmp	L153
L158:
	movl	%esi, -24(%ebp)
	testl	%eax, %eax
	js	L147
	movl	12(%ebp), %ecx
	movl	(%ecx), %ecx
	movl	%ecx, -16(%ebp)
	movl	-20(%ebp), %ebx
	jmp	L148
L149:
	movl	%edi, %ebx
	subl	%esi, %ebx
	subl	$4, %esp
	pushl	%ebx
	pushl	%esi
	pushl	-24(%ebp)
	call	_memcpy
	addl	-24(%ebp), %ebx
	addl	$12, %esp
	movl	16(%ebp), %eax
	pushl	8(%eax)
	pushl	(%eax)
	pushl	%ebx
	call	_memcpy
	movl	16(%ebp), %edx
	addl	8(%edx), %ebx
	movl	%ebx, -24(%ebp)
	movl	%edi, %esi
	movl	12(%ebp), %ecx
	addl	8(%ecx), %esi
	movl	-28(%ebp), %edx
	movl	8(%ebp), %eax
	addl	%edx, 8(%eax)
	addl	$16, %esp
L147:
	subl	$8, %esp
	movl	12(%ebp), %ecx
	pushl	(%ecx)
	pushl	%esi
	call	_strstr
	addl	$16, %esp
	movl	%eax, %edi
	testl	%eax, %eax
	jne	L149
	subl	$8, %esp
	pushl	%esi
	pushl	-24(%ebp)
	call	_strcpy
	addl	$16, %esp
	jmp	L153
L150:
	leal	(%eax,%edi), %esi
	addl	-28(%ebp), %ebx
L148:
	subl	$8, %esp
	pushl	-16(%ebp)
	pushl	%esi
	call	_strstr
	addl	$16, %esp
	testl	%eax, %eax
	jne	L150
	cmpl	-20(%ebp), %ebx
	je	L153
	movl	8(%ebp), %eax
	cmpl	4(%eax), %ebx
	jbe	L151
	subl	$8, %esp
	pushl	%ebx
	pushl	%eax
	call	__ZN6String12changeBufferEj
	addl	$16, %esp
	testb	%al, %al
	jne	L151
	jmp	L153
L154:
	subl	$4, %esp
	pushl	%eax
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	__ZNK6String11lastIndexOfERKS_j
	movl	%eax, %ebx
	addl	$16, %esp
	testl	%eax, %eax
	js	L153