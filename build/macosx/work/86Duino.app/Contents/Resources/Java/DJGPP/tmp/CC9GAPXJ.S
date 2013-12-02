	.file	"mcm.cpp"
	.section	.gnu.linkonce.t._farpokeb,"x"
	.weak	__farpokeb
__farpokeb:
LFB41:
	pushl	%ebp
LCFI0:
	movl	%esp, %ebp
LCFI1:
	subl	$8, %esp
LCFI2:
	movl	8(%ebp), %eax
	movl	16(%ebp), %edx
	movw	%ax, -4(%ebp)
	movb	%dl, -8(%ebp)
	movb	-8(%ebp), %dl
	movl	12(%ebp), %eax
/APP
# 89 "c:/include/sys/farptr.h" 1
	movw -4(%ebp),%fs
	.byte 0x64 
	movb %dl,(%eax)
# 0 "" 2
/NO_APP
	leave
	ret
LFE41:
	.section	.gnu.linkonce.t._farpokel,"x"
	.weak	__farpokel
__farpokel:
LFB43:
	pushl	%ebp
LCFI3:
	movl	%esp, %ebp
LCFI4:
	subl	$4, %esp
LCFI5:
	movl	8(%ebp), %eax
	movw	%ax, -4(%ebp)
	movl	16(%ebp), %edx
	movl	12(%ebp), %eax
/APP
# 113 "c:/include/sys/farptr.h" 1
	movw -4(%ebp),%fs 
	.byte 0x64 
	movl %edx,(%eax)
# 0 "" 2
/NO_APP
	leave
	ret
LFE43:
	.section	.gnu.linkonce.t._farpeekb,"x"
	.weak	__farpeekb
__farpeekb:
LFB44:
	pushl	%ebp
LCFI6:
	movl	%esp, %ebp
LCFI7:
	subl	$20, %esp
LCFI8:
	movl	8(%ebp), %eax
	movw	%ax, -20(%ebp)
	movl	12(%ebp), %eax
/APP
# 125 "c:/include/sys/farptr.h" 1
	movw -20(%ebp),%fs 
	.byte 0x64 
	movb (%eax),%al
# 0 "" 2
/NO_APP
	movb	%al, -1(%ebp)
	movb	-1(%ebp), %al
	leave
	ret
LFE44:
	.section	.gnu.linkonce.t._farpeekl,"x"
	.weak	__farpeekl
__farpeekl:
LFB46:
	pushl	%ebp
LCFI9:
	movl	%esp, %ebp
LCFI10:
	subl	$20, %esp
LCFI11:
	movl	8(%ebp), %eax
	movw	%ax, -20(%ebp)
	movl	12(%ebp), %eax
/APP
# 151 "c:/include/sys/farptr.h" 1
	movw -20(%ebp),%fs
	.byte 0x64
	movl (%eax),%eax
# 0 "" 2
/NO_APP
	movl	%eax, -4(%ebp)
	movl	-4(%ebp), %eax
	leave
	ret
LFE46:
	.section .text
.globl _set_MMIO
_set_MMIO:
LFB55:
	pushl	%ebp
LCFI12:
	movl	%esp, %ebp
LCFI13:
	movb	$1, _MC_useMMIO
	popl	%ebp
	ret
LFE55:
.globl _set_PortIO
_set_PortIO:
LFB56:
	pushl	%ebp
LCFI14:
	movl	%esp, %ebp
LCFI15:
	movb	$0, _MC_useMMIO
	popl	%ebp
	ret
LFE56:
__ZL11mmio_outpdwmm:
LFB59:
	pushl	%ebp
LCFI16:
	movl	%esp, %ebp
LCFI17:
	subl	$8, %esp
LCFI18:
	movl	__ZL13mmio_selector, %eax
	andl	$65535, %eax
	subl	$4, %esp
	pushl	12(%ebp)
	pushl	8(%ebp)
	pushl	%eax
LCFI19:
	call	__farpokel
	addl	$16, %esp
	leave
	ret
LFE59:
__ZL10mmio_inpdwm:
LFB60:
	pushl	%ebp
LCFI20:
	movl	%esp, %ebp
LCFI21:
	subl	$8, %esp
LCFI22:
	movl	__ZL13mmio_selector, %eax
	andl	$65535, %eax
	subl	$8, %esp
	pushl	8(%ebp)
	pushl	%eax
LCFI23:
	call	__farpeekl
	addl	$16, %esp
	leave
	ret
LFE60:
__ZL10mmio_outpbmh:
LFB61:
	pushl	%ebp
LCFI24:
	movl	%esp, %ebp
LCFI25:
	subl	$8, %esp
LCFI26:
	movl	12(%ebp), %eax
	movb	%al, -4(%ebp)
	movl	$0, %edx
	movb	-4(%ebp), %dl
	movl	__ZL13mmio_selector, %eax
	andl	$65535, %eax
	subl	$4, %esp
	pushl	%edx
	pushl	8(%ebp)
	pushl	%eax
LCFI27:
	call	__farpokeb
	addl	$16, %esp
	leave
	ret
LFE61:
__ZL9mmio_inpbm:
LFB62:
	pushl	%ebp
LCFI28:
	movl	%esp, %ebp
LCFI29:
	subl	$8, %esp
LCFI30:
	movl	__ZL13mmio_selector, %eax
	andl	$65535, %eax
	subl	$8, %esp
	pushl	8(%ebp)
	pushl	%eax
LCFI31:
	call	__farpeekb
	addl	$16, %esp
	leave
	ret
LFE62:
__ZL9io_outpdwtm:
LFB8:
	pushl	%ebp
LCFI32:
	movl	%esp, %ebp
LCFI33:
	subl	$8, %esp
LCFI34:
	movl	%edx, -8(%ebp)
	movw	%ax, -4(%ebp)
	movl	-4(%ebp), %eax
	andl	$65535, %eax
	subl	$4, %esp
	pushl	-8(%ebp)
	pushl	%eax
	pushl	$0
LCFI35:
	call	_io_Out32
	addl	$16, %esp
	leave
	ret
LFE8:
.globl _mc_outp
_mc_outp:
LFB65:
	pushl	%ebp
LCFI36:
	movl	%esp, %ebp
LCFI37:
	subl	$8, %esp
LCFI38:
	movb	_MC_useMMIO, %al
	testb	%al, %al
	je	L24
	cmpl	$-1, 8(%ebp)
	jne	L25
	subl	$8, %esp
	pushl	16(%ebp)
	pushl	12(%ebp)
LCFI39:
	call	__ZL11mmio_outpdwmm
	addl	$16, %esp
	jmp	L28
L25:
	movl	8(%ebp), %edx
	movl	%edx, %eax
	addl	%eax, %eax
	addl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	sall	$4, %eax
	addl	12(%ebp), %eax
	addl	$64, %eax
	subl	$8, %esp
	pushl	16(%ebp)
	pushl	%eax
	call	__ZL11mmio_outpdwmm
	addl	$16, %esp
	jmp	L28
L24:
LCFI40:
	call	_io_DisableINT
	movl	8(%ebp), %eax
	leal	1(%eax), %ecx
	movl	$1, %eax
	movl	%eax, %edx
	sall	%cl, %edx
	movl	__ZL12PWM_baseAddr, %eax
	addl	$208, %eax
	andl	$65535, %eax
	call	__ZL9io_outpdwtm
	movl	__ZL12PWM_baseAddr, %eax
	movl	%eax, %edx
	movl	12(%ebp), %eax
	leal	(%edx,%eax), %eax
	andl	$65535, %eax
	movl	16(%ebp), %edx
	call	__ZL9io_outpdwtm
	call	_io_RestoreINT
L28:
	leave
	ret
LFE65:
.globl _mchall_SetCapCtrlREG
_mchall_SetCapCtrlREG:
LFB224:
	pushl	%ebp
LCFI41:
	movl	%esp, %ebp
LCFI42:
	subl	$8, %esp
LCFI43:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI44:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE224:
.globl _mchall_SetCtrlREG
_mchall_SetCtrlREG:
LFB222:
	pushl	%ebp
LCFI45:
	movl	%esp, %ebp
LCFI46:
	subl	$8, %esp
LCFI47:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI48:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE222:
.globl _mchall_SetCntEvt
_mchall_SetCntEvt:
LFB215:
	pushl	%ebp
LCFI49:
	movl	%esp, %ebp
LCFI50:
	subl	$8, %esp
LCFI51:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$28, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI52:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE215:
.globl _mchall_SetCntMax
_mchall_SetCntMax:
LFB214:
	pushl	%ebp
LCFI53:
	movl	%esp, %ebp
LCFI54:
	subl	$8, %esp
LCFI55:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI56:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE214:
.globl _mchall_SetCntMin
_mchall_SetCntMin:
LFB213:
	pushl	%ebp
LCFI57:
	movl	%esp, %ebp
LCFI58:
	subl	$8, %esp
LCFI59:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$16, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI60:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE213:
.globl _mchall_SetHallCnt
_mchall_SetHallCnt:
LFB209:
	pushl	%ebp
LCFI61:
	movl	%esp, %ebp
LCFI62:
	subl	$8, %esp
LCFI63:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$12, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI64:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE209:
.globl _mcenc_SetCapCtrlREG
_mcenc_SetCapCtrlREG:
LFB204:
	pushl	%ebp
LCFI65:
	movl	%esp, %ebp
LCFI66:
	subl	$8, %esp
LCFI67:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI68:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE204:
.globl _mcenc_SetIdxCondREG
_mcenc_SetIdxCondREG:
LFB202:
	pushl	%ebp
LCFI69:
	movl	%esp, %ebp
LCFI70:
	subl	$8, %esp
LCFI71:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$36, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI72:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE202:
.globl _mcenc_SetCtrlREG
_mcenc_SetCtrlREG:
LFB200:
	pushl	%ebp
LCFI73:
	movl	%esp, %ebp
LCFI74:
	subl	$8, %esp
LCFI75:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI76:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE200:
.globl _mcenc_SetCntEvt
_mcenc_SetCntEvt:
LFB189:
	pushl	%ebp
LCFI77:
	movl	%esp, %ebp
LCFI78:
	subl	$8, %esp
LCFI79:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$28, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI80:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE189:
.globl _mcenc_SetCntIdx
_mcenc_SetCntIdx:
LFB188:
	pushl	%ebp
LCFI81:
	movl	%esp, %ebp
LCFI82:
	subl	$8, %esp
LCFI83:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$24, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI84:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE188:
.globl _mcenc_SetCntMax
_mcenc_SetCntMax:
LFB187:
	pushl	%ebp
LCFI85:
	movl	%esp, %ebp
LCFI86:
	subl	$8, %esp
LCFI87:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI88:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE187:
.globl _mcenc_SetCntMin
_mcenc_SetCntMin:
LFB186:
	pushl	%ebp
LCFI89:
	movl	%esp, %ebp
LCFI90:
	subl	$8, %esp
LCFI91:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$16, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI92:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE186:
.globl _mcenc_SetPulCnt
_mcenc_SetPulCnt:
LFB182:
	pushl	%ebp
LCFI93:
	movl	%esp, %ebp
LCFI94:
	subl	$8, %esp
LCFI95:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$12, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI96:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE182:
.globl _mcsif_ClearINT
_mcsif_ClearINT:
LFB178:
	pushl	%ebp
LCFI97:
	movl	%esp, %ebp
LCFI98:
	subl	$8, %esp
LCFI99:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_intOffset(,%eax,4), %ecx
	sall	%cl, 16(%ebp)
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	$4
	pushl	8(%ebp)
LCFI100:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE178:
.globl _mcsif_SetInputFilter
_mcsif_SetInputFilter:
LFB174:
	pushl	%ebp
LCFI101:
	movl	%esp, %ebp
LCFI102:
	subl	$8, %esp
LCFI103:
	movl	16(%ebp), %eax
	movl	%eax, %edx
	andl	$-1073676289, %edx
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$8, %eax
	subl	$4, %esp
	pushl	%edx
	pushl	%eax
	pushl	8(%ebp)
LCFI104:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE174:
.globl _mcsif_SetSWDeadband
_mcsif_SetSWDeadband:
LFB172:
	pushl	%ebp
LCFI105:
	movl	%esp, %ebp
LCFI106:
	subl	$8, %esp
LCFI107:
	movl	16(%ebp), %eax
	movl	%eax, %edx
	andl	$16777215, %edx
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$4, %eax
	subl	$4, %esp
	pushl	%edx
	pushl	%eax
	pushl	8(%ebp)
LCFI108:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE172:
.globl _mcsv_SetDDAPOS
_mcsv_SetDDAPOS:
LFB152:
	pushl	%ebp
LCFI109:
	movl	%esp, %ebp
LCFI110:
	subl	$8, %esp
LCFI111:
	movl	12(%ebp), %eax
	addl	$48, %eax
	sall	$2, %eax
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%eax
	pushl	8(%ebp)
LCFI112:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE152:
.globl _mcsv_SetDDA
_mcsv_SetDDA:
LFB122:
	pushl	%ebp
LCFI113:
	movl	%esp, %ebp
LCFI114:
	subl	$8, %esp
LCFI115:
	movl	16(%ebp), %eax
	movl	%eax, %edx
	sall	$31, %edx
	movl	20(%ebp), %eax
	andl	$2147483647, %eax
	orl	%eax, %edx
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$32, %eax
	subl	$4, %esp
	pushl	%edx
	pushl	%eax
	pushl	8(%ebp)
LCFI116:
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE122:
.globl _mcsv_SetWidth
_mcsv_SetWidth:
LFB120:
	pushl	%ebp
LCFI117:
	movl	%esp, %ebp
LCFI118:
	subl	$24, %esp
LCFI119:
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	movl	%eax, -4(%ebp)
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI120:
	call	_mc_outp
	addl	$16, %esp
	movl	-4(%ebp), %eax
	addl	$4, %eax
	subl	$4, %esp
	pushl	20(%ebp)
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE120:
.globl _mcpwm_SetWidth
_mcpwm_SetWidth:
LFB87:
	pushl	%ebp
LCFI121:
	movl	%esp, %ebp
LCFI122:
	subl	$24, %esp
LCFI123:
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	movl	%eax, -4(%ebp)
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI124:
	call	_mc_outp
	addl	$16, %esp
	movl	-4(%ebp), %eax
	addl	$4, %eax
	subl	$4, %esp
	pushl	20(%ebp)
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE87:
__ZL15write_mc_pciregjm:
LFB57:
	pushl	%ebp
LCFI125:
	movl	%esp, %ebp
LCFI126:
	subl	$24, %esp
LCFI127:
	movl	8(%ebp), %eax
	subl	$2147450880, %eax
	movl	%eax, -4(%ebp)
	call	_io_DisableINT
	movl	-4(%ebp), %eax
	movl	%eax, %edx
	andl	$-4, %edx
	movl	$3320, %eax
	call	__ZL9io_outpdwtm
	movl	12(%ebp), %edx
	movl	$3324, %eax
	call	__ZL9io_outpdwtm
	call	_io_RestoreINT
	leave
	ret
LFE57:
__ZL7io_inpbt:
LFB13:
	pushl	%ebp
LCFI128:
	movl	%esp, %ebp
LCFI129:
	subl	$8, %esp
LCFI130:
	movw	%ax, -4(%ebp)
	movl	-4(%ebp), %eax
	andl	$65535, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	$0
LCFI131:
	call	_io_In8
	addl	$16, %esp
	leave
	ret
LFE13:
.globl _mc_inpb
_mc_inpb:
LFB68:
	pushl	%ebp
LCFI132:
	movl	%esp, %ebp
LCFI133:
	subl	$24, %esp
LCFI134:
	movb	_MC_useMMIO, %al
	testb	%al, %al
	je	L76
	cmpl	$-1, 8(%ebp)
	jne	L77
	subl	$12, %esp
	pushl	12(%ebp)
LCFI135:
	call	__ZL9mmio_inpbm
	addl	$16, %esp
	movb	%al, -17(%ebp)
	jmp	L78
L77:
	movl	8(%ebp), %edx
	movl	%edx, %eax
	addl	%eax, %eax
	addl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	sall	$4, %eax
	addl	12(%ebp), %eax
	addl	$64, %eax
	subl	$12, %esp
	pushl	%eax
	call	__ZL9mmio_inpbm
	addl	$16, %esp
	movb	%al, -17(%ebp)
	jmp	L78
L76:
LCFI136:
	call	_io_DisableINT
	movl	8(%ebp), %eax
	leal	1(%eax), %ecx
	movl	$1, %eax
	movl	%eax, %edx
	sall	%cl, %edx
	movl	__ZL12PWM_baseAddr, %eax
	addl	$208, %eax
	andl	$65535, %eax
	call	__ZL9io_outpdwtm
	movl	__ZL12PWM_baseAddr, %eax
	movl	%eax, %edx
	movl	12(%ebp), %eax
	leal	(%edx,%eax), %eax
	andl	$65535, %eax
	call	__ZL7io_inpbt
	movb	%al, -1(%ebp)
	call	_io_RestoreINT
	movb	-1(%ebp), %al
	movb	%al, -17(%ebp)
L78:
	movb	-17(%ebp), %al
	leave
	ret
LFE68:
.globl _mcsv_ReadReloadOUT
_mcsv_ReadReloadOUT:
LFB166:
	pushl	%ebp
LCFI137:
	movl	%esp, %ebp
LCFI138:
	subl	$8, %esp
LCFI139:
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$42, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI140:
	call	_mc_inpb
	addl	$16, %esp
	andl	$-16, %eax
	leave
	ret
LFE166:
.globl _mcsv_ReadReloadEVT
_mcsv_ReadReloadEVT:
LFB165:
	pushl	%ebp
LCFI141:
	movl	%esp, %ebp
LCFI142:
	subl	$8, %esp
LCFI143:
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$41, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI144:
	call	_mc_inpb
	addl	$16, %esp
	andl	$-16, %eax
	leave
	ret
LFE165:
.globl _mcsv_ReadReloadSV
_mcsv_ReadReloadSV:
LFB164:
	pushl	%ebp
LCFI145:
	movl	%esp, %ebp
LCFI146:
	subl	$8, %esp
LCFI147:
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$43, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI148:
	call	_mc_inpb
	addl	$16, %esp
	andl	$-16, %eax
	leave
	ret
LFE164:
.globl _mcpwm_ReadReloadOUT
_mcpwm_ReadReloadOUT:
LFB116:
	pushl	%ebp
LCFI149:
	movl	%esp, %ebp
LCFI150:
	subl	$8, %esp
LCFI151:
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$42, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI152:
	call	_mc_inpb
	addl	$16, %esp
	andl	$-16, %eax
	leave
	ret
LFE116:
.globl _mcpwm_ReadReloadEVT
_mcpwm_ReadReloadEVT:
LFB115:
	pushl	%ebp
LCFI153:
	movl	%esp, %ebp
LCFI154:
	subl	$8, %esp
LCFI155:
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$41, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI156:
	call	_mc_inpb
	addl	$16, %esp
	andl	$-16, %eax
	leave
	ret
LFE115:
.globl _mcpwm_ReadReloadPWM
_mcpwm_ReadReloadPWM:
LFB114:
	pushl	%ebp
LCFI157:
	movl	%esp, %ebp
LCFI158:
	subl	$8, %esp
LCFI159:
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$43, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI160:
	call	_mc_inpb
	addl	$16, %esp
	andl	$-16, %eax
	leave
	ret
LFE114:
__ZL8io_outpbth:
LFB10:
	pushl	%ebp
LCFI161:
	movl	%esp, %ebp
LCFI162:
	subl	$8, %esp
LCFI163:
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
LCFI164:
	call	_io_Out8
	addl	$16, %esp
	leave
	ret
LFE10:
.globl _mc_outpb
_mc_outpb:
LFB67:
	pushl	%ebp
LCFI165:
	movl	%esp, %ebp
LCFI166:
	subl	$8, %esp
LCFI167:
	movl	16(%ebp), %eax
	movb	%al, -4(%ebp)
	movb	_MC_useMMIO, %al
	testb	%al, %al
	je	L95
	cmpl	$-1, 8(%ebp)
	jne	L96
	movl	$0, %eax
	movb	-4(%ebp), %al
	subl	$8, %esp
	pushl	%eax
	pushl	12(%ebp)
LCFI168:
	call	__ZL10mmio_outpbmh
	addl	$16, %esp
	jmp	L99
L96:
	movl	$0, %ecx
	movb	-4(%ebp), %cl
	movl	8(%ebp), %edx
	movl	%edx, %eax
	addl	%eax, %eax
	addl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	sall	$4, %eax
	addl	12(%ebp), %eax
	addl	$64, %eax
	subl	$8, %esp
	pushl	%ecx
	pushl	%eax
	call	__ZL10mmio_outpbmh
	addl	$16, %esp
	jmp	L99
L95:
LCFI169:
	call	_io_DisableINT
	movl	8(%ebp), %eax
	leal	1(%eax), %ecx
	movl	$1, %eax
	movl	%eax, %edx
	sall	%cl, %edx
	movl	__ZL12PWM_baseAddr, %eax
	addl	$208, %eax
	andl	$65535, %eax
	call	__ZL9io_outpdwtm
	movl	$0, %ecx
	movb	-4(%ebp), %cl
	movl	__ZL12PWM_baseAddr, %eax
	movl	%eax, %edx
	movl	12(%ebp), %eax
	leal	(%edx,%eax), %eax
	andl	$65535, %eax
	movl	%ecx, %edx
	call	__ZL8io_outpbth
	call	_io_RestoreINT
L99:
	leave
	ret
LFE67:
.globl _mcsv_ReloadOUT
_mcsv_ReloadOUT:
LFB163:
	pushl	%ebp
LCFI170:
	movl	%esp, %ebp
LCFI171:
	subl	$8, %esp
LCFI172:
	movl	16(%ebp), %eax
	movb	%al, -4(%ebp)
	movl	$0, %edx
	movb	-4(%ebp), %dl
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$42, %eax
	subl	$4, %esp
	pushl	%edx
	pushl	%eax
	pushl	8(%ebp)
LCFI173:
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE163:
.globl _mcsv_ReloadEVT
_mcsv_ReloadEVT:
LFB162:
	pushl	%ebp
LCFI174:
	movl	%esp, %ebp
LCFI175:
	subl	$8, %esp
LCFI176:
	movl	16(%ebp), %eax
	movb	%al, -4(%ebp)
	movl	$0, %edx
	movb	-4(%ebp), %dl
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$41, %eax
	subl	$4, %esp
	pushl	%edx
	pushl	%eax
	pushl	8(%ebp)
LCFI177:
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE162:
.globl _mcsv_ReloadSV
_mcsv_ReloadSV:
LFB161:
	pushl	%ebp
LCFI178:
	movl	%esp, %ebp
LCFI179:
	subl	$8, %esp
LCFI180:
	movl	16(%ebp), %eax
	movb	%al, -4(%ebp)
	movl	$0, %edx
	movb	-4(%ebp), %dl
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$43, %eax
	subl	$4, %esp
	pushl	%edx
	pushl	%eax
	pushl	8(%ebp)
LCFI181:
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE161:
.globl _mcsv_SetPABInitOut
_mcsv_SetPABInitOut:
LFB156:
	pushl	%ebp
LCFI182:
	movl	%esp, %ebp
LCFI183:
	subl	$24, %esp
LCFI184:
	movl	16(%ebp), %eax
	movb	%al, -20(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$40, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI185:
	call	_mc_inpb
	addl	$16, %esp
	movb	%al, %dl
	andl	$-13, %edx
	movb	-20(%ebp), %al
	orl	%edx, %eax
	andl	$255, %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE156:
.globl _mcsv_ClearSCLAST
_mcsv_ClearSCLAST:
LFB138:
	pushl	%ebp
LCFI186:
	movl	%esp, %ebp
LCFI187:
	subl	$24, %esp
LCFI188:
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$40, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI189:
	call	_mc_inpb
	addl	$16, %esp
	andl	$255, %eax
	andl	$254, %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE138:
.globl _mcsv_SetSCLAST
_mcsv_SetSCLAST:
LFB137:
	pushl	%ebp
LCFI190:
	movl	%esp, %ebp
LCFI191:
	subl	$24, %esp
LCFI192:
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$40, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI193:
	call	_mc_inpb
	addl	$16, %esp
	orl	$1, %eax
	andl	$255, %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE137:
.globl _mcsv_ClearSCRESET
_mcsv_ClearSCRESET:
LFB136:
	pushl	%ebp
LCFI194:
	movl	%esp, %ebp
LCFI195:
	subl	$24, %esp
LCFI196:
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$40, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI197:
	call	_mc_inpb
	addl	$16, %esp
	andl	$255, %eax
	andl	$253, %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE136:
.globl _mcsv_SetSCRESET
_mcsv_SetSCRESET:
LFB135:
	pushl	%ebp
LCFI198:
	movl	%esp, %ebp
LCFI199:
	subl	$24, %esp
LCFI200:
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$40, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI201:
	call	_mc_inpb
	addl	$16, %esp
	orl	$2, %eax
	andl	$255, %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE135:
.globl _mcpwm_ReloadOUT
_mcpwm_ReloadOUT:
LFB111:
	pushl	%ebp
LCFI202:
	movl	%esp, %ebp
LCFI203:
	subl	$8, %esp
LCFI204:
	movl	16(%ebp), %eax
	movb	%al, -4(%ebp)
	movl	$0, %edx
	movb	-4(%ebp), %dl
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$42, %eax
	subl	$4, %esp
	pushl	%edx
	pushl	%eax
	pushl	8(%ebp)
LCFI205:
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE111:
.globl _mcpwm_ReloadEVT
_mcpwm_ReloadEVT:
LFB110:
	pushl	%ebp
LCFI206:
	movl	%esp, %ebp
LCFI207:
	subl	$8, %esp
LCFI208:
	movl	16(%ebp), %eax
	movb	%al, -4(%ebp)
	movl	$0, %edx
	movb	-4(%ebp), %dl
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$41, %eax
	subl	$4, %esp
	pushl	%edx
	pushl	%eax
	pushl	8(%ebp)
LCFI209:
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE110:
.globl _mcpwm_ReloadPWM
_mcpwm_ReloadPWM:
LFB109:
	pushl	%ebp
LCFI210:
	movl	%esp, %ebp
LCFI211:
	subl	$8, %esp
LCFI212:
	movl	16(%ebp), %eax
	movb	%al, -4(%ebp)
	movl	$0, %edx
	movb	-4(%ebp), %dl
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$43, %eax
	subl	$4, %esp
	pushl	%edx
	pushl	%eax
	pushl	8(%ebp)
LCFI213:
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE109:
.globl _mcpwm_ClearSCLAST
_mcpwm_ClearSCLAST:
LFB92:
	pushl	%ebp
LCFI214:
	movl	%esp, %ebp
LCFI215:
	subl	$24, %esp
LCFI216:
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$40, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI217:
	call	_mc_inpb
	addl	$16, %esp
	andl	$255, %eax
	andl	$254, %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE92:
.globl _mcpwm_SetSCLAST
_mcpwm_SetSCLAST:
LFB91:
	pushl	%ebp
LCFI218:
	movl	%esp, %ebp
LCFI219:
	subl	$24, %esp
LCFI220:
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$40, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI221:
	call	_mc_inpb
	addl	$16, %esp
	orl	$1, %eax
	andl	$255, %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE91:
.globl _mcpwm_ClearSCRESET
_mcpwm_ClearSCRESET:
LFB90:
	pushl	%ebp
LCFI222:
	movl	%esp, %ebp
LCFI223:
	subl	$24, %esp
LCFI224:
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$40, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI225:
	call	_mc_inpb
	addl	$16, %esp
	andl	$255, %eax
	andl	$253, %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE90:
.globl _mcpwm_SetSCRESET
_mcpwm_SetSCRESET:
LFB89:
	pushl	%ebp
LCFI226:
	movl	%esp, %ebp
LCFI227:
	subl	$24, %esp
LCFI228:
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$40, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI229:
	call	_mc_inpb
	addl	$16, %esp
	orl	$2, %eax
	andl	$255, %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
LFE89:
__ZL8io_inpdwt:
LFB11:
	pushl	%ebp
LCFI230:
	movl	%esp, %ebp
LCFI231:
	subl	$8, %esp
LCFI232:
	movw	%ax, -4(%ebp)
	movl	-4(%ebp), %eax
	andl	$65535, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	$0
LCFI233:
	call	_io_In32
	addl	$16, %esp
	leave
	ret
LFE11:
.globl _mc_inp
_mc_inp:
LFB66:
	pushl	%ebp
LCFI234:
	movl	%esp, %ebp
LCFI235:
	subl	$24, %esp
LCFI236:
	movb	_MC_useMMIO, %al
	testb	%al, %al
	je	L133
	cmpl	$-1, 8(%ebp)
	jne	L134
	subl	$12, %esp
	pushl	12(%ebp)
LCFI237:
	call	__ZL10mmio_inpdwm
	addl	$16, %esp
	movl	%eax, -20(%ebp)
	jmp	L135
L134:
	movl	8(%ebp), %edx
	movl	%edx, %eax
	addl	%eax, %eax
	addl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	sall	$4, %eax
	addl	12(%ebp), %eax
	addl	$64, %eax
	subl	$12, %esp
	pushl	%eax
	call	__ZL10mmio_inpdwm
	addl	$16, %esp
	movl	%eax, -20(%ebp)
	jmp	L135
L133:
LCFI238:
	call	_io_DisableINT
	movl	8(%ebp), %eax
	leal	1(%eax), %ecx
	movl	$1, %eax
	movl	%eax, %edx
	sall	%cl, %edx
	movl	__ZL12PWM_baseAddr, %eax
	addl	$208, %eax
	andl	$65535, %eax
	call	__ZL9io_outpdwtm
	movl	__ZL12PWM_baseAddr, %eax
	movl	%eax, %edx
	movl	12(%ebp), %eax
	leal	(%edx,%eax), %eax
	andl	$65535, %eax
	call	__ZL8io_inpdwt
	movl	%eax, -4(%ebp)
	call	_io_RestoreINT
	movl	-4(%ebp), %eax
	movl	%eax, -20(%ebp)
L135:
	movl	-20(%ebp), %eax
	leave
	ret
LFE66:
.globl _mcpfau_ReadCapStatREG
_mcpfau_ReadCapStatREG:
LFB271:
	pushl	%ebp
LCFI239:
	movl	%esp, %ebp
LCFI240:
	subl	$8, %esp
LCFI241:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$40, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI242:
	call	_mc_inp
	addl	$16, %esp
	leave
	ret
LFE271:
.globl _mcpfau_ReadCap3CtrlREG
_mcpfau_ReadCap3CtrlREG:
LFB270:
	pushl	%ebp
LCFI243:
	movl	%esp, %ebp
LCFI244:
	subl	$8, %esp
LCFI245:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI246:
	call	_mc_inp
	addl	$16, %esp
	leave
	ret
LFE270:
.globl _mcpfau_ReadCap2CtrlREG
_mcpfau_ReadCap2CtrlREG:
LFB269:
	pushl	%ebp
LCFI247:
	movl	%esp, %ebp
LCFI248:
	subl	$8, %esp
LCFI249:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI250:
	call	_mc_inp
	addl	$16, %esp
	leave
	ret
LFE269:
.globl _mcpfau_ReadCap1CtrlREG
_mcpfau_ReadCap1CtrlREG:
LFB268:
	pushl	%ebp
LCFI251:
	movl	%esp, %ebp
LCFI252:
	subl	$8, %esp
LCFI253:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI254:
	call	_mc_inp
	addl	$16, %esp
	leave
	ret
LFE268:
.globl _mcpfau_SetFAU3TRIG
_mcpfau_SetFAU3TRIG:
LFB267:
	pushl	%ebp
LCFI255:
	movl	%esp, %ebp
LCFI256:
	subl	$24, %esp
LCFI257:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI258:
	call	_mc_inp
	addl	$16, %esp
	andl	$-50331649, %eax
	orl	16(%ebp), %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE267:
.globl _mcpfau_SetFAU2TRIG
_mcpfau_SetFAU2TRIG:
LFB266:
	pushl	%ebp
LCFI259:
	movl	%esp, %ebp
LCFI260:
	subl	$24, %esp
LCFI261:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI262:
	call	_mc_inp
	addl	$16, %esp
	andl	$-50331649, %eax
	orl	16(%ebp), %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE266:
.globl _mcpfau_SetFAU1TRIG
_mcpfau_SetFAU1TRIG:
LFB265:
	pushl	%ebp
LCFI263:
	movl	%esp, %ebp
LCFI264:
	subl	$24, %esp
LCFI265:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI266:
	call	_mc_inp
	addl	$16, %esp
	andl	$-50331649, %eax
	orl	16(%ebp), %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE265:
.globl _mcpfau_SetFAU3RELS
_mcpfau_SetFAU3RELS:
LFB264:
	pushl	%ebp
LCFI267:
	movl	%esp, %ebp
LCFI268:
	subl	$24, %esp
LCFI269:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI270:
	call	_mc_inp
	addl	$16, %esp
	andl	$-201326593, %eax
	orl	16(%ebp), %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE264:
.globl _mcpfau_SetFAU2RELS
_mcpfau_SetFAU2RELS:
LFB263:
	pushl	%ebp
LCFI271:
	movl	%esp, %ebp
LCFI272:
	subl	$24, %esp
LCFI273:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI274:
	call	_mc_inp
	addl	$16, %esp
	andl	$-201326593, %eax
	orl	16(%ebp), %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE263:
.globl _mcpfau_SetFAU1RELS
_mcpfau_SetFAU1RELS:
LFB262:
	pushl	%ebp
LCFI275:
	movl	%esp, %ebp
LCFI276:
	subl	$24, %esp
LCFI277:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI278:
	call	_mc_inp
	addl	$16, %esp
	andl	$-201326593, %eax
	orl	16(%ebp), %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE262:
.globl _mcpfau_ClearFAU3STAT
_mcpfau_ClearFAU3STAT:
LFB261:
	pushl	%ebp
LCFI279:
	movl	%esp, %ebp
LCFI280:
	subl	$24, %esp
LCFI281:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI282:
	call	_mc_inp
	addl	$16, %esp
	orl	$268435456, %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE261:
.globl _mcpfau_ClearFAU2STAT
_mcpfau_ClearFAU2STAT:
LFB260:
	pushl	%ebp
LCFI283:
	movl	%esp, %ebp
LCFI284:
	subl	$24, %esp
LCFI285:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI286:
	call	_mc_inp
	addl	$16, %esp
	orl	$268435456, %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE260:
.globl _mcpfau_ClearFAU1STAT
_mcpfau_ClearFAU1STAT:
LFB259:
	pushl	%ebp
LCFI287:
	movl	%esp, %ebp
LCFI288:
	subl	$24, %esp
LCFI289:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI290:
	call	_mc_inp
	addl	$16, %esp
	orl	$268435456, %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE259:
.globl _mcpfau_ReadFAU3STAT
_mcpfau_ReadFAU3STAT:
LFB258:
	pushl	%ebp
LCFI291:
	movl	%esp, %ebp
LCFI292:
	subl	$8, %esp
LCFI293:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI294:
	call	_mc_inp
	addl	$16, %esp
	shrl	$28, %eax
	andl	$1, %eax
	leave
	ret
LFE258:
.globl _mcpfau_ReadFAU2STAT
_mcpfau_ReadFAU2STAT:
LFB257:
	pushl	%ebp
LCFI295:
	movl	%esp, %ebp
LCFI296:
	subl	$8, %esp
LCFI297:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI298:
	call	_mc_inp
	addl	$16, %esp
	shrl	$28, %eax
	andl	$1, %eax
	leave
	ret
LFE257:
.globl _mcpfau_ReadFAU1STAT
_mcpfau_ReadFAU1STAT:
LFB256:
	pushl	%ebp
LCFI299:
	movl	%esp, %ebp
LCFI300:
	subl	$8, %esp
LCFI301:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI302:
	call	_mc_inp
	addl	$16, %esp
	shrl	$28, %eax
	andl	$1, %eax
	leave
	ret
LFE256:
.globl _mcpfau_ReadEXTRIG3
_mcpfau_ReadEXTRIG3:
LFB255:
	pushl	%ebp
LCFI303:
	movl	%esp, %ebp
LCFI304:
	subl	$8, %esp
LCFI305:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI306:
	call	_mc_inp
	addl	$16, %esp
	andl	$12582912, %eax
	leave
	ret
LFE255:
.globl _mcpfau_ReadEXTRIG2
_mcpfau_ReadEXTRIG2:
LFB254:
	pushl	%ebp
LCFI307:
	movl	%esp, %ebp
LCFI308:
	subl	$8, %esp
LCFI309:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI310:
	call	_mc_inp
	addl	$16, %esp
	andl	$12582912, %eax
	leave
	ret
LFE254:
.globl _mcpfau_ReadEXTRIG1
_mcpfau_ReadEXTRIG1:
LFB253:
	pushl	%ebp
LCFI311:
	movl	%esp, %ebp
LCFI312:
	subl	$8, %esp
LCFI313:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	subl	$8, %esp
	pushl	%eax
	pushl	8(%ebp)
LCFI314:
	call	_mc_inp
	addl	$16, %esp
	andl	$12582912, %eax
	leave
	ret
LFE253:
.globl _mcpfau_SetEXTRIG3
_mcpfau_SetEXTRIG3:
LFB252:
	pushl	%ebp
LCFI315:
	movl	%esp, %ebp
LCFI316:
	subl	$24, %esp
LCFI317:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI318:
	call	_mc_inp
	addl	$16, %esp
	andl	$-12582913, %eax
	orl	16(%ebp), %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE252:
.globl _mcpfau_SetEXTRIG2
_mcpfau_SetEXTRIG2:
LFB251:
	pushl	%ebp
LCFI319:
	movl	%esp, %ebp
LCFI320:
	subl	$24, %esp
LCFI321:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI322:
	call	_mc_inp
	addl	$16, %esp
	andl	$-12582913, %eax
	orl	16(%ebp), %eax
	subl	$4, %esp
	pushl	%eax
	pushl	-4(%ebp)
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
LFE251:
.globl _mcpfau_SetEXTRIG1
_mcpfau_SetEXTRIG1:
LFB250:
	pushl	%ebp
LCFI323:
	movl	%esp, %ebp
LCFI324:
	subl	$24, %esp
LCFI325:
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	movl	%eax, -4(%ebp)
	subl	$8, %esp
	pushl	-4(%ebp)
	pushl	8(%ebp)
LCFI326:
	call	_mc_inp
	addl	$16, %esp
	andl	$-12582913, %eax
	orl	16(%ebp), %eax
	subl	$4, %esp
	p