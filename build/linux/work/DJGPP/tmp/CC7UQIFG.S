	.file	"CDC.cpp"
	.section	.ctors,"w"
	.p2align 2
	.long	__GLOBAL__I__ZN7Serial_5beginEj
	.section	.gnu.linkonce.t._ZN5PrintC2Ev,"x"
	.p2align 1
	.weak	__ZN5PrintC2Ev
__ZN5PrintC2Ev:
LFB74:
	pushl	%ebp
LCFI0:
	movl	%esp, %ebp
LCFI1:
	movl	$__ZTV5Print+8, %eax
	movl	8(%ebp), %edx
	movl	%eax, (%edx)
	movl	8(%ebp), %eax
	movl	$0, 4(%eax)
	popl	%ebp
	ret
LFE74:
	.section	.gnu.linkonce.t._ZN6StreamC2Ev,"x"
	.p2align 1
	.weak	__ZN6StreamC2Ev
__ZN6StreamC2Ev:
LFB80:
	pushl	%ebp
LCFI2:
	movl	%esp, %ebp
LCFI3:
	subl	$8, %esp
LCFI4:
	movl	8(%ebp), %eax
	subl	$12, %esp
	pushl	%eax
LCFI5:
	call	__ZN5PrintC2Ev
	addl	$16, %esp
	movl	$__ZTV6Stream+8, %edx
	movl	8(%ebp), %eax
	movl	%edx, (%eax)
	movl	8(%ebp), %eax
	movl	$1000, 8(%eax)
	leave
	ret
LFE80:
	.section .text
	.p2align 1
.globl __ZN7Serial_6acceptEv
__ZN7Serial_6acceptEv:
LFB88:
	pushl	%ebp
LCFI6:
	movl	%esp, %ebp
LCFI7:
	popl	%ebp
	ret
LFE88:
	.p2align 1
.globl __ZN7Serial_4peekEv
__ZN7Serial_4peekEv:
LFB90:
	pushl	%ebp
LCFI8:
	movl	%esp, %ebp
LCFI9:
	popl	%ebp
	ret
LFE90:
	.section	.gnu.linkonce.t._ZN7Serial_C1Ev,"x"
	.p2align 1
	.weak	__ZN7Serial_C1Ev
__ZN7Serial_C1Ev:
LFB98:
	pushl	%ebp
LCFI10:
	movl	%esp, %ebp
LCFI11:
	subl	$8, %esp
LCFI12:
	movl	8(%ebp), %eax
	subl	$12, %esp
	pushl	%eax
LCFI13:
	call	__ZN6StreamC2Ev
	addl	$16, %esp
	movl	$__ZTV7Serial_+8, %edx
	movl	8(%ebp), %eax
	movl	%edx, (%eax)
	leave
	ret
LFE98:
	.section .text
	.p2align 1
__Z41__static_initialization_and_destruction_0ii:
LFB95:
	pushl	%ebp
LCFI14:
	movl	%esp, %ebp
LCFI15:
	subl	$8, %esp
LCFI16:
	cmpl	$1, 8(%ebp)
	jne	L15
	cmpl	$65535, 12(%ebp)
	jne	L15
	subl	$12, %esp
	pushl	$_Serial
LCFI17:
	call	__ZN7Serial_C1Ev
	addl	$16, %esp
L15:
	leave
	ret
LFE95:
	.p2align 1
__GLOBAL__I__ZN7Serial_5beginEj:
LFB99:
	pushl	%ebp
LCFI18:
	movl	%esp, %ebp
LCFI19:
	subl	$8, %esp
LCFI20:
	subl	$8, %esp
	pushl	$65535
	pushl	$1
LCFI21:
	call	__Z41__static_initialization_and_destruction_0ii
	addl	$16, %esp
	leave
	ret
LFE99:
	.p2align 1
.globl __ZN7Serial_cvbEv
__ZN7Serial_cvbEv:
LFB94:
	pushl	%ebp
LCFI22:
	movl	%esp, %ebp
LCFI23:
	subl	$24, %esp
LCFI24:
	movb	$0, -1(%ebp)
	movl	_USBDEV, %eax
	subl	$12, %esp
	pushl	%eax
LCFI25:
	call	_usb_GetControlLineState
	addl	$16, %esp
	testw	%ax, %ax
	setne	%al
	testb	%al, %al
	je	L19
	movb	$1, -1(%ebp)
L19:
	subl	$12, %esp
	pushl	$10
	call	_delay_ms
	addl	$16, %esp
	movl	$0, %eax
	movb	-1(%ebp), %al
	leave
	ret
LFE94:
	.p2align 1
.globl __ZN7Serial_5writeEh
__ZN7Serial_5writeEh:
LFB93:
	pushl	%ebp
LCFI26:
	movl	%esp, %ebp
LCFI27:
	subl	$8, %esp
LCFI28:
	movl	12(%ebp), %eax
	movb	%al, -4(%ebp)
	movl	_USBDEV, %eax
	subl	$12, %esp
	pushl	%eax
LCFI29:
	call	_usb_GetControlLineState
	addl	$16, %esp
	testw	%ax, %ax
	setne	%al
	testb	%al, %al
	je	L23
	movl	$0, %eax
	movb	-4(%ebp), %al
	movl	_USBDEV, %edx
	subl	$8, %esp
	pushl	%eax
	pushl	%edx
	call	_usb_Write
	addl	$16, %esp
L23:
	movl	$0, %eax
	leave
	ret
LFE93:
	.p2align 1
.globl __ZN7Serial_5flushEv
__ZN7Serial_5flushEv:
LFB92:
	pushl	%ebp
LCFI30:
	movl	%esp, %ebp
LCFI31:
	subl	$8, %esp
LCFI32:
	movl	_USBDEV, %eax
	subl	$12, %esp
	pushl	%eax
LCFI33:
	call	_usb_FlushTxQueue
	addl	$16, %esp
	leave
	ret
LFE92:
	.p2align 1
.globl __ZN7Serial_4readEv
__ZN7Serial_4readEv:
LFB91:
	pushl	%ebp
LCFI34:
	movl	%esp, %ebp
LCFI35:
	subl	$8, %esp
LCFI36:
	movl	_USBDEV, %eax
	subl	$12, %esp
	pushl	%eax
LCFI37:
	call	_usb_RxQueueEmpty
	addl	$16, %esp
	testb	%al, %al
	je	L29
	movl	$-1, -4(%ebp)
	jmp	L31
L29:
	movl	_USBDEV, %eax
	subl	$12, %esp
	pushl	%eax
	call	_usb_Read
	addl	$16, %esp
	movl	%eax, -4(%ebp)
L31:
	movl	-4(%ebp), %eax
	leave
	ret
LFE91:
	.p2align 1
.globl __ZN7Serial_9availableEv
__ZN7Serial_9availableEv:
LFB89:
	pushl	%ebp
LCFI38:
	movl	%esp, %ebp
LCFI39:
	subl	$8, %esp
LCFI40:
	movl	_USBDEV, %eax
	subl	$12, %esp
	pushl	%eax
LCFI41:
	call	_usb_QueryRxQueue
	addl	$16, %esp
	leave
	ret
LFE89:
	.p2align 1
.globl __ZN7Serial_3endEv
__ZN7Serial_3endEv:
LFB87:
	pushl	%ebp
LCFI42:
	movl	%esp, %ebp
LCFI43:
	subl	$8, %esp
LCFI44:
	movl	_USBDEV, %eax
	testl	%eax, %eax
	je	L38
	movl	_USBDEV, %eax
	subl	$12, %esp
	pushl	%eax
LCFI45:
	call	_usb_Close
	addl	$16, %esp
L38:
	leave
	ret
LFE87:
	.p2align 1
.globl __ZN7Serial_5beginEj
__ZN7Serial_5beginEj:
LFB86:
	pushl	%ebp
LCFI46:
	movl	%esp, %ebp
LCFI47:
	subl	$8, %esp
LCFI48:
L40:
	movl	_USBDEV, %eax
	subl	$12, %esp
	pushl	%eax
LCFI49:
	call	_usb_Ready
	addl	$16, %esp
	xorl	$1, %eax
	testb	%al, %al
	jne	L40
	leave
	ret
LFE86:
.globl _Serial
	.section	.bss
	.p2align 4
_Serial:
	.space 20
	.weak	__ZTV7Serial_
	.section	.gnu.linkonce.r._ZTV7Serial_,""
	.p2align 4
__ZTV7Serial_:
	.long	0
	.long	__ZTI7Serial_
	.long	__ZN7Serial_5writeEh
	.long	__ZN5Print5writeEPKhm
	.long	__ZN7Serial_9availableEv
	.long	__ZN7Serial_4readEv
	.long	__ZN7Serial_4peekEv
	.long	__ZN7Serial_5flushEv
	.long	__ZN7Serial_6acceptEv
	.weak	__ZTS7Serial_
	.section	.gnu.linkonce.r._ZTS7Serial_,""
__ZTS7Serial_:
	.ascii "7Serial_\0"
	.weak	__ZTI7Serial_
	.section	.gnu.linkonce.r._ZTI7Serial_,""
	.p2align 2
__ZTI7Serial_:
	.long	__ZTVN10__cxxabiv120__si_class_type_infoE+8
	.long	__ZTS7Serial_
	.long	__ZTI6Stream
	.weak	__ZTI6Stream
	.section	.gnu.linkonce.r._ZTI6Stream,""
	.p2align 2
__ZTI6Stream:
	.long	__ZTVN10__cxxabiv120__si_class_type_infoE+8
	.long	__ZTS6Stream
	.long	__ZTI5Print
	.weak	__ZTS6Stream
	.section	.gnu.linkonce.r._ZTS6Stream,""
__ZTS6Stream:
	.ascii "6Stream\0"
	.weak	__ZTV6Stream
	.section	.gnu.linkonce.r._ZTV6Stream,""
	.p2align 4
__ZTV6Stream:
	.long	0
	.long	__ZTI6Stream
	.long	___cxa_pure_virtual
	.long	__ZN5Print5writeEPKhm
	.long	___cxa_pure_virtual
	.long	___cxa_pure_virtual
	.long	___cxa_pure_virtual
	.long	___cxa_pure_virtual
	.section	.eh_frame,"w"
Lframe1:
	.long	LECIE1-LSCIE1
LSCIE1:
	.long	0x0
	.byte	0x1
	.ascii "zP\0"
	.uleb128 0x1
	.sleb128 -4
	.byte	0x8
	.uleb128 0x5
	.byte	0x0
	.long	___gxx_personality_v0
	.byte	0xc
	.uleb128 0x4
	.uleb128 0x4
	.byte	0x88
	.uleb128 0x1
	.p2align 2
LECIE1:
LSFDE15:
	.long	LEFDE15-LASFDE15
LASFDE15:
	.long	LASFDE15-Lframe1
	.long	LFB94
	.long	LFE94-LFB94
	.uleb128 0x0
	.byte	0x4
	.long	LCFI22-LFB94
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI23-LCFI22
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI25-LCFI23
	.byte	0x2e
	.uleb128 0x10
	.p2align 2
LEFDE15:
LSFDE17:
	.long	LEFDE17-LASFDE17
LASFDE17:
	.long	LASFDE17-Lframe1
	.long	LFB93
	.long	LFE93-LFB93
	.uleb128 0x0
	.byte	0x4
	.long	LCFI26-LFB93
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI27-LCFI26
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI29-LCFI27
	.byte	0x2e
	.uleb128 0x10
	.p2align 2
LEFDE17:
LSFDE19:
	.long	LEFDE19-LASFDE19
LASFDE19:
	.long	LASFDE19-Lframe1
	.long	LFB92
	.long	LFE92-LFB92
	.uleb128 0x0
	.byte	0x4
	.long	LCFI30-LFB92
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI31-LCFI30
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI33-LCFI31
	.byte	0x2e
	.uleb128 0x10
	.p2align 2
LEFDE19:
LSFDE21:
	.long	LEFDE21-LASFDE21
LASFDE21:
	.long	LASFDE21-Lframe1
	.long	LFB91
	.long	LFE91-LFB91
	.uleb128 0x0
	.byte	0x4
	.long	LCFI34-LFB91
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI35-LCFI34
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI37-LCFI35
	.byte	0x2e
	.uleb128 0x10
	.p2align 2
LEFDE21:
LSFDE23:
	.long	LEFDE23-LASFDE23
LASFDE23:
	.long	LASFDE23-Lframe1
	.long	LFB89
	.long	LFE89-LFB89
	.uleb128 0x0
	.byte	0x4
	.long	LCFI38-LFB89
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI39-LCFI38
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI41-LCFI39
	.byte	0x2e
	.uleb128 0x10
	.p2align 2
LEFDE23:
LSFDE25:
	.long	LEFDE25-LASFDE25
LASFDE25:
	.long	LASFDE25-Lframe1
	.long	LFB87
	.long	LFE87-LFB87
	.uleb128 0x0
	.byte	0x4
	.long	LCFI42-LFB87
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI43-LCFI42
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI45-LCFI43
	.byte	0x2e
	.uleb128 0x10
	.p2align 2
LEFDE25:
LSFDE27:
	.long	LEFDE27-LASFDE27
LASFDE27:
	.long	LASFDE27-Lframe1
	.long	LFB86
	.long	LFE86-LFB86
	.uleb128 0x0
	.byte	0x4
	.long	LCFI46-LFB86
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI47-LCFI46
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI49-LCFI47
	.byte	0x2e
	.uleb128 0x10
	.p2align 2
LEFDE27:
	.ident	"GCC: (GNU) 4.2.3"
