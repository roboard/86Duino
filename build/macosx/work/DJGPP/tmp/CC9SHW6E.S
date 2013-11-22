	.file	"Servo.cpp"
	.section	.ctors,"w"
	.p2align 2
	.long	__GLOBAL__I_interrupts_times
	.section	.text._Z13insertionSortv,"x"
	.p2align 1
.globl __Z13insertionSortv
__Z13insertionSortv:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$16, %esp
	movl	$0, -20(%ebp)
	movl	$0, -20(%ebp)
	jmp	L2
L3:
	movl	-20(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+12(%eax), %eax
	movl	%eax, -16(%ebp)
	movl	-20(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+8(%eax), %eax
	movl	%eax, -12(%ebp)
	movl	-20(%ebp), %eax
	decl	%eax
	movl	%eax, -8(%ebp)
	jmp	L4
L5:
	movl	-8(%ebp), %ebx
	incl	%ebx
	movl	-8(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+12(%eax), %ecx
	movl	%ebx, %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	%ecx, _servos+12(%eax)
	movl	-8(%ebp), %eax
	movl	-16(%ebp), %ecx
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	%ecx, _servos+12(%eax)
	movl	-8(%ebp), %ebx
	incl	%ebx
	movl	-8(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+8(%eax), %ecx
	movl	%ebx, %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	%ecx, _servos+8(%eax)
	movl	-8(%ebp), %eax
	movl	-12(%ebp), %ecx
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	%ecx, _servos+8(%eax)
	decl	-8(%ebp)
L4:
	cmpl	$0, -8(%ebp)
	js	L6
	movl	-8(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+12(%eax), %edx
	movl	-16(%ebp), %eax
	cmpl	%eax, %edx
	ja	L5
L6:
	incl	-20(%ebp)
L2:
	movb	_ServoCount_use, %al
	andl	$255, %eax
	cmpl	-20(%ebp), %eax
	jg	L3
	movl	$0, -20(%ebp)
	jmp	L9
L10:
	cmpl	$0, -20(%ebp)
	jne	L11
	movl	-20(%ebp), %ebx
	movl	-20(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+12(%eax), %eax
	movl	%eax, %ecx
	shrl	%ecx
	movl	%ebx, %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	%ecx, _servos+24(%eax)
	movl	-20(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+12(%eax), %eax
	movb	%al, %dl
	movb	_end_wait, %al
	leal	(%edx,%eax), %eax
	movb	%al, _end_wait
	jmp	L13
L11:
	movl	-20(%ebp), %ebx
	movl	-20(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+12(%eax), %ecx
	movl	-20(%ebp), %eax
	decl	%eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+24(%eax), %eax
	subl	%eax, %ecx
	movl	%ebx, %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	%ecx, _servos+24(%eax)
	movl	-20(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+24(%eax), %eax
	movb	%al, %dl
	movb	_end_wait, %al
	leal	(%edx,%eax), %eax
	movb	%al, _end_wait
L13:
	incl	-20(%ebp)
L9:
	movb	_ServoCount_use, %al
	andl	$255, %eax
	cmpl	-20(%ebp), %eax
	jg	L10
	movl	-20(%ebp), %ebx
	movb	_end_wait, %al
	movl	$0, %edx
	movb	%al, %dl
	movl	$5000, %eax
	subl	%edx, %eax
	movl	%eax, %ecx
	movl	%ebx, %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	%ecx, _servos+24(%eax)
	addl	$16, %esp
	popl	%ebx
	popl	%ebp
	ret
	.section	.text._ZN5ServoC2Ev,"x"
	.p2align 1
.globl __ZN5ServoC2Ev
__ZN5ServoC2Ev:
	pushl	%ebp
	movl	%esp, %ebp
	movb	_ServoCount, %al
	cmpb	$11, %al
	ja	L17
	movb	_ServoCount, %al
	movl	8(%ebp), %edx
	movb	%al, (%edx)
	incl	%eax
	movb	%al, _ServoCount
	jmp	L20
L17:
	movl	8(%ebp), %eax
	movb	$-1, (%eax)
L20:
	popl	%ebp
	ret
	.section	.text._ZN5ServoC1Ev,"x"
	.p2align 1
.globl __ZN5ServoC1Ev
__ZN5ServoC1Ev:
	pushl	%ebp
	movl	%esp, %ebp
	movb	_ServoCount, %al
	cmpb	$11, %al
	ja	L22
	movb	_ServoCount, %al
	movl	8(%ebp), %edx
	movb	%al, (%edx)
	incl	%eax
	movb	%al, _ServoCount
	jmp	L25
L22:
	movl	8(%ebp), %eax
	movb	$-1, (%eax)
L25:
	popl	%ebp
	ret
	.section	.text._ZN5Servo6detachEv,"x"
	.p2align 1
.globl __ZN5Servo6detachEv
__ZN5Servo6detachEv:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	movb	(%eax), %al
	andl	$255, %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movb	_servos(%eax), %dl
	andl	$-65, %edx
	movb	%dl, _servos(%eax)
	popl	%ebp
	ret
	.section	.text._Z41__static_initialization_and_destruction_0ii,"x"
	.p2align 1
__Z41__static_initialization_and_destruction_0ii:
	pushl	%ebp
	movl	%esp, %ebp
	cmpl	$1, 8(%ebp)
	jne	L32
	cmpl	$65535, 12(%ebp)
	jne	L32
	movl	_MC_list+12, %eax
	movl	%eax, _arduint_to_mc_md+12
	movl	_MC_list, %eax
	movl	%eax, _arduint_to_mc_md+20
	movl	_MC_list, %eax
	movl	%eax, _arduint_to_mc_md+24
	movl	_MC_list, %eax
	movl	%eax, _arduint_to_mc_md+36
	movl	_MC_list+4, %eax
	movl	%eax, _arduint_to_mc_md+40
	movl	_MC_list+4, %eax
	movl	%eax, _arduint_to_mc_md+44
	movl	_MC_list+4, %eax
	movl	%eax, _arduint_to_mc_md+52
	movl	_MC_list+8, %eax
	movl	%eax, _arduint_to_mc_md+116
	movl	_MC_list+8, %eax
	movl	%eax, _arduint_to_mc_md+120
	movl	_MC_list+8, %eax
	movl	%eax, _arduint_to_mc_md+124
	movl	_MC_list+12, %eax
	movl	%eax, _arduint_to_mc_md+128
	movl	_MD_list, %eax
	movl	%eax, _arduint_to_mc_md+192
	movl	_MD_list+8, %eax
	movl	%eax, _arduint_to_mc_md+200
	movl	_MD_list+4, %eax
	movl	%eax, _arduint_to_mc_md+204
	movl	_MD_list, %eax
	movl	%eax, _arduint_to_mc_md+216
	movl	_MD_list+8, %eax
	movl	%eax, _arduint_to_mc_md+220
	movl	_MD_list+4, %eax
	movl	%eax, _arduint_to_mc_md+224
	movl	_MD_list, %eax
	movl	%eax, _arduint_to_mc_md+232
	movl	_MD_list, %eax
	movl	%eax, _arduint_to_mc_md+296
	movl	_MD_list+4, %eax
	movl	%eax, _arduint_to_mc_md+300
	movl	_MD_list+8, %eax
	movl	%eax, _arduint_to_mc_md+304
	movl	_MD_list+4, %eax
	movl	%eax, _arduint_to_mc_md+308
	movl	_MC_list+12, %eax
	movl	%eax, _mc
	movl	_MD_list+8, %eax
	movl	%eax, _md
L32:
	popl	%ebp
	ret
	.section	.text._GLOBAL__I_interrupts_times,"x"
	.p2align 1
__GLOBAL__I_interrupts_times:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	$65535
	pushl	$1
	call	__Z41__static_initialization_and_destruction_0ii
	addl	$8, %esp
	leave
	ret
	.section	.text._Z8io_outpbth,"x"
	.p2align 1
__Z8io_outpbth:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movw	%ax, -4(%ebp)
	movb	%dl, -8(%ebp)
	movl	$0, %edx
	movb	-8(%ebp), %dl
	movl	-4(%ebp), %eax
	andl	$65535, %eax
	subl	$4, %esp
	pushl	%edx
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
	leave
	ret
	.section	.text._ZN5Servo6attachEiii,"x"
	.p2align 1
.globl __ZN5Servo6attachEiii
__ZN5Servo6attachEiii:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	movb	(%eax), %al
	cmpb	$11, %al
	ja	L38
	movl	12(%ebp), %eax
	andl	$255, %eax
	subl	$8, %esp
	pushl	$1
	pushl	%eax
	call	_pinMode
	addl	$16, %esp
	movl	8(%ebp), %eax
	movb	(%eax), %al
	movl	$0, %edx
	movb	%al, %dl
	movl	12(%ebp), %eax
	movb	%al, %cl
	movl	%edx, %eax
	sall	$2, %eax
	leal	0(,%eax,8), %edx
	subl	%eax, %edx
	andl	$63, %ecx
	movb	_servos(%edx), %al
	andl	$-64, %eax
	orl	%ecx, %eax
	movb	%al, _servos(%edx)
	movl	$544, %eax
	movl	%eax, %edx
	subl	16(%ebp), %edx
	movl	%edx, %eax
	sarl	$31, %eax
	shrl	$30, %eax
	addl	%edx, %eax
	sarl	$2, %eax
	movb	%al, %dl
	movl	8(%ebp), %eax
	movb	%dl, 1(%eax)
	movl	$2400, %eax
	movl	%eax, %edx
	subl	20(%ebp), %edx
	movl	%edx, %eax
	sarl	$31, %eax
	shrl	$30, %eax
	addl	%edx, %eax
	sarl	$2, %eax
	movb	%al, %dl
	movl	8(%ebp), %eax
	movb	%dl, 2(%eax)
	movl	8(%ebp), %eax
	movb	(%eax), %al
	andl	$255, %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movb	_servos(%eax), %dl
	orl	$64, %edx
	movb	%dl, _servos(%eax)
	movl	8(%ebp), %eax
	movb	(%eax), %al
	andl	$255, %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	$1500, _servos+4(%eax)
	movb	_ServoCount_use, %al
	andl	$255, %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	$1500, _servos+12(%eax)
	movb	_ServoCount_use, %al
	andl	$255, %eax
	movl	12(%ebp), %ecx
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	%ecx, _servos+8(%eax)
	movb	_ServoCount_use, %al
	incl	%eax
	movb	%al, _ServoCount_use
L38:
	movl	8(%ebp), %eax
	movb	(%eax), %al
	andl	$255, %eax
	leave
	ret
	.section	.text._ZN5Servo6attachEi,"x"
	.p2align 1
.globl __ZN5Servo6attachEi
__ZN5Servo6attachEi:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	pushl	$2400
	pushl	$544
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	__ZN5Servo6attachEiii
	addl	$16, %esp
	andl	$255, %eax
	leave
	ret
	.section .text
LC0:
	.ascii "%d \0"
	.section	.text._Z9testtestti,"x"
	.p2align 1
.globl __Z9testtestti
__Z9testtestti:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	_yy, %eax
	incl	%eax
	movl	%eax, _yy
	subl	$8, %esp
	pushl	8(%ebp)
	pushl	$LC0
	call	_printf
	addl	$16, %esp
	movl	_yy, %eax
	cmpl	$500, %eax
	jne	L46
L45:
	jmp	L45
L46:
	leave
	ret
	.section .text
LC1:
	.ascii "Pin: %d ticks: %d  \0"
	.p2align 2
LC2:
	.ascii "end_Pin: %d end_ticks: %d value: %d\12\0"
LC3:
	.ascii "value:%d\12\0"
	.section	.text._Z8testtestv,"x"
	.p2align 1
.globl __Z8testtestv
__Z8testtestv:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$20, %esp
	movl	$0, -8(%ebp)
	jmp	L48
L49:
	movl	-8(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+4(%eax), %ecx
	movl	-8(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movb	_servos(%eax), %al
	andl	$63, %eax
	andl	$255, %eax
	subl	$4, %esp
	pushl	%ecx
	pushl	%eax
	pushl	$LC1
	call	_printf
	addl	$16, %esp
	movl	-8(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+24(%eax), %ebx
	movl	-8(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+12(%eax), %ecx
	movl	-8(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+8(%eax), %eax
	pushl	%ebx
	pushl	%ecx
	pushl	%eax
	pushl	$LC2
	call	_printf
	addl	$16, %esp
	incl	-8(%ebp)
L48:
	movb	_ServoCount, %al
	andl	$255, %eax
	cmpl	-8(%ebp), %eax
	jg	L49
	movl	-8(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+24(%eax), %eax
	subl	$8, %esp
	pushl	%eax
	pushl	$LC3
	call	_printf
	addl	$16, %esp
	movl	$0, -8(%ebp)
	jmp	L51
L52:
	incl	-8(%ebp)
L51:
	movb	_ServoCount_use, %al
	andl	$255, %eax
	cmpl	-8(%ebp), %eax
	jg	L52
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._Z7io_inpbt,"x"
	.p2align 1
__Z7io_inpbt:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movw	%ax, -4(%ebp)
	movl	-4(%ebp), %eax
	andl	$65535, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$16, %esp
	andl	$255, %eax
	leave
	ret
	.section	.text._Z18digitalWrite_servohh,"x"
	.p2align 1
.globl __Z18digitalWrite_servohh
__Z18digitalWrite_servohh:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$20, %esp
	movl	8(%ebp), %eax
	movl	12(%ebp), %edx
	movb	%al, -8(%ebp)
	movb	%dl, -12(%ebp)
	cmpb	$44, -8(%ebp)
	ja	L60
	movl	$0, %eax
	movb	-8(%ebp), %al
	movl	_pinMap(,%eax,4), %eax
	movl	%eax, _crossbar_bit
	movl	_crossbar_bit, %edx
	movl	%edx, %eax
	sarl	$31, %eax
	shrl	$29, %eax
	addl	%edx, %eax
	sarl	$3, %eax
	sall	$2, %eax
	addl	$61952, %eax
	movl	%eax, _servo_x
	movl	_crossbar_bit, %edx
	movl	%edx, %eax
	sarl	$31, %eax
	shrl	$29, %eax
	addl	%edx, %eax
	sarl	$3, %eax
	sall	$2, %eax
	subl	$3584, %eax
	andl	$65535, %eax
	call	__Z7io_inpbt
	movl	$0, %ebx
	movb	%al, %bl
	movl	_crossbar_bit, %edx
	movl	%edx, %eax
	sarl	$31, %eax
	movl	%eax, %ecx
	shrl	$29, %ecx
	leal	(%edx,%ecx), %eax
	andl	$7, %eax
	subl	%ecx, %eax
	movl	%eax, %ecx
	movl	$1, %eax
	sall	%cl, %eax
	orl	%ebx, %eax
	movl	%eax, _servo_y
	movl	_crossbar_bit, %edx
	movl	%edx, %eax
	sarl	$31, %eax
	shrl	$29, %eax
	addl	%edx, %eax
	sarl	$3, %eax
	sall	$2, %eax
	subl	$3584, %eax
	andl	$65535, %eax
	call	__Z7io_inpbt
	movl	$0, %ebx
	movb	%al, %bl
	movl	_crossbar_bit, %edx
	movl	%edx, %eax
	sarl	$31, %eax
	movl	%eax, %ecx
	shrl	$29, %ecx
	leal	(%edx,%ecx), %eax
	andl	$7, %eax
	subl	%ecx, %eax
	movl	%eax, %ecx
	movl	$1, %eax
	sall	%cl, %eax
	notl	%eax
	andl	%ebx, %eax
	movl	%eax, _servo_z
L60:
	addl	$20, %esp
	popl	%ebx
	popl	%ebp
	ret
	.section	.text._Z9io_outpdwtm,"x"
	.p2align 1
__Z9io_outpdwtm:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	%edx, -8(%ebp)
	movw	%ax, -4(%ebp)
	movl	-4(%ebp), %eax
	andl	$65535, %eax
	subl	$4, %esp
	pushl	-8(%ebp)
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
	leave
	ret
	.section	.text._Z12enable_MCINTm,"x"
	.p2align 1
__Z12enable_MCINTm:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	subl	$8, %esp
	pushl	$56
	pushl	$-1
	call	_mc_inp
	addl	$16, %esp
	movl	%eax, %edx
	movl	_mc, %ecx
	movl	$1, %eax
	sall	%cl, %eax
	notl	%eax
	andl	%edx, %eax
	subl	$4, %esp
	pushl	%eax
	pushl	$56
	pushl	$-1
	call	_mc_outp
	addl	$16, %esp
	movl	_md, %eax
	movl	_mcint_offset(,%eax,4), %ecx
	movl	8(%ebp), %eax
	sall	%cl, %eax
	movl	_mc, %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$0
	pushl	%edx
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text._Z13disable_MCINTv,"x"
	.p2align 1
__Z13disable_MCINTv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	_mc, %eax
	subl	$4, %esp
	pushl	$0
	pushl	$0
	pushl	%eax
	call	_mc_outp
	addl	$16, %esp
	subl	$8, %esp
	pushl	$56
	pushl	$-1
	call	_mc_inp
	addl	$16, %esp
	movl	%eax, %edx
	movl	_mc, %ecx
	movl	$1, %eax
	sall	%cl, %eax
	orl	%edx, %eax
	subl	$4, %esp
	pushl	%eax
	pushl	$56
	pushl	$-1
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text._Z15clear_INTSTATUSv,"x"
	.p2align 1
__Z15clear_INTSTATUSv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	_mc, %eax
	subl	$4, %esp
	pushl	$16777215
	pushl	$4
	pushl	%eax
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text._Z15write_mc_pciregjm,"x"
	.p2align 1
__Z15write_mc_pciregjm:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	8(%ebp), %eax
	subl	$2147450880, %eax
	movl	%eax, -4(%ebp)
	call	_io_DisableINT
	movl	-4(%ebp), %edx
	andl	$-4, %edx
	movl	$3320, %eax
	call	__Z9io_outpdwtm
	movl	12(%ebp), %edx
	movl	$3324, %eax
	call	__Z9io_outpdwtm
	call	_io_EnableINT
	leave
	ret
	.section	.text._Z8io_inpdwt,"x"
	.p2align 1
__Z8io_inpdwt:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movw	%ax, -4(%ebp)
	movl	-4(%ebp), %eax
	andl	$65535, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	$0
	call	_io_In32
	addl	$16, %esp
	leave
	ret
	.section	.text._Z14read_mc_pciregj,"x"
	.p2align 1
__Z14read_mc_pciregj:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	8(%ebp), %eax
	subl	$2147450880, %eax
	movl	%eax, -4(%ebp)
	call	_io_DisableINT
	movl	-4(%ebp), %edx
	andl	$-4, %edx
	movl	$3320, %eax
	call	__Z9io_outpdwtm
	movl	$3324, %eax
	call	__Z8io_inpdwt
	movl	%eax, -8(%ebp)
	call	_io_EnableINT
	movl	-8(%ebp), %eax
	leave
	ret
	.section	.text._Z8GetMCIRQv,"x"
	.p2align 1
__Z8GetMCIRQv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	subl	$12, %esp
	pushl	$60
	call	__Z14read_mc_pciregj
	addl	$16, %esp
	andl	$255, %eax
	leave
	ret
	.section .text
LC4:
	.ascii "irq_init fail\0"
LC5:
	.ascii "irq_install fail\0"
	.section	.text._Z11init_mc_irqv,"x"
	.p2align 1
__Z11init_mc_irqv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	call	_irq_Init
	xorl	$1, %eax
	testb	%al, %al
	je	L78
	subl	$12, %esp
	pushl	$LC4
	call	_puts
	addl	$16, %esp
	movl	$0, -4(%ebp)
	jmp	L80
L78:
	call	__Z8GetMCIRQv
	andl	$255, %eax
	subl	$8, %esp
	pushl	$1
	pushl	%eax
	call	_irq_Setting
	addl	$16, %esp
	xorl	$1, %eax
	testb	%al, %al
	je	L81
	subl	$12, %esp
	pushl	$__ZZ11init_mc_irqvE12__FUNCTION__
	call	_puts
	addl	$16, %esp
	movl	$0, -4(%ebp)
	jmp	L80
L81:
	call	__Z8GetMCIRQv
	andl	$255, %eax
	subl	$4, %esp
	pushl	$0
	pushl	$__Z11isr_handleriPv
	pushl	%eax
	call	_irq_InstallISR
	addl	$16, %esp
	xorl	$1, %eax
	testb	%al, %al
	je	L83
	subl	$12, %esp
	pushl	$LC5
	call	_puts
	addl	$16, %esp
	movl	$0, -4(%ebp)
	jmp	L80
L83:
	movl	$1, -4(%ebp)
L80:
	movl	-4(%ebp), %eax
	leave
	ret
	.section	.text._Z15write_sbf1_regbhh,"x"
	.p2align 1
__Z15write_sbf1_regbhh:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	8(%ebp), %eax
	movl	12(%ebp), %edx
	movb	%al, -20(%ebp)
	movb	%dl, -24(%ebp)
	movl	$0, %eax
	movb	-20(%ebp), %al
	andl	$3, %eax
	sall	$3, %eax
	movl	%eax, -4(%ebp)
	call	_io_DisableINT
	movl	$0, %eax
	movb	-20(%ebp), %al
	subl	$2147469056, %eax
	movl	%eax, %edx
	andl	$-4, %edx
	movl	$3320, %eax
	call	__Z9io_outpdwtm
	movl	$3324, %eax
	call	__Z8io_inpdwt
	movl	%eax, %edx
	movl	-4(%ebp), %ecx
	movl	$255, %eax
	sall	%cl, %eax
	notl	%eax
	andl	%eax, %edx
	movl	$0, %eax
	movb	-24(%ebp), %al
	movl	-4(%ebp), %ecx
	sall	%cl, %eax
	orl	%eax, %edx
	movl	$3324, %eax
	call	__Z9io_outpdwtm
	call	_io_EnableINT
	leave
	ret
	.section	.text._Z14read_sbf1_regbh,"x"
	.p2align 1
__Z14read_sbf1_regbh:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	8(%ebp), %eax
	movb	%al, -20(%ebp)
	call	_io_DisableINT
	movl	$0, %eax
	movb	-20(%ebp), %al
	subl	$2147469056, %eax
	movl	%eax, %edx
	andl	$-4, %edx
	movl	$3320, %eax
	call	__Z9io_outpdwtm
	movl	$3324, %eax
	call	__Z8io_inpdwt
	movl	%eax, %edx
	movl	$0, %eax
	movb	-20(%ebp), %al
	andl	$3, %eax
	leal	0(,%eax,8), %ecx
	movl	%edx, %eax
	shrl	%cl, %eax
	movb	%al, -1(%ebp)
	call	_io_EnableINT
	movl	$0, %eax
	movb	-1(%ebp), %al
	leave
	ret
	.section	.text._Z9Set_MCIRQh,"x"
	.p2align 1
__Z9Set_MCIRQh:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	movb	%al, -4(%ebp)
	subl	$12, %esp
	pushl	$60
	call	__Z14read_mc_pciregj
	addl	$16, %esp
	movl	%eax, %edx
	movb	$0, %dl
	movl	$0, %eax
	movb	-4(%ebp), %al
	orl	%edx, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	$60
	call	__Z15write_mc_pciregjm
	addl	$16, %esp
	subl	$12, %esp
	pushl	$180
	call	__Z14read_sbf1_regbh
	addl	$16, %esp
	movb	%al, %dl
	movb	$-16, %al
	andl	%eax, %edx
	movl	$0, %eax
	movb	-4(%ebp), %al
	movb	_int_routing_table(%eax), %al
	orl	%edx, %eax
	andl	$255, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	$180
	call	__Z15write_sbf1_regbhh
	addl	$16, %esp
	leave
	ret
	.section .text
	.p2align 2
LC6:
	.ascii "ERROR: fail to change MC mode!\0"
LC7:
	.ascii "Init MC IRQ fail\0"
	.section	.text._Z11MCM_Controlf,"x"
	.p2align 1
__Z11MCM_Controlf:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$20, %esp
	movl	_mc_md_use, %eax
	testl	%eax, %eax
	jne	L93
	call	__Z8GetMCIRQv
	andl	$255, %eax
	subl	$12, %esp
	pushl	%eax
	call	__Z9Set_MCIRQh
	addl	$16, %esp
	call	_mc_setbaseaddr
	movl	%eax, _MCM_baseaddr
	movl	_mc, %eax
	subl	$8, %esp
	pushl	$1
	pushl	%eax
	call	_mc_SetMode
	addl	$16, %esp
	xorl	$1, %eax
	testb	%al, %al
	je	L95
	subl	$12, %esp
	pushl	$LC6
	call	_puts
	addl	$16, %esp
L95:
	movl	_md, %eax
	movl	_mc, %edx
	subl	$4, %esp
	pushl	$0
	pushl	%eax
	pushl	%edx
	call	_mcpwm_SetOutMask
	addl	$16, %esp
	movl	_md, %eax
	movl	_mc, %edx
	subl	$4, %esp
	pushl	$0
	pushl	%eax
	pushl	%edx
	call	_mcpwm_SetOutPolarity
	addl	$16, %esp
	movl	_md, %eax
	movl	_mc, %edx
	subl	$4, %esp
	pushl	$0
	pushl	%eax
	pushl	%edx
	call	_mcpwm_SetDeadband
	addl	$16, %esp
	movl	_md, %eax
	movl	_mc, %edx
	subl	$4, %esp
	pushl	$112
	pushl	%eax
	pushl	%edx
	call	_mcpwm_ReloadOUT_Unsafe
	addl	$16, %esp
	movl	_md, %eax
	movl	_mc, %edx
	subl	$4, %esp
	pushl	$-2147483648
	pushl	%eax
	pushl	%edx
	call	_mcpwm_SetWaveform
	addl	$16, %esp
	movl	_md, %eax
	movl	_mc, %edx
	subl	$4, %esp
	pushl	$4
	pushl	%eax
	pushl	%edx
	call	_mcpwm_SetSamplCycle
	addl	$16, %esp
	movl	_md, %eax
	movl	_mc, %edx
	subl	$4, %esp
	pushl	$805306368
	pushl	%eax
	pushl	%edx
	call	_mcpwm_SetOutPolarity
	addl	$16, %esp
	movl	_md, %eax
	movl	_mc, %edx
	subl	$4, %esp
	pushl	$3
	pushl	%eax
	pushl	%edx
	call	_mcpwm_SetDeadband
	addl	$16, %esp
	movl	_md, %eax
	movl	_mc, %edx
	subl	$4, %esp
	pushl	$16
	pushl	%eax
	pushl	%edx
	call	_mcpwm_ReloadOUT_Unsafe
	addl	$16, %esp
	call	__Z11init_mc_irqv
	xorl	$1, %eax
	testb	%al, %al
	je	L97
	subl	$12, %esp
	pushl	$LC7
	call	_puts
	addl	$16, %esp
	jmp	L100
L97:
	call	__Z15clear_INTSTATUSv
	subl	$12, %esp
	pushl	$1
	call	__Z12enable_MCINTm
	addl	$16, %esp
	flds	8(%ebp)
	fldl	LC8
	fmulp	%st, %st(1)
	fldl	LC9
	fmulp	%st, %st(1)
	fnstcw	-10(%ebp)
	movw	-10(%ebp), %ax
	movb	$12, %ah
	movw	%ax, -12(%ebp)
	fldcw	-12(%ebp)
	fistpq	-24(%ebp)
	fldcw	-10(%ebp)
	movl	-24(%ebp), %eax
	movl	-20(%ebp), %edx
	movl	%eax, %ebx
	flds	8(%ebp)
	fldl	LC9
	fmulp	%st, %st(1)
	fldcw	-12(%ebp)
	fistpq	-24(%ebp)
	fldcw	-10(%ebp)
	movl	-24(%ebp), %eax
	movl	-20(%ebp), %edx
	movl	_md, %edx
	movl	_mc, %ecx
	pushl	%ebx
	pushl	%eax
	pushl	%edx
	pushl	%ecx
	call	_mcpwm_SetWidth
	addl	$16, %esp
	movl	_md, %eax
	movl	_mc, %edx
	subl	$4, %esp
	pushl	$16
	pushl	%eax
	pushl	%edx
	call	_mcpwm_ReloadPWM
	addl	$16, %esp
	jmp	L100
L93:
	flds	8(%ebp)
	fldl	LC8
	fmulp	%st, %st(1)
	fldl	LC9
	fmulp	%st, %st(1)
	fnstcw	-10(%ebp)
	movw	-10(%ebp), %ax
	movb	$12, %ah
	movw	%ax, -12(%ebp)
	fldcw	-12(%ebp)
	fistpq	-24(%ebp)
	fldcw	-10(%ebp)
	movl	-24(%ebp), %eax
	movl	-20(%ebp), %edx
	movl	%eax, %ebx
	flds	8(%ebp)
	fldl	LC9
	fmulp	%st, %st(1)
	fldcw	-12(%ebp)
	fistpq	-24(%ebp)
	fldcw	-10(%ebp)
	movl	-24(%ebp), %eax
	movl	-20(%ebp), %edx
	movl	_md, %edx
	movl	_mc, %ecx
	pushl	%ebx
	pushl	%eax
	pushl	%edx
	pushl	%ecx
	call	_mcpwm_SetWidth
	addl	$16, %esp
	movl	_md, %eax
	movl	_mc, %edx
	subl	$4, %esp
	pushl	$16
	pushl	%eax
	pushl	%edx
	call	_mcpwm_ReloadPWM
	addl	$16, %esp
L100:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._Z17handle_interruptsv,"x"
	.p2align 1
__Z17handle_interruptsv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	_interrupts_times, %eax
	testl	%eax, %eax
	jne	L104
	movl	_servos+16, %eax
	andl	$255, %eax
	subl	$8, %esp
	pushl	$1
	pushl	%eax
	call	__Z18digitalWrite_servohh
	addl	$16, %esp
	movl	_servos+20, %eax
	movl	$0, %edx
	pushl	%edx
	pushl	%eax
	fildq	(%esp)
	leal	8(%esp), %esp
	subl	$12, %esp
	leal	-4(%esp), %esp
	fstps	(%esp)
	call	__Z11MCM_Controlf
	addl	$16, %esp
	movl	_servo_y, %eax
	movl	$0, %edx
	movb	%al, %dl
	movl	_servo_x, %eax
	andl	$65535, %eax
	call	__Z8io_outpbth
	movl	_md, %eax
	movl	_mc, %edx
	subl	$8, %esp
	pushl	%eax
	pushl	%edx
	call	_mcpwm_Enable
	addl	$16, %esp
	movl	_servos+48, %eax
	movl	$0, %edx
	pushl	%edx
	pushl	%eax
	fildq	(%esp)
	leal	8(%esp), %esp
	subl	$12, %esp
	leal	-4(%esp), %esp
	fstps	(%esp)
	call	__Z11MCM_Controlf
	addl	$16, %esp
	movl	$1, _mc_md_use
	movl	$1, _interrupts_times
L104:
	leave
	ret
	.section	.text._ZN5Servo17writeMicrosecondsEi,"x"
	.p2align 1
.globl __ZN5Servo17writeMicrosecondsEi
__ZN5Servo17writeMicrosecondsEi:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$20, %esp
	movl	8(%ebp), %eax
	movb	(%eax), %al
	movb	%al, -5(%ebp)
	cmpb	$11, -5(%ebp)
	ja	L118
	movl	8(%ebp), %eax
	movb	1(%eax), %al
	movsbl	%al,%eax
	leal	0(,%eax,4), %edx
	movl	$544, %eax
	subl	%edx, %eax
	cmpl	12(%ebp), %eax
	jle	L108
	movl	8(%ebp), %eax
	movb	1(%eax), %al
	movsbl	%al,%eax
	leal	0(,%eax,4), %edx
	movl	$544, %eax
	subl	%edx, %eax
	movl	%eax, 12(%ebp)
	jmp	L110
L108:
	movl	8(%ebp), %eax
	movb	2(%eax), %al
	movsbl	%al,%eax
	leal	0(,%eax,4), %edx
	movl	$2400, %eax
	subl	%edx, %eax
	cmpl	12(%ebp), %eax
	jge	L110
	movl	8(%ebp), %eax
	movb	2(%eax), %al
	movsbl	%al,%eax
	leal	0(,%eax,4), %edx
	movl	$2400, %eax
	subl	%edx, %eax
	movl	%eax, 12(%ebp)
L110:
	movl	$0, %eax
	movb	-5(%ebp), %al
	movl	12(%ebp), %ecx
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	%ecx, _servos+4(%eax)
	movl	$0, -12(%ebp)
	jmp	L112
L113:
	movl	$0, %eax
	movb	-5(%ebp), %al
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movb	_servos(%eax), %al
	andl	$63, %eax
	movl	$0, %ecx
	movb	%al, %cl
	movl	-12(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+8(%eax), %eax
	cmpl	%eax, %ecx
	jne	L114
	movl	$0, %eax
	movb	-5(%ebp), %al
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+4(%eax), %ecx
	movl	-12(%ebp), %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+12(%eax), %eax
	cmpl	%eax, %ecx
	je	L114
	movl	-12(%ebp), %ebx
	movl	$0, %eax
	movb	-5(%ebp), %al
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+4(%eax), %ecx
	movl	%ebx, %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	%ecx, _servos+12(%eax)
	call	__Z13insertionSortv
L114:
	incl	-12(%ebp)
L112:
	movb	_ServoCount_use, %al
	andl	$255, %eax
	cmpl	-12(%ebp), %eax
	jg	L113
	call	__Z17handle_interruptsv
L118:
	addl	$20, %esp
	popl	%ebx
	popl	%ebp
	ret
	.section	.text._ZN5Servo5writeEi,"x"
	.p2align 1
.globl __ZN5Servo5writeEi
__ZN5Servo5writeEi:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	cmpl	$543, 12(%ebp)
	jg	L120
	cmpl	$0, 12(%ebp)
	jns	L122
	movl	$0, 12(%ebp)
L122:
	cmpl	$180, 12(%ebp)
	jle	L124
	movl	$180, 12(%ebp)
L124:
	movl	8(%ebp), %eax
	movb	2(%eax), %al
	movsbl	%al,%eax
	leal	0(,%eax,4), %edx
	movl	$2400, %eax
	movl	%eax, %ecx
	subl	%edx, %ecx
	movl	8(%ebp), %eax
	movb	1(%eax), %al
	movsbl	%al,%eax
	leal	0(,%eax,4), %edx
	movl	$544, %eax
	subl	%edx, %eax
	subl	$12, %esp
	pushl	%ecx
	pushl	%eax
	pushl	$180
	pushl	$0
	pushl	12(%ebp)
	call	__Z3maplllll
	addl	$32, %esp
	movl	%eax, 12(%ebp)
L120:
	subl	$8, %esp
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	__ZN5Servo17writeMicrosecondsEi
	addl	$16, %esp
	leave
	ret
	.section	.text._Z11isr_handleriPv,"x"
	.p2align 1
__Z11isr_handleriPv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	_mc, %eax
	leal	1(%eax), %ecx
	movl	$1, %eax
	movl	%eax, %edx
	sall	%cl, %edx
	movl	_MCM_baseaddr, %eax
	addl	$208, %eax
	andl	$65535, %eax
	call	__Z9io_outpdwtm
	movl	_md, %eax
	movl	_mcint_offset(,%eax,4), %ecx
	movl	$1, %eax
	sall	%cl, %eax
	movl	%eax, %edx
	movl	_MCM_baseaddr, %eax
	addl	$4, %eax
	andl	$65535, %eax
	call	__Z9io_outpdwtm
	movb	_ServoCount_use, %al
	movl	$0, %edx
	movb	%al, %dl
	movl	_settime, %eax
	cmpl	%eax, %edx
	je	L128
	movl	_settime, %eax
	testl	%eax, %eax
	js	L130
	movb	_ServoCount_use, %al
	movl	$0, %edx
	movb	%al, %dl
	movl	_settime, %eax
	cmpl	%eax, %edx
	je	L130
	movl	_settime, %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+16(%eax), %eax
	andl	$255, %eax
	subl	$8, %esp
	pushl	$0
	pushl	%eax
	call	__Z18digitalWrite_servohh
	addl	$16, %esp
	movl	_servo_z, %eax
	movl	$0, %edx
	movb	%al, %dl
	movl	_servo_x, %eax
	andl	$65535, %eax
	call	__Z8io_outpbth
L130:
	movl	_settime, %eax
	leal	2(%eax), %edx
	movb	_ServoCount_use, %al
	andl	$255, %eax
	cmpl	%eax, %edx
	jge	L133
	movl	_settime, %eax
	addl	$2, %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+16(%eax), %eax
	andl	$255, %eax
	subl	$8, %esp
	pushl	$1
	pushl	%eax
	call	__Z18digitalWrite_servohh
	addl	$16, %esp
	movl	_servo_y, %eax
	movl	$0, %edx
	movb	%al, %dl
	movl	_servo_x, %eax
	andl	$65535, %eax
	call	__Z8io_outpbth
L133:
	movl	_settime, %eax
	addl	$3, %eax
	leal	0(,%eax,4), %edx
	leal	0(,%edx,8), %eax
	subl	%edx, %eax
	movl	_servos+20(%eax), %eax
	movl	$0, %edx
	pushl	%edx
	pushl	%eax
	fildq	(%esp)
	leal	8(%esp), %esp
	subl	$12, %esp
	leal	-4(%esp), %esp
	fstps	(%esp)
	call	__Z11MCM_Controlf
	addl	$16, %esp
	movl	_settime, %eax
	incl	%eax
	movl	%eax, _settime
L128:
	movl	$1, %eax
	leave
	ret
.globl _interrupts_times
	.section	.bss.interrupts_times,"w"
	.p2align 2
_interrupts_times:
	.space 4
.globl _interrupts_width
	.section	.bss.interrupts_width,"w"
	.p2align 2
_interrupts_width:
	.space 4
.globl _ServoCount
	.section	.bss.ServoCount,"w"
_ServoCount:
	.space 1
.globl _ServoCount_use
	.section	.bss.ServoCount_use,"w"
_ServoCount_use:
	.space 1
.globl _end_wait
	.section	.bss.end_wait,"w"
_end_wait:
	.space 1
.globl _Servo_wait
	.section	.bss.Servo_wait,"w"
_Servo_wait:
	.space 1
.globl _xxx
	.section	.data.xxx,"w"
	.p2align 2
_xxx:
	.long	5000
.globl _settime
	.section	.bss.settime,"w"
	.p2align 2
_settime:
	.space 4
.globl _yy
	.section	.bss.yy,"w"
	.p2align 2
_yy:
	.space 4
	.section	.data.MC_list,"w"
	.p2align 4
_MC_list:
	.long	0
	.long	1
	.long	2
	.long	3
	.section	.data.arduint_to_mc_md,"w"
	.p2align 4
_arduint_to_mc_md:
	.long	-1
	.long	-1
	.long	-1
	.space 4
	.long	-1
	.space 8
	.long	-1
	.long	-1
	.space 12
	.long	-1
	.space 4
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.space 16
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.space 4
	.long	-1
	.space 8
	.long	-1
	.long	-1
	.space 12
	.long	-1
	.space 4
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.space 16
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.long	-1
	.section	.data.MD_list,"w"
	.p2align 2
_MD_list:
	.long	0
	.long	1
	.long	2
	.section	.bss.mc,"w"
	.p2align 2
_mc:
	.space 4
	.section	.bss.md,"w"
	.p2align 2
_md:
	.space 4
	.section	.bss.servos,"w"
	.p2align 4
_servos:
	.space 336
	.section	.bss.servo_y,"w"
	.p2align 2
_servo_y:
	.space 4
	.section	.bss.servo_x,"w"
	.p2align 2
_servo_x:
	.space 4
	.section	.bss.mc_md_use,"w"
	.p2align 2
_mc_md_use:
	.space 4
	.section	.data.pinMap,"w"
	.p2align 4
_pinMap:
	.long	11
	.long	10
	.long	39
	.long	23
	.long	37
	.long	20
	.long	19
	.long	35
	.long	33
	.long	17
	.long	28
	.long	27
	.long	32
	.long	25
	.long	12
	.long	13
	.long	14
	.long	15
	.long	24
	.long	26
	.long	29
	.long	47
	.long	46
	.long	45
	.long	44
	.long	43
	.long	42
	.long	41
	.long	40
	.long	1
	.long	3
	.long	4
	.long	31
	.long	0
	.long	2
	.long	5
	.long	22
	.long	30
	.long	6
	.long	38
	.long	36
	.long	34
	.long	16
	.long	18
	.long	21
	.section	.bss.crossbar_bit,"w"
	.p2align 2
_crossbar_bit:
	.space 4
	.section	.bss.servo_z,"w"
	.p2align 2
_servo_z:
	.space 4
	.section	.bss.MCM_baseaddr,"w"
	.p2align 2
_MCM_baseaddr:
	.space 4
	.section	.rodata._ZZ11init_mc_irqvE12__FUNCTION__,""
__ZZ11init_mc_irqvE12__FUNCTION__:
	.ascii "init_mc_irq\0"
	.section	.data.mcint_offset,"w"
	.p2align 2
_mcint_offset:
	.long	0
	.long	8
	.long	16
	.section	.data.int_routing_table,"w"
	.p2align 4
_int_routing_table:
	.byte	-1
	.byte	8
	.byte	-1
	.byte	2
	.byte	4
	.byte	5
	.byte	7
	.byte	6
	.byte	-1
	.byte	1
	.byte	3
	.byte	9
	.byte	11
	.byte	-1
	.byte	13
	.byte	15
	.section .text
	.p2align 3
LC8:
	.long	0
	.long	1071644672
	.p2align 3
LC9:
	.long	0
	.long	1079574528
	.ident	"GCC: (GNU) 4.2.3"
