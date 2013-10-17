	.file	"can.cpp"
	.section .text
	.p2align 1
__Z18can_RoundRobin_endv:
LFB35:
	pushl	%ebp
LCFI0:
	movl	%esp, %ebp
LCFI1:
	popl	%ebp
	ret
LFE35:
	.p2align 1
__Z11CAN_ISR_endv:
LFB37:
	pushl	%ebp
LCFI2:
	movl	%esp, %ebp
LCFI3:
	popl	%ebp
	ret
LFE37:
	.p2align 1
.globl __Z14can_RoundRobinP7can_bus
__Z14can_RoundRobinP7can_bus:
LFB34:
	pushl	%ebp
LCFI4:
	movl	%esp, %ebp
LCFI5:
	subl	$8, %esp
LCFI6:
	movl	8(%ebp), %eax
	movl	576(%eax), %eax
	subl	$12, %esp
	pushl	%eax
LCFI7:
	call	_QueueEmpty
	addl	$16, %esp
	testb	%al, %al
	jne	L11
LCFI8:
	call	_io_DisableINT
	movl	8(%ebp), %eax
	movl	600(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
LCFI9:
	call	_io_In32
	addl	$16, %esp
	movl	%eax, %edx
	movl	_round, %eax
	leal	(%eax,%eax), %ecx
	movl	$1, %eax
	sall	%cl, %eax
	andl	%edx, %eax
	testl	%eax, %eax
	sete	%al
	testb	%al, %al
	je	L8
	movl	8(%ebp), %edx
	addl	$552, %edx
	movl	8(%ebp), %eax
	movl	576(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_PopBufQueue
	addl	$16, %esp
	movl	8(%ebp), %eax
	movb	552(%eax), %al
	movl	$0, %ecx
	movb	%al, %cl
	movl	_round, %eax
	movl	8(%ebp), %edx
	sall	$4, %eax
	addl	%edx, %eax
	addl	$80, %eax
	movw	14(%eax), %ax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	%ecx
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
	movl	8(%ebp), %eax
	movl	556(%eax), %ecx
	movl	_round, %eax
	movl	8(%ebp), %edx
	sall	$4, %eax
	addl	%edx, %eax
	addl	$80, %eax
	movl	16(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	%ecx
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
	movl	8(%ebp), %eax
	movl	560(%eax), %ecx
	movl	_round, %eax
	movl	8(%ebp), %edx
	sall	$4, %eax
	addl	%edx, %eax
	addl	$80, %eax
	movw	18(%eax), %ax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	%ecx
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
	movl	8(%ebp), %eax
	movl	564(%eax), %ecx
	movl	_round, %eax
	movl	8(%ebp), %edx
	sall	$4, %eax
	addl	%edx, %eax
	addl	$80, %eax
	movl	20(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	%ecx
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
	movl	8(%ebp), %eax
	movl	600(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_io_In32
	addl	$16, %esp
	movl	%eax, %edx
	movl	_round, %eax
	leal	(%eax,%eax), %ecx
	movl	$1, %eax
	sall	%cl, %eax
	movl	%edx, %ecx
	orl	%eax, %ecx
	movl	8(%ebp), %eax
	movl	600(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	%ecx
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
	movl	_round, %eax
	incl	%eax
	movl	%eax, _round
	movl	_round, %eax
	cmpl	$3, %eax
	jne	L8
	movl	$0, _round
L8:
LCFI10:
	call	_io_RestoreINT
L11:
	leave
	ret
LFE34:
	.p2align 1
__Z7CAN_ISRiPv:
LFB36:
	pushl	%ebp
LCFI11:
	movl	%esp, %ebp
LCFI12:
	subl	$40, %esp
LCFI13:
	movl	12(%ebp), %eax
	movl	%eax, -20(%ebp)
	movl	-20(%ebp), %eax
	movl	596(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
LCFI14:
	call	_io_In32
	addl	$16, %esp
	movl	%eax, -24(%ebp)
	movl	-24(%ebp), %eax
	andl	$2047, %eax
	testl	%eax, %eax
	setne	%al
	testb	%al, %al
	je	L13
	movl	-24(%ebp), %eax
	andl	$1, %eax
	testb	%al, %al
	je	L15
LCFI15:
	call	_io_DisableINT
	movl	-20(%ebp), %eax
	movw	142(%eax), %ax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
LCFI16:
	call	_io_In32
	addl	$16, %esp
	movb	%al, %dl
	movl	-20(%ebp), %eax
	movb	%dl, 536(%eax)
	movl	-20(%ebp), %eax
	movl	144(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_io_In32
	addl	$16, %esp
	movl	%eax, %edx
	movl	-20(%ebp), %eax
	movl	%edx, 540(%eax)
	movl	-20(%ebp), %eax
	movl	148(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_io_In32
	addl	$16, %esp
	movl	%eax, %edx
	movl	-20(%ebp), %eax
	movl	%edx, 548(%eax)
	movl	-20(%ebp), %eax
	movw	146(%eax), %ax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_io_In32
	addl	$16, %esp
	movl	%eax, %edx
	movl	-20(%ebp), %eax
	movl	%edx, 544(%eax)
	movl	-20(%ebp), %edx
	addl	$536, %edx
	movl	-20(%ebp), %eax
	movl	572(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_PushBufQueue
	addl	$16, %esp
LCFI17:
	call	_io_RestoreINT
	movl	-20(%ebp), %eax
	movl	600(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	$256
	pushl	%edx
	pushl	%eax
LCFI18:
	call	_io_Out32
	addl	$16, %esp
	movl	-20(%ebp), %eax
	movl	596(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	$1
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
L15:
	movl	-24(%ebp), %eax
	andl	$2, %eax
	testl	%eax, %eax
	je	L17
	movl	-20(%ebp), %eax
	movl	92(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_io_In32
	addl	$16, %esp
	movl	%eax, -16(%ebp)
	movl	-16(%ebp), %eax
	andl	$32, %eax
	testl	%eax, %eax
	je	L19
	movl	-20(%ebp), %eax
	movl	88(%eax), %eax
	leal	1(%eax), %edx
	movl	-20(%ebp), %eax
	movl	%edx, 88(%eax)
	movl	-16(%ebp), %eax
	andl	$458752, %eax
	shrl	$16, %eax
	leal	64(%eax), %edx
	movl	-20(%ebp), %eax
	movb	%dl, 64(%eax)
	movl	-20(%ebp), %eax
	movb	64(%eax), %al
	movl	$0, %edx
	movb	%al, %dl
	movl	-20(%ebp), %eax
	movl	568(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_PushQueue
	addl	$16, %esp
	movl	-20(%ebp), %eax
	movl	580(%eax), %eax
	testl	%eax, %eax
	je	L19
	movl	-20(%ebp), %eax
	movl	580(%eax), %eax
	subl	$12, %esp
	pushl	-20(%ebp)
	call	*%eax
	addl	$16, %esp
L19:
	movl	-16(%ebp), %eax
	andl	$7, %eax
	cmpl	$5, %eax
	jne	L22
	subl	$12, %esp
	pushl	-20(%ebp)
	call	__Z14can_RoundRobinP7can_bus
	addl	$16, %esp
L22:
	movl	-20(%ebp), %eax
	movl	596(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	$2
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
L17:
	movl	-24(%ebp), %eax
	andl	$4, %eax
	testl	%eax, %eax
	je	L24
	movl	-20(%ebp), %eax
	movl	108(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_io_In32
	addl	$16, %esp
	movl	%eax, -12(%ebp)
	movl	-12(%ebp), %eax
	andl	$32, %eax
	testl	%eax, %eax
	je	L26
	movl	-20(%ebp), %eax
	movl	104(%eax), %eax
	leal	1(%eax), %edx
	movl	-20(%ebp), %eax
	movl	%edx, 104(%eax)
	movl	-12(%ebp), %eax
	andl	$458752, %eax
	shrl	$16, %eax
	leal	-128(%eax), %edx
	movl	-20(%ebp), %eax
	movb	%dl, 64(%eax)
	movl	-20(%ebp), %eax
	movb	64(%eax), %al
	movl	$0, %edx
	movb	%al, %dl
	movl	-20(%ebp), %eax
	movl	568(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_PushQueue
	addl	$16, %esp
	movl	-20(%ebp), %eax
	movl	580(%eax), %eax
	testl	%eax, %eax
	je	L26
	movl	-20(%ebp), %eax
	movl	580(%eax), %eax
	subl	$12, %esp
	pushl	-20(%ebp)
	call	*%eax
	addl	$16, %esp
L26:
	movl	-12(%ebp), %eax
	andl	$7, %eax
	cmpl	$5, %eax
	jne	L29
	subl	$12, %esp
	pushl	-20(%ebp)
	call	__Z14can_RoundRobinP7can_bus
	addl	$16, %esp
L29:
	movl	-20(%ebp), %eax
	movl	596(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	$4
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
L24:
	movl	-24(%ebp), %eax
	andl	$8, %eax
	testl	%eax, %eax
	je	L31
	movl	-20(%ebp), %eax
	movl	124(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_io_In32
	addl	$16, %esp
	movl	%eax, -8(%ebp)
	movl	-8(%ebp), %eax
	andl	$32, %eax
	testl	%eax, %eax
	je	L33
	movl	-20(%ebp), %eax
	movl	120(%eax), %eax
	leal	1(%eax), %edx
	movl	-20(%ebp), %eax
	movl	%edx, 120(%eax)
	movl	-8(%ebp), %eax
	andl	$458752, %eax
	shrl	$16, %eax
	leal	-64(%eax), %edx
	movl	-20(%ebp), %eax
	movb	%dl, 64(%eax)
	movl	-20(%ebp), %eax
	movb	64(%eax), %al
	movl	$0, %edx
	movb	%al, %dl
	movl	-20(%ebp), %eax
	movl	568(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_PushQueue
	addl	$16, %esp
	movl	-20(%ebp), %eax
	movl	580(%eax), %eax
	testl	%eax, %eax
	je	L33
	movl	-20(%ebp), %eax
	movl	580(%eax), %eax
	subl	$12, %esp
	pushl	-20(%ebp)
	call	*%eax
	addl	$16, %esp
L33:
	movl	-8(%ebp), %eax
	andl	$7, %eax
	cmpl	$5, %eax
	jne	L36
	subl	$12, %esp
	pushl	-20(%ebp)
	call	__Z14can_RoundRobinP7can_bus
	addl	$16, %esp
L36:
	movl	-20(%ebp), %eax
	movl	596(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	$8
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
L31:
	movl	-24(%ebp), %eax
	andl	$16, %eax
	testl	%eax, %eax
	je	L38
	movl	-20(%ebp), %eax
	movl	$1, 60(%eax)
	movl	-20(%ebp), %eax
	movl	596(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	$16
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
L38:
	movl	-24(%ebp), %eax
	andl	$32, %eax
	testl	%eax, %eax
	je	L40
	movl	-20(%ebp), %eax
	movl	$2, 60(%eax)
	movl	-20(%ebp), %eax
	movl	596(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	$32
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
L40:
	movl	-24(%ebp), %eax
	andl	$64, %eax
	testl	%eax, %eax
	je	L42
	movl	-20(%ebp), %eax
	movl	$3, 60(%eax)
	movl	-20(%ebp), %eax
	movl	596(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	$64
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
L42:
	movl	-24(%ebp), %eax
	andl	$128, %eax
	testl	%eax, %eax
	je	L44
	movl	-20(%ebp), %eax
	movl	68(%eax), %eax
	leal	1(%eax), %edx
	movl	-20(%ebp), %eax
	movl	%edx, 68(%eax)
	movl	-20(%ebp), %eax
	movl	596(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	$128
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
L44:
	movl	-24(%ebp), %eax
	andl	$256, %eax
	testl	%eax, %eax
	je	L46
	movl	-20(%ebp), %eax
	movl	140(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_io_In32
	addl	$16, %esp
	movl	%eax, -4(%ebp)
	movl	-20(%ebp), %eax
	movl	136(%eax), %eax
	leal	1(%eax), %edx
	movl	-20(%ebp), %eax
	movl	%edx, 136(%eax)
	movl	-4(%ebp), %eax
	andl	$112, %eax
	shrl	$4, %eax
	movb	%al, %dl
	movl	-20(%ebp), %eax
	movb	%dl, 64(%eax)
	movl	-20(%ebp), %eax
	movb	64(%eax), %al
	movl	$0, %edx
	movb	%al, %dl
	movl	-20(%ebp), %eax
	movl	568(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_PushQueue
	addl	$16, %esp
	movl	-20(%ebp), %eax
	movl	580(%eax), %eax
	testl	%eax, %eax
	je	L48
	movl	-20(%ebp), %eax
	movl	580(%eax), %eax
	subl	$12, %esp
	pushl	-20(%ebp)
	call	*%eax
	addl	$16, %esp
L48:
	movl	-20(%ebp), %eax
	movl	596(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	$256
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
L46:
	movl	-24(%ebp), %eax
	andl	$512, %eax
	testl	%eax, %eax
	je	L50
	movl	-20(%ebp), %eax
	movl	72(%eax), %eax
	leal	1(%eax), %edx
	movl	-20(%ebp), %eax
	movl	%edx, 72(%eax)
	movl	-20(%ebp), %eax
	movl	596(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	$512
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
L50:
	movl	-24(%ebp), %eax
	andl	$1024, %eax
	testl	%eax, %eax
	je	L13
	movl	-20(%ebp), %eax
	movl	596(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-20(%ebp), %eax
	movl	(%eax), %eax
	subl	$4, %esp
	pushl	$1024
	pushl	%edx
	pushl	%eax
	call	_io_Out32
	addl	$16, %esp
L13:
	movl	$1, %eax
	leave
	ret
LFE36:
	.p2align 1
__Z9err_printPcz:
LFB32:
	pushl	%ebp
LCFI19:
	movl	%esp, %ebp
LCFI20:
	subl	$24, %esp
LCFI21:
	leal	12(%ebp), %eax
	movl	%eax, -4(%ebp)
	movl	-4(%ebp), %eax
	subl	$4, %esp
	pushl	%eax
	pushl	8(%ebp)
	pushl	$___dj_stderr
LCFI22:
	call	_vfprintf
	addl	$16, %esp
	subl	$12, %esp
	pushl	$___dj_stderr
	call	_fflush
	addl	$16, %esp
	leave
	ret
LFE32:
LC0:
	.ascii "%s: CAN bus is null.\12\0"
	.p2align 1
.globl _can_SetCANBusOfHandler
_can_SetCANBusOfHandler:
LFB65:
	pushl	%ebp
LCFI23:
	movl	%esp, %ebp
LCFI24:
	subl	$24, %esp
LCFI25:
	movl	8(%ebp), %eax
	movl	%eax, -4(%ebp)
	cmpl	$0, -4(%ebp)
	jne	L57
	subl	$8, %esp
	pushl	$__ZZ22can_SetCANBusOfHandlerE12__FUNCTION__
	pushl	$LC0
LCFI26:
	call	__Z9err_printPcz
	addl	$16, %esp
	jmp	L60
L57:
	movl	-4(%ebp), %edx
	movl	12(%ebp), %eax
	movl	%eax, 584(%edx)
L60:
	leave
	ret
LFE65:
	.p2align 1
.globl _can_SetCANErrorHandler
_can_SetCANErrorHandler:
LFB64:
	pushl	%ebp
LCFI27:
	movl	%esp, %ebp
LCFI28:
	subl	$24, %esp
LCFI29:
	movl	8(%ebp), %eax
	movl	%eax, -4(%ebp)
	cmpl	$0, -4(%ebp)
	jne	L62
	subl	$8, %esp
	pushl	$__ZZ22can_SetCANErrorHandlerE12__FUNCTION__
	pushl	$LC0
LCFI30:
	call	__Z9err_printPcz
	addl	$16, %esp
	jmp	L65
L62:
	movl	-4(%ebp), %edx
	movl	12(%ebp), %eax
	movl	%eax, 580(%edx)
L65:
	leave
	ret
LFE64:
LC1:
	.ascii "%s: CAN bus is not in use.\12\0"
	.p2align 1
.globl _can_FlushWFIFO
_can_FlushWFIFO:
LFB63:
	pushl	%ebp
LCFI31:
	movl	%esp, %ebp
LCFI32:
	subl	$24, %esp
LCFI33:
	movl	8(%ebp), %eax
	movl	%eax, -4(%ebp)
	cmpl	$0, -4(%ebp)
	jne	L67
	subl	$8, %esp
	pushl	$__ZZ14can_FlushWFIFOE12__FUNCTION__
	pushl	$LC0
LCFI34:
	call	__Z9err_printPcz
	addl	$16, %esp
	jmp	L73
L67:
	movl	-4(%ebp), %eax
	movl	20(%eax), %eax
	cmpl	$1, %eax
	je	L70
	subl	$8, %esp
	pushl	$__ZZ14can_FlushWFIFOE12__FUNCTION__
	pushl	$LC1
	call	__Z9err_printPcz
	addl	$16, %esp
	jmp	L73
L70:
	movl	-4(%ebp), %eax
	movl	576(%eax), %eax
	subl	$12, %esp
	pushl	%eax
	call	_QueueEmpty
	addl	$16, %esp
	xorl	$1, %eax
	testb	%al, %al
	jne	L70
L72:
	movl	-4(%ebp), %eax
	movl	600(%eax), %eax
	movl	%eax, %edx
	andl	$65535, %edx
	movl	-4(%ebp), %eax
	movl	(%eax), %eax
	subl	$8, %esp
	pushl	%edx
	pushl	%eax
	call	_io_In32
	addl	$16, %esp
	andl	$21, %eax
	testl	%eax, %eax
	setne	%al
	testb	%al, %al
	jne	L72
L73:
	leave
	ret
LFE63:
	.p2align 1
.globl _can_TxReady
_can_TxReady:
LFB62:
	pushl	%ebp
LCFI35:
	movl	%esp, %ebp
LCFI36:
	subl	$24, %esp
LCFI37:
	movl	8(%ebp), %eax
	movl	%eax, -4(%ebp)
	cmpl	$0, -4(%ebp)
	jne	L75
	subl	$8, %esp
	pushl	$__ZZ11can_TxReadyE12__FUNCTION__
	pushl	$LC0
LCFI38:
	call	__Z9err_printPcz
	addl	$16, %esp
	movl	$0, -24(%ebp)
	jmp	L77
L75:
	movl	-4(%ebp), %eax
	movl	576(%eax), %eax
	subl	$12, %esp
	pushl	%eax
	call	_QueueFull
	addl	$16, %esp
	testb	%al, %al
	je	L78
	movl	$0, -20(%ebp)
	jmp	L80
L78:
	movl	$1, -20(%ebp)
L80:
	movl	-20(%ebp), %eax
	movl	%eax, -24(%ebp)
L77:
	movl	-24(%ebp), %eax
	leave
	ret
LFE62:
	.p2align 1
.globl _can_FlushTxQueue
_can_FlushTxQueue:
LFB61:
	pushl	%ebp
LCFI39:
	movl	%esp, %ebp
LCFI40:
	subl	$24, %esp
LCFI41:
	movl	8(%ebp), %eax
	movl	%eax, -4(%ebp)
	cmpl	$0, -4(%ebp)
	jne	L83
	subl	$8, %esp
	pushl	$__ZZ16can_FlushTxQueueE12__FUNCTION__
	pushl	$LC0
LCFI42:
	call	__Z9err_printPcz
	addl	$16, %esp
	jmp	L86
L83:
	movl	-4(%ebp), %eax
	movl	576(%eax), %eax
	subl	$12, %esp
	pushl	%eax
	call	_ClearQueue
	addl	$16, %esp
L86:
	leave
	ret
LFE61:
	.p2align 1
.globl _can_TxQueueEmpty
_can_TxQueueEmpty:
LFB60:
	pushl	%ebp
LCFI43:
	movl	%esp, %ebp
LCFI44:
	subl	$24, %esp
LCFI45:
	movl	8(%ebp), %eax
	movl	%eax, -4(%ebp)
	cmpl	$0, -4(%ebp)
	jne	L88
	subl	$8, %esp
	pushl	$__ZZ16can_TxQueueEmptyE12__FUNCTION__
	pushl	$LC0
LCFI46:
	call	__Z9err_printPcz
	addl	$16, %esp
	movl	$0, -20(%ebp)
	jmp	L90
L88:
	movl	-4(%ebp), %eax
	movl	576(%eax), %eax
	subl	$12, %esp
	pushl	%eax
	call	_QueueEmpty
	addl	$16, %esp
	movl	$0, %edx
	movb	%al, %dl
	movl	%edx, -20(%ebp)
L90:
	movl	-20(%ebp), %eax
	leave
	ret
LFE60:
	.p2align 1
.globl _can_TxQueueFull
_can_TxQueueFull:
LFB59:
	pushl	%ebp
LCFI47:
	movl	%esp, %ebp
LCFI48:
	subl	$24, %esp
LCFI49:
	movl	8(%ebp), %eax
	movl	%eax, -4(%ebp)
	cmpl	$0, -4(%ebp)
	jne	L93
	subl	$8, %esp
	pushl	$__ZZ15can_TxQueueFullE12__FUNCTION__
	pushl	$LC0
LCFI50:
	call	__Z9err_printPcz
	addl	$16, %esp
	movl	$0, -20(%ebp