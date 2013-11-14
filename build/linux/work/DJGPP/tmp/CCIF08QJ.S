	.file	"Blink.cpp"
	.section .text
	.p2align 1
.globl _loop
_loop:
LFB87:
	pushl	%ebp
LCFI0:
	movl	%esp, %ebp
LCFI1:
	subl	$8, %esp
LCFI2:
	movl	_led, %eax
	andl	$255, %eax
	subl	$8, %esp
	pushl	$1
	pushl	%eax
LCFI3:
	call	_digitalWrite
	addl	$16, %esp
	subl	$12, %esp
	pushl	$1000
	call	_delay
	addl	$16, %esp
	movl	_led, %eax
	andl	$255, %eax
	subl	$8, %esp
	pushl	$0
	pushl	%eax
	call	_digitalWrite
	addl	$16, %esp
	subl	$12, %esp
	pushl	$1000
	call	_delay
	addl	$16, %esp
	leave
	ret
LFE87:
	.p2align 1
.globl _setup
_setup:
LFB86:
	pushl	%ebp
LCFI4:
	movl	%esp, %ebp
LCFI5:
	subl	$8, %esp
LCFI6:
	movl	_led, %eax
	andl	$255, %eax
	subl	$8, %esp
	pushl	$1
	pushl	%eax
LCFI7:
	call	_pinMode
	addl	$16, %esp
	leave
	ret
LFE86:
.globl _led
	.section .data
	.p2align 2
_led:
	.long	13
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
LSFDE1:
	.long	LEFDE1-LASFDE1
LASFDE1:
	.long	LASFDE1-Lframe1
	.long	LFB87
	.long	LFE87-LFB87
	.uleb128 0x0
	.byte	0x4
	.long	LCFI0-LFB87
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI1-LCFI0
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI3-LCFI1
	.byte	0x2e
	.uleb128 0x10
	.p2align 2
LEFDE1:
LSFDE3:
	.long	LEFDE3-LASFDE3
LASFDE3:
	.long	LASFDE3-Lframe1
	.long	LFB86
	.long	LFE86-LFB86
	.uleb128 0x0
	.byte	0x4
	.long	LCFI4-LFB86
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI5-LCFI4
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI7-LCFI5
	.byte	0x2e
	.uleb128 0x10
	.p2align 2
LEFDE3:
	.ident	"GCC: (GNU) 4.2.3"
