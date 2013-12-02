	.file	"uart.cpp"
	.section	.text.uart_QueryRxQueue,"x"
.globl _uart_QueryRxQueue
_uart_QueryRxQueue:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	movl	96(%eax), %eax
	movl	8(%eax), %eax
	popl	%ebp
	ret
	.section	.text.uart_RxQueueFull,"x"
.globl _uart_RxQueueFull
_uart_RxQueueFull:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	movl	96(%eax), %edx
	movl	8(%edx), %eax
	cmpl	12(%edx), %eax
	setae	%al
	popl	%ebp
	ret
	.section	.text.uart_RxQueueEmpty,"x"
.globl _uart_RxQueueEmpty
_uart_RxQueueEmpty:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	movl	96(%eax), %eax
	cmpl	$0, 8(%eax)
	sete	%al
	popl	%ebp
	ret
	.section	.text.uart_QueryTxQueue,"x"
.globl _uart_QueryTxQueue
_uart_QueryTxQueue:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	movl	100(%eax), %eax
	movl	8(%eax), %eax
	popl	%ebp
	ret
	.section	.text.uart_TxQueueFull,"x"
.globl _uart_TxQueueFull
_uart_TxQueueFull:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	movl	100(%eax), %edx
	movl	8(%edx), %eax
	cmpl	12(%edx), %eax
	setae	%al
	popl	%ebp
	ret
	.section	.text.uart_TxQueueEmpty,"x"
.globl _uart_TxQueueEmpty
_uart_TxQueueEmpty:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	movl	100(%eax), %eax
	cmpl	$0, 8(%eax)
	sete	%al
	popl	%ebp
	ret
	.section	.text.uart_TxReady,"x"
.globl _uart_TxReady
_uart_TxReady:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	movl	100(%eax), %edx
	movl	8(%edx), %eax
	cmpl	12(%edx), %eax
	setb	%al
	popl	%ebp
	ret
	.section	.text._ZL12UART_ISR_endv,"x"
__ZL12UART_ISR_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL9err_printPcz,"x"
__ZL9err_printPcz:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	leal	12(%ebp), %eax
	pushl	%eax
	pushl	8(%ebp)
	pushl	$___dj_stderr
	call	_vfprintf
	movl	$___dj_stderr, (%esp)
	call	_fflush
	addl	$16, %esp
	leave
	ret
	.section .text
LC0:
	.ascii "%s: port is null.\12\0"
	.section	.text.uart_SetMSRHandler,"x"
.globl _uart_SetMSRHandler
_uart_SetMSRHandler:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L20
	subl	$8, %esp
	pushl	$__ZZ18uart_SetMSRHandlerE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L22
L20:
	movl	12(%ebp), %edx
	movl	%edx, 108(%eax)
L22:
	leave
	ret
	.section	.text.uart_SetLSRHandler,"x"
.globl _uart_SetLSRHandler
_uart_SetLSRHandler:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L24
	subl	$8, %esp
	pushl	$__ZZ18uart_SetLSRHandlerE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L26
L24:
	movl	12(%ebp), %edx
	movl	%edx, 104(%eax)
L26:
	leave
	ret
	.section	.text.uart_SetWFIFOSize,"x"
.globl _uart_SetWFIFOSize
_uart_SetWFIFOSize:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	12(%ebp), %ecx
	movl	8(%ebp), %edx
	testl	%edx, %edx
	jne	L28
	subl	$8, %esp
	pushl	$__ZZ17uart_SetWFIFOSizeE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L29
L28:
	movb	$0, %al
	cmpl	%ecx, 88(%edx)
	jl	L29
	movl	%ecx, 92(%edx)
	movb	$1, %al
L29:
	leave
	ret
	.section	.text.uart_SetTimeOut,"x"
.globl _uart_SetTimeOut
_uart_SetTimeOut:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L33
	subl	$8, %esp
	pushl	$__ZZ15uart_SetTimeOutE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L35
L33:
	movl	12(%ebp), %edx
	movl	%edx, 84(%eax)
L35:
	leave
	ret
	.section	.text._ZL7io_inpbt,"x"
__ZL7io_inpbt:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	leave
	ret
	.section	.text._ZL8io_outpbth,"x"
__ZL8io_outpbth:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	andl	$255, %edx
	pushl	%edx
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
	leave
	ret
	.section	.text.uart_FlushTxQueue,"x"
.globl _uart_FlushTxQueue
_uart_FlushTxQueue:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	call	_io_DisableINT
	subl	$12, %esp
	movl	8(%ebp), %eax
	pushl	100(%eax)
	call	_ClearQueue
	call	_io_RestoreINT
	addl	$16, %esp
	leave
	ret
	.section	.text.uart_Read,"x"
.globl _uart_Read
_uart_Read:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %ebx
	testl	%ebx, %ebx
	jne	L43
	subl	$8, %esp
	pushl	$__ZZ9uart_ReadE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	movl	$65535, %edx
	addl	$16, %esp
	jmp	L44
L43:
	cmpl	$-1, 84(%ebx)
	je	L45
	call	_timer_nowtime
	movl	%eax, %esi
L48:
	movl	96(%ebx), %eax
	cmpl	$0, 8(%eax)
	jne	L46
	call	_timer_nowtime
	subl	%esi, %eax
	cmpl	84(%ebx), %eax
	jae	L47
	jmp	L48
L45:
	movl	96(%ebx), %eax
	cmpl	$0, 8(%eax)
	je	L49
L46:
	call	_io_DisableINT
	subl	$12, %esp
	pushl	96(%ebx)
	call	_PopQueue
	movl	%eax, %esi
	addl	$16, %esp
	movl	28(%ebx), %eax
	cmpl	$1, %eax
	je	L51
	cmpl	$2, %eax
	jne	L50
	jmp	L55
L51:
	movl	96(%ebx), %edx
	movl	12(%edx), %eax
	subl	$1024, %eax
	cmpl	%eax, 8(%edx)
	jae	L50
	cmpl	$0, 40(%ebx)
	jne	L50
	movl	128(%ebx), %eax
	andl	$65535, %eax
	call	__ZL7io_inpbt
	orl	$2, %eax
	movl	$0, %edx
	movb	%al, %dl
	movl	128(%ebx), %eax
	andl	$65535, %eax
	call	__ZL8io_outpbth
	movl	$1, 40(%ebx)
	jmp	L50
L55:
	movl	96(%ebx), %edx
	movl	12(%edx), %eax
	subl	$1024, %eax
	cmpl	%eax, 8(%edx)
	jae	L50
	cmpl	$1, 52(%ebx)
	je	L50
	movl	$1, 56(%ebx)
	movb	78(%ebx), %dl
	orl	$2, %edx
	movb	%dl, 78(%ebx)
	andl	$255, %edx
	movl	120(%ebx), %eax
	andl	$65535, %eax
	call	__ZL8io_outpbth
L50:
	call	_io_RestoreINT
	movl	%esi, %edx
	andl	$255, %edx
L44:
	movl	%edx, %eax
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
L47:
	movl	96(%ebx), %eax
	movl	$65535, %edx
	cmpl	$0, 8(%eax)
	je	L44
	jmp	L46
L49:
L54:
	jmp	L54
	.section	.text.uart_Receive,"x"
.globl _uart_Receive
_uart_Receive:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	12(%ebp), %edi
	movl	16(%ebp), %esi
	movl	$0, %ebx
	testl	%esi, %esi
	jle	L58
L61:
	subl	$12, %esp
	pushl	8(%ebp)
	call	_uart_Read
	addl	$16, %esp
	cmpl	$65535, %eax
	je	L58
	movb	%al, (%edi,%ebx)
	incl	%ebx
	cmpl	%ebx, %esi
	jg	L61
L58:
	movl	%ebx, %eax
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZL9uart_FreeP11serial_port,"x"
__ZL9uart_FreeP11serial_port:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	testl	%ebx, %ebx
	jne	L64
	subl	$8, %esp
	pushl	$__ZZL9uart_FreeP11serial_portE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L66
L64:
	subl	$12, %esp
	pushl	96(%ebx)
	call	_DestoryQueue
	addl	$4, %esp
	pushl	100(%ebx)
	call	_DestoryQueue
	movl	%ebx, (%esp)
	call	_ker_Mfree
	addl	$16, %esp
L66:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section .text
LC1:
	.ascii "%s: Init IO lib error!!\12\0"
	.p2align 2
LC2:
	.ascii "%s: COM%d is null. [Base address: 0x%04x, IRQ: %d]\12\0"
	.section	.text.CreateUART,"x"
.globl _CreateUART
_CreateUART:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	call	_io_Init
	testb	%al, %al
	jne	L68
	subl	$8, %esp
	pushl	$__ZZ10CreateUARTE12__FUNCTION__
	pushl	$LC1
	call	__ZL9err_printPcz
	movl	$0, %eax
	addl	$16, %esp
	jmp	L69
L68:
	subl	$12, %esp
	pushl	$136
	call	_ker_Malloc
	movl	%eax, %ebx
	addl	$16, %esp
	testl	%eax, %eax
	jne	L70
	call	_io_Close
	movl	$0, %eax
	jmp	L69
L70:
	movl	%esi, 4(%eax)
	subl	$12, %esp
	pushl	%esi
	call	_vx86_uart_GetBaseAddr
	movw	%ax, (%ebx)
	movl	%esi, (%esp)
	call	_vx86_uart_GetIRQ
	movl	%eax, 8(%ebx)
	movl	(%ebx), %edx
	addl	$16, %esp
	testw	%dx, %dx
	je	L71
	testl	%eax, %eax
	jne	L72
L71:
	subl	$12, %esp
	pushl	%eax
	movl	%edx, %eax
	andl	$65535, %eax
	pushl	%eax
	leal	1(%esi), %eax
	pushl	%eax
	pushl	$__ZZ10CreateUARTE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	addl	$20, %esp
	pushl	%ebx
	call	_ker_Mfree
	call	_io_Close
	movl	$0, %eax
	addl	$16, %esp
	jmp	L69
L72:
	movl	$115200, 12(%ebx)
	movl	$3, 16(%ebx)
	movl	$0, 20(%ebx)
	movl	$0, 24(%ebx)
	movl	$0, 28(%ebx)
	movl	$0, 32(%ebx)
	movl	$0, 36(%ebx)
	movl	$0, 40(%ebx)
	movl	$0, 44(%ebx)
	movl	$0, 48(%ebx)
	movl	$0, 52(%ebx)
	movl	$0, 56(%ebx)
	movl	$0, 60(%ebx)
	movb	$0, 76(%ebx)
	movb	$0, 64(%ebx)
	movb	$0, 77(%ebx)
	movb	$0, 65(%ebx)
	movb	$0, 78(%ebx)
	movb	$0, 66(%ebx)
	movb	$0, 79(%ebx)
	movb	$0, 67(%ebx)
	movb	$0, 80(%ebx)
	movb	$0, 68(%ebx)
	movl	$-1, 84(%ebx)
	movl	$-1, 72(%ebx)
	movb	$0, 81(%ebx)
	movl	$0, 88(%ebx)
	movl	$0, 92(%ebx)
	subl	$12, %esp
	pushl	$4096
	call	_CreateQueue
	movl	%eax, 96(%ebx)
	addl	$16, %esp
	testl	%eax, %eax
	je	L73
	subl	$12, %esp
	pushl	$256
	call	_CreateQueue
	movl	%eax, 100(%ebx)
	addl	$16, %esp
	testl	%eax, %eax
	je	L74
	movl	$0, 108(%ebx)
	movl	$0, 104(%ebx)
	movl	(%ebx), %eax
	movw	%ax, 112(%ebx)
	movw	%ax, 114(%ebx)
	movw	%ax, 116(%ebx)
	leal	1(%eax), %edx
	movw	%dx, 118(%ebx)
	movw	%dx, 120(%ebx)
	leal	2(%eax), %edx
	movw	%dx, 122(%ebx)
	movw	%dx, 124(%ebx)
	leal	3(%eax), %edx
	movw	%dx, 126(%ebx)
	leal	4(%eax), %edx
	movw	%dx, 128(%ebx)
	leal	5(%eax), %edx
	movw	%dx, 130(%ebx)
	leal	6(%eax), %edx
	movw	%dx, 132(%ebx)
	addl	$7, %eax
	movw	%ax, 134(%ebx)
	movl	%ebx, %eax
	jmp	L69
L74:
	subl	$12, %esp
	pushl	96(%ebx)
	call	_DestoryQueue
	addl	$16, %esp
L73:
	subl	$12, %esp
	pushl	%ebx
	call	_ker_Mfree
	call	_io_Close
	movl	$0, %eax
	addl	$16, %esp
L69:
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.uart_FlushWFIFO,"x"
.globl _uart_FlushWFIFO
_uart_FlushWFIFO:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	movl	100(%ebx), %eax
	cmpl	$0, 8(%eax)
	jne	L77
L80:
	subl	$8, %esp
	movw	130(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	andl	$96, %eax
	addl	$16, %esp
	cmpl	$96, %eax
	jne	L80
	jmp	L84
L77:
L83:
	jmp	L83
L84:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text.uart_GetMSR,"x"
.globl _uart_GetMSR
_uart_GetMSR:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L86
	subl	$8, %esp
	pushl	$__ZZ11uart_GetMSRE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L87
L86:
	subl	$8, %esp
	movl	132(%eax), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$16, %esp
L87:
	leave
	ret
	.section	.text.uart_GetLSR,"x"
.globl _uart_GetLSR
_uart_GetLSR:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L90
	subl	$8, %esp
	pushl	$__ZZ11uart_GetLSRE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L91
L90:
	subl	$8, %esp
	movw	130(%eax), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$16, %esp
L91:
	leave
	ret
	.section	.text.uart_SetFormat,"x"
.globl _uart_SetFormat
_uart_SetFormat:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movzbl	12(%ebp), %esi
	movl	8(%ebp), %ebx
	testl	%ebx, %ebx
	jne	L94
	subl	$8, %esp
	pushl	$__ZZ14uart_SetFormatE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L96
L94:
	call	_io_DisableINT
	subl	$4, %esp
	movl	%esi, %eax
	andl	$255, %eax
	pushl	%eax
	movw	126(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	call	_io_RestoreINT
	addl	$16, %esp
L96:
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text._ZL9uart_IsOKP11serial_port,"x"
__ZL9uart_IsOKP11serial_port:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %ebx
	testl	%ebx, %ebx
	jne	L98
	subl	$8, %esp
	pushl	$__ZZL9uart_IsOKP11serial_portE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L99
L98:
	subl	$8, %esp
	movw	122(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$16, %esp
	testb	$48, %al
	jne	L100
	subl	$8, %esp
	movw	134(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movl	%eax, %esi
	addl	$12, %esp
	pushl	$85
	movw	134(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$8, %esp
	movw	134(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$16, %esp
	cmpb	$85, %al
	jne	L100
	movw	134(%ebx), %ax
	andl	$65535, %eax
	movl	$170, %edx
	call	__ZL8io_outpbth
	movw	134(%ebx), %ax
	andl	$65535, %eax
	call	__ZL7io_inpbt
	cmpb	$-86, %al
	jne	L100
	movl	%esi, %edx
	andl	$255, %edx
	movw	134(%ebx), %ax
	andl	$65535, %eax
	call	__ZL8io_outpbth
	movb	$1, %al
	jmp	L99
L100:
	movb	$0, %al
L99:
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.uart_SetFlowControl,"x"
.globl _uart_SetFlowControl
_uart_SetFlowControl:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	12(%ebp), %eax
	movl	8(%ebp), %ebx
	testl	%ebx, %ebx
	jne	L103
	subl	$8, %esp
	pushl	$__ZZ19uart_SetFlowControlE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L107
L103:
	movl	%eax, 28(%ebx)
	cmpl	$1, %eax
	je	L105
	cmpl	$2, %eax
	jne	L107
	jmp	L108
L105:
	subl	$8, %esp
	movl	128(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	orl	$3, %eax
	andl	$255, %eax
	pushl	%eax
	movl	128(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	movl	$1, 40(%ebx)
	addl	$8, %esp
	movl	132(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	shrl	$4, %eax
	andl	$1, %eax
	movl	%eax, 44(%ebx)
	addl	$16, %esp
	jmp	L107
L108:
	movl	$1, 48(%ebx)
	movl	$1, 52(%ebx)
L107:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text.uart_IntDisable,"x"
.globl _uart_IntDisable
_uart_IntDisable:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	testl	%ebx, %ebx
	jne	L110
	subl	$8, %esp
	pushl	$__ZZ15uart_IntDisableE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L112
L110:
	cmpl	$0, 36(%ebx)
	je	L112
	call	_io_DisableINT
	subl	$8, %esp
	movl	128(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	andl	$247, %eax
	pushl	%eax
	movl	128(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	movl	$0, %eax
	movb	66(%ebx), %al
	pushl	%eax
	movl	120(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$8, %esp
	pushl	%ebx
	pushl	8(%ebx)
	call	_irq_UninstallISR
	call	_io_RestoreINT
	movl	$0, 36(%ebx)
	addl	$16, %esp
L112:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section .text
LC3:
	.ascii "%s: There is no IRQ%d.\12\0"
	.section	.text.uart_IntEnable,"x"
.globl _uart_IntEnable
_uart_IntEnable:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	testl	%ebx, %ebx
	jne	L114
	subl	$8, %esp
	pushl	$__ZZ14uart_IntEnableE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L117
L114:
	cmpl	$0, 36(%ebx)
	jne	L117
	movl	8(%ebx), %edx
	leal	-1(%edx), %eax
	cmpl	$14, %eax
	jbe	L116
	subl	$4, %esp
	pushl	%edx
	pushl	$__ZZ14uart_IntEnableE12__FUNCTION__
	pushl	$LC3
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L117
L116:
	subl	$8, %esp
	movl	$__ZL12UART_ISR_endv, %eax
	subl	$__ZL8UART_ISRiPv, %eax
	pushl	%eax
	pushl	$__ZL8UART_ISRiPv
	call	_dpmi_LockCode
	call	_io_DisableINT
	addl	$8, %esp
	pushl	$1
	pushl	8(%ebx)
	call	_irq_Setting
	addl	$12, %esp
	pushl	%ebx
	pushl	$__ZL8UART_ISRiPv
	pushl	8(%ebx)
	call	_irq_InstallISR
	movb	12(%ebp), %dl
	orb	78(%ebx), %dl
	movb	%dl, 78(%ebx)
	addl	$12, %esp
	movl	$0, %eax
	movb	%dl, %al
	pushl	%eax
	movl	120(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$8, %esp
	movl	128(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	orl	$8, %eax
	andl	$255, %eax
	pushl	%eax
	movl	128(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	call	_io_RestoreINT
	incl	36(%ebx)
	addl	$16, %esp
L117:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text.uart_Send,"x"
.globl _uart_Send
_uart_Send:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %ebx
	testl	%ebx, %ebx
	je	L119
	movl	$0, %edi
	cmpl	$0, 16(%ebp)
	jg	L132
	jmp	L121
L119:
	subl	$8, %esp
	pushl	$__ZZ9uart_SendE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	movl	$0, %edi
	addl	$16, %esp
	jmp	L121
L132:
	movl	16(%ebp), %eax
	decl	%eax
	movl	%eax, -16(%ebp)
L135:
	cmpl	$-1, 84(%ebx)
	je	L122
	call	_timer_nowtime
	movl	%eax, %esi
L125:
	movl	100(%ebx), %edx
	movl	8(%edx), %eax
	cmpl	12(%edx), %eax
	jb	L123
	call	_timer_nowtime
	subl	%esi, %eax
	cmpl	84(%ebx), %eax
	jae	L124
	jmp	L125
L138:
	movb	78(%ebx), %al
	testb	$2, %al
	jne	L121
	movb	%al, %dl
	orl	$2, %edx
	movb	%dl, 78(%ebx)
	andl	$255, %edx
	movl	120(%ebx), %eax
	andl	$65535, %eax
	call	__ZL8io_outpbth
	jmp	L121
L122:
	movl	100(%ebx), %edx
	movl	8(%edx), %eax
	cmpl	12(%edx), %eax
	jae	L126
L123:
	call	_io_DisableINT
	subl	$8, %esp
	movl	12(%ebp), %edx
	movl	$0, %eax
	movb	(%edx,%edi), %al
	pushl	%eax
	pushl	100(%ebx)
	call	_PushQueue
	movb	78(%ebx), %cl
	addl	$16, %esp
	testb	$2, %cl
	jne	L127
	cmpl	%edi, -16(%ebp)
	je	L128
	movl	100(%ebx), %edx
	movl	8(%edx), %eax
	cmpl	12(%edx), %eax
	jb	L127
L128:
	movl	28(%ebx), %eax
	cmpl	$1, %eax
	je	L130
	cmpl	$2, %eax
	je	L131
	testl	%eax, %eax
	jne	L127
	movb	%cl, %al
	orl	$2, %eax
	movb	%al, 78(%ebx)
	subl	$4, %esp
	andl	$255, %eax
	pushl	%eax
	movl	120(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
	jmp	L127
L130:
	cmpl	$1, 44(%ebx)
	jne	L127
	movb	%cl, %al
	orl	$2, %eax
	movb	%al, 78(%ebx)
	subl	$4, %esp
	andl	$255, %eax
	pushl	%eax
	movl	120(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
	jmp	L127
L131:
	cmpl	$0, 48(%ebx)
	je	L127
	movb	%cl, %al
	orl	$2, %eax
	movb	%al, 78(%ebx)
	subl	$4, %esp
	andl	$255, %eax
	pushl	%eax
	movl	120(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
L127:
	call	_io_RestoreINT
	incl	%edi
	cmpl	%edi, 16(%ebp)
	jg	L135
L121:
	movl	%edi, %eax
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
L124:
	movl	100(%ebx), %edx
	movl	8(%edx), %eax
	cmpl	12(%edx), %eax
	jb	L123
	jmp	L138
L126:
L137:
	jmp	L137
	.section	.text.uart_Write,"x"
.globl _uart_Write
_uart_Write:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	12(%ebp), %eax
	movb	%al, -4(%ebp)
	pushl	$1
	leal	-4(%ebp), %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_uart_Send
	addl	$16, %esp
	cmpl	$1, %eax
	sete	%al
	leave
	ret
	.section	.text._ZL8UART_ISRiPv,"x"
__ZL8UART_ISRiPv:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	12(%ebp), %ebx
	movl	$0, %esi
	movl	$L148, %edi
	jmp	L142
L176:
	movl	%edx, %eax
	andl	$7, %eax
	jmp	*(%edi,%eax,4)
	.p2align 2
	.section	.rodata._ZL8UART_ISRiPv,""
	.p2align 2
L148:
	.long	L144
	.long	L143
	.long	L145
	.long	L143
	.long	L178
	.long	L143
	.long	L147
	.long	L143
	.long	L143
	.long	L143
	.long	L143
	.long	L143
	.long	L178
	.section	.text._ZL8UART_ISRiPv,"x"
L178:
	subl	$8, %esp
	movw	114(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movb	%al, %dl
	addl	$16, %esp
	movl	28(%ebx), %eax
	cmpl	$1, %eax
	je	L151
	cmpl	$2, %eax
	je	L152
	testl	%eax, %eax
	jne	L149
	subl	$8, %esp
	movl	$0, %eax
	movb	%dl, %al
	pushl	%eax
	pushl	96(%ebx)
	call	_PushQueue
	addl	$16, %esp
	jmp	L149
L151:
	subl	$8, %esp
	movl	$0, %eax
	movb	%dl, %al
	pushl	%eax
	pushl	96(%ebx)
	call	_PushQueue
	movl	96(%ebx), %edx
	movl	12(%edx), %eax
	subl	$1024, %eax
	addl	$16, %esp
	cmpl	%eax, 8(%edx)
	jb	L149
	subl	$8, %esp
	movl	128(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	andl	$253, %eax
	pushl	%eax
	movl	128(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	movl	$0, 40(%ebx)
	addl	$16, %esp
	jmp	L149
L152:
	cmpb	$17, %dl
	jne	L153
	movl	$1, 48(%ebx)
	movb	78(%ebx), %al
	orl	$2, %eax
	movb	%al, 78(%ebx)
	subl	$4, %esp
	andl	$255, %eax
	pushl	%eax
	movl	120(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
	jmp	L149
L153:
	cmpb	$19, %dl
	jne	L154
	movl	$0, 48(%ebx)
	movb	78(%ebx), %al
	andl	$-3, %eax
	movb	%al, 78(%ebx)
	subl	$4, %esp
	andl	$255, %eax
	pushl	%eax
	movl	120(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
	jmp	L149
L154:
	subl	$8, %esp
	movl	$0, %eax
	movb	%dl, %al
	pushl	%eax
	pushl	96(%ebx)
	call	_PushQueue
	movl	96(%ebx), %edx
	movl	12(%edx), %eax
	subl	$1024, %eax
	addl	$16, %esp
	cmpl	%eax, 8(%edx)
	jb	L149
	cmpl	$0, 52(%ebx)
	je	L149
	movl	$1, 60(%ebx)
	movb	78(%ebx), %al
	orl	$2, %eax
	movb	%al, 78(%ebx)
	subl	$4, %esp
	andl	$255, %eax
	pushl	%eax
	movl	120(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
L149:
	subl	$8, %esp
	movw	130(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$16, %esp
	testb	$1, %al
	jne	L178
	jmp	L143
L145:
	movl	28(%ebx), %eax
	cmpl	$1, %eax
	je	L156
	cmpl	$2, %eax
	je	L157
	testl	%eax, %eax
	jne	L143
	movl	100(%ebx), %eax
	cmpl	$0, 8(%eax)
	je	L158
	movl	$0, %esi
	cmpl	$0, 92(%ebx)
	jg	L160
	jmp	L143
L158:
	movb	78(%ebx), %dl
	andl	$-3, %edx
	movb	%dl, 78(%ebx)
	andl	$255, %edx
	movl	120(%ebx), %eax
	andl	$65535, %eax
	call	__ZL8io_outpbth
	jmp	L143
L160:
	subl	$12, %esp
	pushl	%eax
	call	_PopQueue
	addl	$12, %esp
	andl	$255, %eax
	pushl	%eax
	movl	112(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	incl	%esi
	addl	$16, %esp
	cmpl	%esi, 92(%ebx)
	jle	L143
	movl	100(%ebx), %eax
	cmpl	$0, 8(%eax)
	jne	L160
	jmp	L143
L156:
	movl	100(%ebx), %eax
	cmpl	$0, 8(%eax)
	je	L161
	cmpl	$0, 92(%ebx)
	jle	L143
	movl	$0, %esi
	cmpl	$0, 44(%ebx)
	jne	L164
	jmp	L162
L161:
	movb	78(%ebx), %dl
	andl	$-3, %edx
	movb	%dl, 78(%ebx)
	andl	$255, %edx
	movl	120(%ebx), %eax
	andl	$65535, %eax
	call	__ZL8io_outpbth
	jmp	L143
L165:
	cmpl	$0, 44(%ebx)
	jne	L164
L162:
	movb	78(%ebx), %dl
	andl	$-3, %edx
	movb	%dl, 78(%ebx)
	andl	$255, %edx
	movl	120(%ebx), %eax
	andl	$65535, %eax
	call	__ZL8io_outpbth
	jmp	L143
L164:
	subl	$12, %esp
	pushl	%eax
	call	_PopQueue
	addl	$12, %esp
	andl	$255, %eax
	pushl	%eax
	movl	112(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	incl	%esi
	addl	$16, %esp
	cmpl	%esi, 92(%ebx)
	jle	L143
	movl	100(%ebx), %eax
	cmpl	$0, 8(%eax)
	jne	L165
	jmp	L143
L157:
	cmpl	$1, 60(%ebx)
	jne	L166
	movl	112(%ebx), %eax
	andl	$65535, %eax
	movl	$19, %edx
	call	__ZL8io_outpbth
	movl	$0, 60(%ebx)
	movl	$0, 52(%ebx)
	jmp	L167
L166:
	cmpl	$1, 56(%ebx)
	jne	L167
	movl	112(%ebx), %eax
	andl	$65535, %eax
	movl	$17, %edx
	call	__ZL8io_outpbth
	movl	$0, 56(%ebx)
	movl	$1, 52(%ebx)
L167:
	movl	100(%ebx), %eax
	cmpl	$0, 8(%eax)
	je	L168
	cmpl	$0, 48(%ebx)
	je	L168
	movl	$0, %esi
	cmpl	$0, 92(%ebx)
	jg	L170
	jmp	L143
L168:
	movb	78(%ebx), %al
	andl	$-3, %eax
	movb	%al, 78(%ebx)
	subl	$4, %esp
	andl	$255, %eax
	pushl	%eax
	movl	120(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
	jmp	L143
L171:
	cmpl	$0, 48(%ebx)
	jne	L170
	movb	78(%ebx), %dl
	andl	$-3, %edx
	movb	%dl, 78(%ebx)
	andl	$255, %edx
	movl	120(%ebx), %eax
	andl	$65535, %eax
	call	__ZL8io_outpbth
	jmp	L143
L170:
	subl	$12, %esp
	pushl	%eax
	call	_PopQueue
	addl	$12, %esp
	andl	$255, %eax
	pushl	%eax
	movl	112(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	incl	%esi
	addl	$16, %esp
	cmpl	%esi, 92(%ebx)
	jle	L143
	movl	100(%ebx), %eax
	cmpl	$0, 8(%eax)
	jne	L171
	jmp	L143
L147:
	subl	$8, %esp
	movw	130(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movb	%al, 82(%ebx)
	movl	104(%ebx), %eax
	addl	$16, %esp
	testl	%eax, %eax
	je	L143
	subl	$12, %esp
	pushl	%ebx
	call	*%eax
	addl	$16, %esp
	jmp	L143
L144:
	subl	$8, %esp
	movl	132(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movb	%al, 83(%ebx)
	addl	$16, %esp
	cmpl	$1, 28(%ebx)
	jne	L172
	movl	44(%ebx), %edx
	testl	%edx, %edx
	jne	L173
	testb	$16, %al
	je	L173
	movl	$1, 44(%ebx)
	movl	100(%ebx), %eax
	cmpl	$0, 8(%eax)
	je	L172
	movb	78(%ebx), %dl
	orl	$2, %edx
	movb	%dl, 78(%ebx)
	andl	$255, %edx
	movl	120(%ebx), %eax
	andl	$65535, %eax
	call	__ZL8io_outpbth
	jmp	L172
L173:
	cmpl	$1, %edx
	jne	L172
	testb	$16, %al
	jne	L172
	movl	$0, 44(%ebx)
	movb	78(%ebx), %dl
	andl	$-3, %edx
	movb	%dl, 78(%ebx)
	andl	$255, %edx
	movl	120(%ebx), %eax
	andl	$65535, %eax
	call	__ZL8io_outpbth
L172:
	movl	108(%ebx), %eax
	testl	%eax, %eax
	je	L143
	subl	$12, %esp
	pushl	%ebx
	call	*%eax
	addl	$16, %esp
L143:
	movl	96(%ebx), %eax
	movl	8(%eax), %edx
	leal	-1024(%edx), %eax
	cmpl	%eax, %edx
	jb	L174
	movl	$1, %esi
	jmp	L175
L174:
	movl	$1, %esi
L142:
	subl	$8, %esp
	movw	122(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movl	$0, %edx
	movb	%al, %dl
	addl	$16, %esp
	testb	$1, %dl
	je	L176
L175:
	movl	%esi, %eax
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text.uart_FlushRxQueue,"x"
.globl _uart_FlushRxQueue
_uart_FlushRxQueue:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	call	_io_DisableINT
	subl	$12, %esp
	pushl	96(%ebx)
	call	_ClearQueue
	addl	$16, %esp
	movl	28(%ebx), %eax
	cmpl	$1, %eax
	je	L188
	cmpl	$2, %eax
	jne	L187
	jmp	L191
L188:
	cmpl	$0, 40(%ebx)
	jne	L187
	subl	$8, %esp
	movl	128(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	orl	$2, %eax
	andl	$255, %eax
	pushl	%eax
	movl	128(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	movl	$1, 40(%ebx)
	addl	$16, %esp
	jmp	L187
L191:
	cmpl	$1, 52(%ebx)
	je	L187
	movl	$1, 56(%ebx)
	movb	78(%ebx), %al
	orl	$2, %eax
	movb	%al, 78(%ebx)
	subl	$4, %esp
	andl	$255, %eax
	pushl	%eax
	movl	120(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
L187:
	call	_io_RestoreINT
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text.uart_SetRFIFOTrig,"x"
.globl _uart_SetRFIFOTrig
_uart_SetRFIFOTrig:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %edi
	testl	%edi, %edi
	jne	L193
	subl	$8, %esp
	pushl	$__ZZ17uart_SetRFIFOTrigE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L195
L193:
	movb	81(%edi), %bl
	andl	$63, %ebx
	orl	12(%ebp), %ebx
	movb	%bl, 81(%edi)
	movl	124(%edi), %esi
	call	_io_DisableINT
	subl	$8, %esp
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movb	%al, -13(%ebp)
	addl	$12, %esp
	pushl	$128
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	andl	$255, %ebx
	pushl	%ebx
	andl	$65535, %esi
	pushl	%esi
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	movl	$0, %eax
	movb	-13(%ebp), %al
	pushl	%eax
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	call	_io_RestoreINT
	addl	$16, %esp
L195:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text.uart_ClearWFIFO,"x"
.globl _uart_ClearWFIFO
_uart_ClearWFIFO:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %edi
	testl	%edi, %edi
	jne	L197
	subl	$8, %esp
	pushl	$__ZZ15uart_ClearWFIFOE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L199
L197:
	movb	81(%edi), %bl
	orl	$4, %ebx
	movb	%bl, 81(%edi)
	movl	124(%edi), %esi
	call	_io_DisableINT
	subl	$8, %esp
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movb	%al, -13(%ebp)
	addl	$12, %esp
	pushl	$128
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	andl	$255, %ebx
	pushl	%ebx
	andl	$65535, %esi
	pushl	%esi
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	movl	$0, %eax
	movb	-13(%ebp), %al
	pushl	%eax
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	call	_io_RestoreINT
	addl	$16, %esp
L199:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text.uart_ClearRFIFO,"x"
.globl _uart_ClearRFIFO
_uart_ClearRFIFO:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %edi
	testl	%edi, %edi
	jne	L201
	subl	$8, %esp
	pushl	$__ZZ15uart_ClearRFIFOE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L203
L201:
	movb	81(%edi), %bl
	orl	$2, %ebx
	movb	%bl, 81(%edi)
	movl	124(%edi), %esi
	call	_io_DisableINT
	subl	$8, %esp
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movb	%al, -13(%ebp)
	addl	$12, %esp
	pushl	$128
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	andl	$255, %ebx
	pushl	%ebx
	andl	$65535, %esi
	pushl	%esi
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	movl	$0, %eax
	movb	-13(%ebp), %al
	pushl	%eax
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	call	_io_RestoreINT
	addl	$16, %esp
L203:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section .text
LC4:
	.ascii "Close IO lib error!!\12\0"
	.section	.text.uart_Close,"x"
.globl _uart_Close
_uart_Close:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %esi
	movl	%esi, %edi
	testl	%esi, %esi
	jne	L205
	subl	$8, %esp
	pushl	$__ZZ10uart_CloseE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L208
L205:
	cmpl	$0, 32(%esi)
	je	L207
	subl	$12, %esp
	pushl	%esi
	call	_uart_IntDisable
	call	_io_DisableINT
	addl	$12, %esp
	movl	$0, %eax
	movb	68(%esi), %al
	pushl	%eax
	movl	128(%esi), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	call	_io_RestoreINT
	call	_irq_Close
	movl	$0, %eax
	movb	64(%esi), %al
	movl	%eax, -24(%ebp)
	movl	116(%esi), %ebx
	call	_io_DisableINT
	addl	$8, %esp
	movw	126(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movb	%al, -14(%ebp)
	addl	$12, %esp
	pushl	$128
	movw	126(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	pushl	-24(%ebp)
	andl	$65535, %ebx
	pushl	%ebx
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	movl	$0, %eax
	movb	-14(%ebp), %al
	pushl	%eax
	movw	126(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	call	_io_RestoreINT
	movl	$0, %eax
	movb	65(%esi), %al
	movl	%eax, -20(%ebp)
	movw	118(%esi), %bx
	call	_io_DisableINT
	addl	$8, %esp
	movw	126(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movb	%al, -13(%ebp)
	addl	$12, %esp
	pushl	$128
	movw	126(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	pushl	-20(%ebp)
	andl	$65535, %ebx
	pushl	%ebx
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	movl	$0, %eax
	movb	-13(%ebp), %al
	pushl	%eax
	movw	126(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	call	_io_RestoreINT
	addl	$8, %esp
	movl	$0, %eax
	movb	67(%esi), %al
	pushl	%eax
	pushl	%esi
	call	_uart_SetFormat
	addl	$8, %esp
	pushl	72(%esi)
	pushl	%esi
	call	_uart_SetTimeOut
	addl	$4, %esp
	pushl	4(%esi)
	call	_vx86_uart_Close
	call	_io_Close
	addl	$16, %esp
	testb	%al, %al
	jne	L207
	subl	$12, %esp
	pushl	$LC4
	call	__ZL9err_printPcz
	addl	$16, %esp
L207:
	subl	$12, %esp
	pushl	96(%edi)
	call	_DestoryQueue
	addl	$4, %esp
	pushl	100(%edi)
	call	_DestoryQueue
	movl	%edi, (%esp)
	call	_ker_Mfree
	addl	$16, %esp
L208:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text.uart_SetHWFIFO,"x"
.globl _uart_SetHWFIFO
_uart_SetHWFIFO:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	12(%ebp), %eax
	movl	8(%ebp), %edi
	testl	%edi, %edi
	jne	L210
	subl	$8, %esp
	pushl	$__ZZ14uart_SetHWFIFOE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L216
L210:
	cmpl	$16, %eax
	je	L213
	cmpl	$16, %eax
	jg	L215
	cmpl	$1, %eax
	jne	L216
	jmp	L217
L215:
	cmpl	$32, %eax
	je	L214
	cmpl	$128, %eax
	jne	L216
	jmp	L214
L217:
	movb	81(%edi), %bl
	andl	$-2, %ebx
	movb	%bl, 81(%edi)
	movl	124(%edi), %esi
	call	_io_DisableINT
	subl	$8, %esp
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movb	%al, -15(%ebp)
	addl	$12, %esp
	pushl	$128
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	andl	$255, %ebx
	pushl	%ebx
	andl	$65535, %esi
	pushl	%esi
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	movl	$0, %eax
	movb	-15(%ebp), %al
	pushl	%eax
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	call	_io_RestoreINT
	movl	$1, 88(%edi)
	movl	$1, 92(%edi)
	addl	$16, %esp
	jmp	L216
L213:
	subl	$12, %esp
	pushl	4(%edi)
	call	_vx86_uart_SetFIFO16
	movb	81(%edi), %bl
	orl	$1, %ebx
	movb	%bl, 81(%edi)
	movl	124(%edi), %esi
	call	_io_DisableINT
	addl	$8, %esp
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movb	%al, -14(%ebp)
	addl	$12, %esp
	pushl	$128
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	andl	$255, %ebx
	pushl	%ebx
	andl	$65535, %esi
	pushl	%esi
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	movl	$0, %eax
	movb	-14(%ebp), %al
	pushl	%eax
	movw	126(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	call	_io_RestoreINT
	movl	$16, 88(%edi)
	movl	$16, 92(%edi)
	addl	$16, %esp
	jmp	L216
L214:
	subl	$12, %esp
	pushl	4(%edi)
	call	_vx86_uart_SetFIFO32
	movb	81(%edi), %bl
	orl	$1, %ebx
	movb	%bl, 81(%edi)
	movl	124(%edi), %esi
	call	_io_DisableINT
	addl	$8, %esp
	movw	