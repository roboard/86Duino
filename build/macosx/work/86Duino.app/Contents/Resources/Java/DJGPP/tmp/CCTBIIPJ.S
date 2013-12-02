	.file	"Blink.cpp"
	.section	.ctors,"w"
	.p2align 2
	.long	__GLOBAL__I_led
	.section .text
	.p2align 1
__Z41__static_initialization_and_destruction_0ii:
LFB88:
	pushl	%ebp
LCFI0:
	movl	%esp, %ebp
LCFI1:
	cmpl	$1, 8(%ebp)
	jne	L5
	cmpl	$65535, 12(%ebp)
	jne	L5
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
	movl	_MD_list, %eax
	movl	%eax, _arduint_to_mc_md+68
	movl	_MD_list+8, %eax
	movl	%eax, _arduint_to_mc_md+76
	movl	_MD_list+4, %eax
	movl	%eax, _arduint_to_mc_md+80
	movl	_MD_list, %eax
	movl	%eax, _arduint_to_mc_md+92
	movl	_MD_list+8, %eax
	movl	%eax, _arduint_to_mc_md+96
	movl	_MD_list+4, %eax
	movl	%eax, _arduint_to_mc_md+100
	movl	_MD_list, %eax
	movl	%eax, _arduint_to_mc_md+108
L5:
	popl	%ebp
	ret
LFE88:
	.p2align 1
__GLOBAL__I_led:
LFB89:
	pushl	%ebp
LCFI2:
	movl	%esp, %ebp
LCFI3:
	pushl	$65535
	pushl	$1
LCFI4:
	call	__Z41__static_initialization_and_destruction_0ii
	addl	$8, %esp
	leave
	ret
LFE89:
	.p2align 1
.globl _loop
_loop:
LFB87:
	pushl	%ebp
LCFI5:
	movl	%esp, %ebp
LCFI6:
	subl	$8, %esp
LCFI7:
	movl	_led, %eax
	andl	$255, %eax
	subl	$8, %esp
	pushl	$1
	pushl	%eax
LCFI8:
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
LCFI9:
	movl	%esp, %ebp
LCFI10:
	subl	$8, %esp
LCFI11:
	movl	_led, %eax
	andl	$255, %eax
	subl	$8, %esp
	pushl	$1
	pushl	%eax
LCFI12:
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
	.p2align 4
_MC_list:
	.long	0
	.long	1
	.long	2
	.long	3
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
	.space 4
	.long	-1
	.space 8
	.long	-1
	.long	-1
	.space 12
	.long	-1
	.space 4
	.p2align 2
_MD_list:
	.long	0
	.long	1
	.long	2
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
LSFDE5:
	.long	LEFDE5-LASFDE5
LASFDE5:
	.long	LASFDE5-Lframe1
	.long	LFB87
	.long	LFE87-LFB87
	.uleb128 0x0
	.byte	0x4
	.long	LCFI5-LFB87
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI6-LCFI5
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI8-LCFI6
	.byte	0x2e
	.uleb128 0x10
	.p2align 2
LEFDE5:
LSFDE7:
	.long	LEFDE7-LASFDE7
LASFDE7:
	.long	LASFDE7-Lframe1
	.long	LFB86
	.long	LFE86-LFB86
	.uleb128 0x0
	.byte	0x4
	.long	LCFI9-LFB86
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI10-LCFI9
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI12-LCFI10
	.byte	0x2e
	.uleb128 0x10
	.p2align 2
LEFDE7:
	.ident	"GCC: (GNU) 4.2.3"
