	.file	"mcm.cpp"
	.section	.text.set_MMIO,"x"
.globl _set_MMIO
_set_MMIO:
	pushl	%ebp
	movl	%esp, %ebp
	movb	$1, _MC_useMMIO
	popl	%ebp
	ret
	.section	.text.set_PortIO,"x"
.globl _set_PortIO
_set_PortIO:
	pushl	%ebp
	movl	%esp, %ebp
	movb	$0, _MC_useMMIO
	popl	%ebp
	ret
	.section	.text._ZL11mmio_outpdwmm,"x"
__ZL11mmio_outpdwmm:
	pushl	%ebp
	movl	%esp, %ebp
/APP
# 113 "c:/include/sys/farptr.h" 1
	movw __ZL13mmio_selector,%fs 
	.byte 0x64 
	movl %edx,(%eax)
# 0 "" 2
/NO_APP
	popl	%ebp
	ret
	.section	.text._ZL10mmio_inpdwm,"x"
__ZL10mmio_inpdwm:
	pushl	%ebp
	movl	%esp, %ebp
/APP
# 151 "c:/include/sys/farptr.h" 1
	movw __ZL13mmio_selector,%fs
	.byte 0x64
	movl (%eax),%eax
# 0 "" 2
/NO_APP
	popl	%ebp
	ret
	.section	.text._ZL10mmio_outpbmh,"x"
__ZL10mmio_outpbmh:
	pushl	%ebp
	movl	%esp, %ebp
/APP
# 89 "c:/include/sys/farptr.h" 1
	movw __ZL13mmio_selector,%fs
	.byte 0x64 
	movb %dl,(%eax)
# 0 "" 2
/NO_APP
	popl	%ebp
	ret
	.section	.text._ZL9mmio_inpbm,"x"
__ZL9mmio_inpbm:
	pushl	%ebp
	movl	%esp, %ebp
/APP
# 125 "c:/include/sys/farptr.h" 1
	movw __ZL13mmio_selector,%fs 
	.byte 0x64 
	movb (%eax),%al
# 0 "" 2
/NO_APP
	popl	%ebp
	ret
	.section	.text.mc_outp,"x"
.globl _mc_outp
_mc_outp:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %ebx
	movl	12(%ebp), %esi
	cmpb	$0, _MC_useMMIO
	je	L14
	cmpl	$-1, %ebx
	jne	L15
	movl	16(%ebp), %edx
	movl	%esi, %eax
	call	__ZL11mmio_outpdwmm
	jmp	L17
L15:
	leal	(%ebx,%ebx,2), %eax
	leal	(%ebx,%eax,4), %eax
	sall	$4, %eax
	leal	64(%esi,%eax), %eax
	movl	16(%ebp), %edx
	call	__ZL11mmio_outpdwmm
	jmp	L17
L14:
	call	_io_DisableINT
	subl	$4, %esp
	leal	1(%ebx), %ecx
	movl	$1, %eax
	sall	%cl, %eax
	pushl	%eax
	movl	__ZL12PWM_baseAddr, %eax
	addl	$208, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$12, %esp
	pushl	16(%ebp)
	movl	__ZL12PWM_baseAddr, %eax
	addl	%esi, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	call	_io_RestoreINT
	addl	$16, %esp
L17:
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mchall_SetCapCtrlREG,"x"
.globl _mchall_SetCapCtrlREG
_mchall_SetCapCtrlREG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mchall_SetCtrlREG,"x"
.globl _mchall_SetCtrlREG
_mchall_SetCtrlREG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mchall_SetCntEvt,"x"
.globl _mchall_SetCntEvt
_mchall_SetCntEvt:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$28, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mchall_SetCntMax,"x"
.globl _mchall_SetCntMax
_mchall_SetCntMax:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mchall_SetCntMin,"x"
.globl _mchall_SetCntMin
_mchall_SetCntMin:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$16, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mchall_SetHallCnt,"x"
.globl _mchall_SetHallCnt
_mchall_SetHallCnt:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$12, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mcenc_SetCapCtrlREG,"x"
.globl _mcenc_SetCapCtrlREG
_mcenc_SetCapCtrlREG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mcenc_SetIdxCondREG,"x"
.globl _mcenc_SetIdxCondREG
_mcenc_SetIdxCondREG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$36, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mcenc_SetCtrlREG,"x"
.globl _mcenc_SetCtrlREG
_mcenc_SetCtrlREG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mcenc_SetCntEvt,"x"
.globl _mcenc_SetCntEvt
_mcenc_SetCntEvt:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$28, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mcenc_SetCntIdx,"x"
.globl _mcenc_SetCntIdx
_mcenc_SetCntIdx:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$24, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mcenc_SetCntMax,"x"
.globl _mcenc_SetCntMax
_mcenc_SetCntMax:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mcenc_SetCntMin,"x"
.globl _mcenc_SetCntMin
_mcenc_SetCntMin:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$16, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mcenc_SetPulCnt,"x"
.globl _mcenc_SetPulCnt
_mcenc_SetPulCnt:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$12, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mcsif_ClearINT,"x"
.globl _mcsif_ClearINT
_mcsif_ClearINT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_intOffset(,%eax,4), %ecx
	movl	16(%ebp), %eax
	sall	%cl, %eax
	pushl	%eax
	pushl	$4
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mcsif_SetInputFilter,"x"
.globl _mcsif_SetInputFilter
_mcsif_SetInputFilter:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	16(%ebp), %eax
	andl	$-1073676289, %eax
	pushl	%eax
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$8, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mcsif_SetSWDeadband,"x"
.globl _mcsif_SetSWDeadband
_mcsif_SetSWDeadband:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	16(%ebp), %eax
	andl	$16777215, %eax
	pushl	%eax
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$4, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mcsv_SetDDAPOS,"x"
.globl _mcsv_SetDDAPOS
_mcsv_SetDDAPOS:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	16(%ebp)
	movl	12(%ebp), %eax
	leal	192(,%eax,4), %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mcsv_SetDDA,"x"
.globl _mcsv_SetDDA
_mcsv_SetDDA:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	20(%ebp), %edx
	andl	$2147483647, %edx
	movl	16(%ebp), %eax
	sall	$31, %eax
	orl	%edx, %eax
	pushl	%eax
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$32, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leave
	ret
	.section	.text.mcsv_SetWidth,"x"
.globl _mcsv_SetWidth
_mcsv_SetWidth:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %ebx
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %esi
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%esi
	pushl	%ebx
	call	_mc_outp
	addl	$12, %esp
	pushl	20(%ebp)
	addl	$4, %esi
	pushl	%esi
	pushl	%ebx
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpwm_SetWidth,"x"
.globl _mcpwm_SetWidth
_mcpwm_SetWidth:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %ebx
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %esi
	subl	$4, %esp
	pushl	16(%ebp)
	pushl	%esi
	pushl	%ebx
	call	_mc_outp
	addl	$12, %esp
	pushl	20(%ebp)
	addl	$4, %esi
	pushl	%esi
	pushl	%ebx
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mc_inpb,"x"
.globl _mc_inpb
_mc_inpb:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %ebx
	movl	12(%ebp), %esi
	cmpb	$0, _MC_useMMIO
	je	L61
	cmpl	$-1, %ebx
	jne	L62
	movl	%esi, %eax
	call	__ZL9mmio_inpbm
	movb	%al, %bl
	jmp	L63
L62:
	leal	(%ebx,%ebx,2), %eax
	leal	(%ebx,%eax,4), %eax
	sall	$4, %eax
	leal	64(%esi,%eax), %eax
	call	__ZL9mmio_inpbm
	movb	%al, %bl
	jmp	L63
L61:
	call	_io_DisableINT
	subl	$4, %esp
	leal	1(%ebx), %ecx
	movl	$1, %eax
	sall	%cl, %eax
	pushl	%eax
	movl	__ZL12PWM_baseAddr, %eax
	addl	$208, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$8, %esp
	movl	__ZL12PWM_baseAddr, %eax
	addl	%esi, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movb	%al, %bl
	call	_io_RestoreINT
	addl	$16, %esp
L63:
	movb	%bl, %al
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcsv_ReadReloadOUT,"x"
.globl _mcsv_ReadReloadOUT
_mcsv_ReadReloadOUT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$42, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inpb
	andl	$-16, %eax
	leave
	ret
	.section	.text.mcsv_ReadReloadEVT,"x"
.globl _mcsv_ReadReloadEVT
_mcsv_ReadReloadEVT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$41, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inpb
	andl	$-16, %eax
	leave
	ret
	.section	.text.mcsv_ReadReloadSV,"x"
.globl _mcsv_ReadReloadSV
_mcsv_ReadReloadSV:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$43, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inpb
	andl	$-16, %eax
	leave
	ret
	.section	.text.mcpwm_ReadReloadOUT,"x"
.globl _mcpwm_ReadReloadOUT
_mcpwm_ReadReloadOUT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$42, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inpb
	andl	$-16, %eax
	leave
	ret
	.section	.text.mcpwm_ReadReloadEVT,"x"
.globl _mcpwm_ReadReloadEVT
_mcpwm_ReadReloadEVT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$41, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inpb
	andl	$-16, %eax
	leave
	ret
	.section	.text.mcpwm_ReadReloadPWM,"x"
.globl _mcpwm_ReadReloadPWM
_mcpwm_ReadReloadPWM:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$43, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inpb
	andl	$-16, %eax
	leave
	ret
	.section	.text.mc_outpb,"x"
.globl _mc_outpb
_mc_outpb:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %ebx
	movl	12(%ebp), %edi
	movzbl	16(%ebp), %esi
	cmpb	$0, _MC_useMMIO
	je	L78
	cmpl	$-1, %ebx
	jne	L79
	movl	%esi, %edx
	andl	$255, %edx
	movl	%edi, %eax
	call	__ZL10mmio_outpbmh
	jmp	L81
L79:
	movl	%esi, %edx
	andl	$255, %edx
	leal	(%ebx,%ebx,2), %eax
	leal	(%ebx,%eax,4), %eax
	sall	$4, %eax
	leal	64(%edi,%eax), %eax
	call	__ZL10mmio_outpbmh
	jmp	L81
L78:
	call	_io_DisableINT
	subl	$4, %esp
	leal	1(%ebx), %ecx
	movl	$1, %eax
	sall	%cl, %eax
	pushl	%eax
	movl	__ZL12PWM_baseAddr, %eax
	addl	$208, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$12, %esp
	movl	%esi, %eax
	andl	$255, %eax
	pushl	%eax
	movl	__ZL12PWM_baseAddr, %eax
	addl	%edi, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	call	_io_RestoreINT
	addl	$16, %esp
L81:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text.mcsv_ReloadOUT,"x"
.globl _mcsv_ReloadOUT
_mcsv_ReloadOUT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	$0, %eax
	movb	16(%ebp), %al
	pushl	%eax
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$42, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
	.section	.text.mcsv_ReloadEVT,"x"
.globl _mcsv_ReloadEVT
_mcsv_ReloadEVT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	$0, %eax
	movb	16(%ebp), %al
	pushl	%eax
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$41, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
	.section	.text.mcsv_ReloadSV,"x"
.globl _mcsv_ReloadSV
_mcsv_ReloadSV:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	$0, %eax
	movb	16(%ebp), %al
	pushl	%eax
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %eax
	addl	$43, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
	.section	.text.mcsv_SetPABInitOut,"x"
.globl _mcsv_SetPABInitOut
_mcsv_SetPABInitOut:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$20, %esp
	movl	8(%ebp), %edi
	movb	16(%ebp), %bl
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %esi
	addl	$40, %esi
	pushl	%esi
	pushl	%edi
	call	_mc_inpb
	addl	$12, %esp
	andl	$-13, %eax
	orl	%eax, %ebx
	andl	$255, %ebx
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	call	_mc_outpb
	addl	$16, %esp
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text.mcsv_ClearSCLAST,"x"
.globl _mcsv_ClearSCLAST
_mcsv_ClearSCLAST:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %ebx
	addl	$40, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inpb
	addl	$12, %esp
	andl	$254, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outpb
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcsv_SetSCLAST,"x"
.globl _mcsv_SetSCLAST
_mcsv_SetSCLAST:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %ebx
	addl	$40, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inpb
	addl	$12, %esp
	orl	$1, %eax
	andl	$255, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outpb
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcsv_ClearSCRESET,"x"
.globl _mcsv_ClearSCRESET
_mcsv_ClearSCRESET:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %ebx
	addl	$40, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inpb
	addl	$12, %esp
	andl	$253, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outpb
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcsv_SetSCRESET,"x"
.globl _mcsv_SetSCRESET
_mcsv_SetSCRESET:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL14MCSV_modOffset(,%eax,4), %ebx
	addl	$40, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inpb
	addl	$12, %esp
	orl	$2, %eax
	andl	$255, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outpb
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpwm_ReloadOUT,"x"
.globl _mcpwm_ReloadOUT
_mcpwm_ReloadOUT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	$0, %eax
	movb	16(%ebp), %al
	pushl	%eax
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$42, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
	.section	.text.mcpwm_ReloadEVT,"x"
.globl _mcpwm_ReloadEVT
_mcpwm_ReloadEVT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	$0, %eax
	movb	16(%ebp), %al
	pushl	%eax
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$41, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
	.section	.text.mcpwm_ReloadPWM,"x"
.globl _mcpwm_ReloadPWM
_mcpwm_ReloadPWM:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	$0, %eax
	movb	16(%ebp), %al
	pushl	%eax
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %eax
	addl	$43, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_outpb
	addl	$16, %esp
	leave
	ret
	.section	.text.mcpwm_ClearSCLAST,"x"
.globl _mcpwm_ClearSCLAST
_mcpwm_ClearSCLAST:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %ebx
	addl	$40, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inpb
	addl	$12, %esp
	andl	$254, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outpb
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpwm_SetSCLAST,"x"
.globl _mcpwm_SetSCLAST
_mcpwm_SetSCLAST:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %ebx
	addl	$40, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inpb
	addl	$12, %esp
	orl	$1, %eax
	andl	$255, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outpb
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpwm_ClearSCRESET,"x"
.globl _mcpwm_ClearSCRESET
_mcpwm_ClearSCRESET:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %ebx
	addl	$40, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inpb
	addl	$12, %esp
	andl	$253, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outpb
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpwm_SetSCRESET,"x"
.globl _mcpwm_SetSCRESET
_mcpwm_SetSCRESET:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCPWM_modOffset(,%eax,4), %ebx
	addl	$40, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inpb
	addl	$12, %esp
	orl	$2, %eax
	andl	$255, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outpb
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mc_inp,"x"
.globl _mc_inp
_mc_inp:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %ebx
	movl	12(%ebp), %esi
	cmpb	$0, _MC_useMMIO
	je	L113
	cmpl	$-1, %ebx
	jne	L114
	movl	%esi, %eax
	call	__ZL10mmio_inpdwm
	movl	%eax, %ebx
	jmp	L115
L114:
	leal	(%ebx,%ebx,2), %eax
	leal	(%ebx,%eax,4), %eax
	sall	$4, %eax
	leal	64(%esi,%eax), %eax
	call	__ZL10mmio_inpdwm
	movl	%eax, %ebx
	jmp	L115
L113:
	call	_io_DisableINT
	subl	$4, %esp
	leal	1(%ebx), %ecx
	movl	$1, %eax
	sall	%cl, %eax
	pushl	%eax
	movl	__ZL12PWM_baseAddr, %eax
	addl	$208, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$8, %esp
	movl	__ZL12PWM_baseAddr, %eax
	addl	%esi, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	movl	%eax, %ebx
	call	_io_RestoreINT
	addl	$16, %esp
L115:
	movl	%ebx, %eax
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_ReadCapStatREG,"x"
.globl _mcpfau_ReadCapStatREG
_mcpfau_ReadCapStatREG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$40, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	leave
	ret
	.section	.text.mcpfau_ReadCap3CtrlREG,"x"
.globl _mcpfau_ReadCap3CtrlREG
_mcpfau_ReadCap3CtrlREG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	leave
	ret
	.section	.text.mcpfau_ReadCap2CtrlREG,"x"
.globl _mcpfau_ReadCap2CtrlREG
_mcpfau_ReadCap2CtrlREG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	leave
	ret
	.section	.text.mcpfau_ReadCap1CtrlREG,"x"
.globl _mcpfau_ReadCap1CtrlREG
_mcpfau_ReadCap1CtrlREG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	leave
	ret
	.section	.text.mcpfau_SetFAU3TRIG,"x"
.globl _mcpfau_SetFAU3TRIG
_mcpfau_SetFAU3TRIG:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$20, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-50331649, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetFAU2TRIG,"x"
.globl _mcpfau_SetFAU2TRIG
_mcpfau_SetFAU2TRIG:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$32, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-50331649, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetFAU1TRIG,"x"
.globl _mcpfau_SetFAU1TRIG
_mcpfau_SetFAU1TRIG:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$44, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-50331649, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetFAU3RELS,"x"
.globl _mcpfau_SetFAU3RELS
_mcpfau_SetFAU3RELS:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$20, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-201326593, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetFAU2RELS,"x"
.globl _mcpfau_SetFAU2RELS
_mcpfau_SetFAU2RELS:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$32, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-201326593, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetFAU1RELS,"x"
.globl _mcpfau_SetFAU1RELS
_mcpfau_SetFAU1RELS:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$44, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-201326593, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_ClearFAU3STAT,"x"
.globl _mcpfau_ClearFAU3STAT
_mcpfau_ClearFAU3STAT:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$20, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	orl	$268435456, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_ClearFAU2STAT,"x"
.globl _mcpfau_ClearFAU2STAT
_mcpfau_ClearFAU2STAT:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$32, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	orl	$268435456, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_ClearFAU1STAT,"x"
.globl _mcpfau_ClearFAU1STAT
_mcpfau_ClearFAU1STAT:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$44, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	orl	$268435456, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_ReadFAU3STAT,"x"
.globl _mcpfau_ReadFAU3STAT
_mcpfau_ReadFAU3STAT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	shrl	$28, %eax
	andl	$1, %eax
	leave
	ret
	.section	.text.mcpfau_ReadFAU2STAT,"x"
.globl _mcpfau_ReadFAU2STAT
_mcpfau_ReadFAU2STAT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	shrl	$28, %eax
	andl	$1, %eax
	leave
	ret
	.section	.text.mcpfau_ReadFAU1STAT,"x"
.globl _mcpfau_ReadFAU1STAT
_mcpfau_ReadFAU1STAT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	shrl	$28, %eax
	andl	$1, %eax
	leave
	ret
	.section	.text.mcpfau_ReadEXTRIG3,"x"
.globl _mcpfau_ReadEXTRIG3
_mcpfau_ReadEXTRIG3:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	andl	$12582912, %eax
	leave
	ret
	.section	.text.mcpfau_ReadEXTRIG2,"x"
.globl _mcpfau_ReadEXTRIG2
_mcpfau_ReadEXTRIG2:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	andl	$12582912, %eax
	leave
	ret
	.section	.text.mcpfau_ReadEXTRIG1,"x"
.globl _mcpfau_ReadEXTRIG1
_mcpfau_ReadEXTRIG1:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	andl	$12582912, %eax
	leave
	ret
	.section	.text.mcpfau_SetEXTRIG3,"x"
.globl _mcpfau_SetEXTRIG3
_mcpfau_SetEXTRIG3:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$20, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-12582913, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetEXTRIG2,"x"
.globl _mcpfau_SetEXTRIG2
_mcpfau_SetEXTRIG2:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$32, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-12582913, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetEXTRIG1,"x"
.globl _mcpfau_SetEXTRIG1
_mcpfau_SetEXTRIG1:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$44, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-12582913, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_ReadRLDTRIG3,"x"
.globl _mcpfau_ReadRLDTRIG3
_mcpfau_ReadRLDTRIG3:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$20, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	andl	$3145728, %eax
	leave
	ret
	.section	.text.mcpfau_ReadRLDTRIG2,"x"
.globl _mcpfau_ReadRLDTRIG2
_mcpfau_ReadRLDTRIG2:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	andl	$3145728, %eax
	leave
	ret
	.section	.text.mcpfau_ReadRLDTRIG1,"x"
.globl _mcpfau_ReadRLDTRIG1
_mcpfau_ReadRLDTRIG1:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	andl	$3145728, %eax
	leave
	ret
	.section	.text.mcpfau_SetRLDTRIG3,"x"
.globl _mcpfau_SetRLDTRIG3
_mcpfau_SetRLDTRIG3:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$20, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-3145729, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetRLDTRIG2,"x"
.globl _mcpfau_SetRLDTRIG2
_mcpfau_SetRLDTRIG2:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$32, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-3145729, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetRLDTRIG1,"x"
.globl _mcpfau_SetRLDTRIG1
_mcpfau_SetRLDTRIG1:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$44, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-3145729, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetMask3,"x"
.globl _mcpfau_SetMask3
_mcpfau_SetMask3:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$20, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-1879048193, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetMask2,"x"
.globl _mcpfau_SetMask2
_mcpfau_SetMask2:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$32, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-1879048193, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetMask1,"x"
.globl _mcpfau_SetMask1
_mcpfau_SetMask1:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$44, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-1879048193, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetPolarity3,"x"
.globl _mcpfau_SetPolarity3
_mcpfau_SetPolarity3:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$20, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$2147483647, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetPolarity2,"x"
.globl _mcpfau_SetPolarity2
_mcpfau_SetPolarity2:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$32, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$2147483647, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetPolarity1,"x"
.globl _mcpfau_SetPolarity1
_mcpfau_SetPolarity1:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$44, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$2147483647, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_ReadCAPFIFO3,"x"
.globl _mcpfau_ReadCAPFIFO3
_mcpfau_ReadCAPFIFO3:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$12, %esp
	movl	16(%ebp), %ebx
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$24, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	movl	%eax, %edx
	addl	$16, %esp
	testl	%ebx, %ebx
	je	L186
	andl	$268435455, %eax
	movl	%eax, (%ebx)
L186:
	movl	%edx, %eax
	andl	$-268435456, %eax
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text.mcpfau_ReadCAPFIFO2,"x"
.globl _mcpfau_ReadCAPFIFO2
_mcpfau_ReadCAPFIFO2:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$12, %esp
	movl	16(%ebp), %ebx
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$36, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	movl	%eax, %edx
	addl	$16, %esp
	testl	%ebx, %ebx
	je	L189
	andl	$268435455, %eax
	movl	%eax, (%ebx)
L189:
	movl	%edx, %eax
	andl	$-268435456, %eax
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text.mcpfau_ReadCAPFIFO1,"x"
.globl _mcpfau_ReadCAPFIFO1
_mcpfau_ReadCAPFIFO1:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$12, %esp
	movl	16(%ebp), %ebx
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$48, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	movl	%eax, %edx
	addl	$16, %esp
	testl	%ebx, %ebx
	je	L192
	andl	$268435455, %eax
	movl	%eax, (%ebx)
L192:
	movl	%edx, %eax
	andl	$-268435456, %eax
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text.mcpfau_ReadCAPSTAT3,"x"
.globl _mcpfau_ReadCAPSTAT3
_mcpfau_ReadCAPSTAT3:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$40, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	andl	$3072, %eax
	shrl	$10, %eax
	leave
	ret
	.section	.text.mcpfau_ReadCAPSTAT2,"x"
.globl _mcpfau_ReadCAPSTAT2
_mcpfau_ReadCAPSTAT2:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$40, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	andl	$768, %eax
	shrl	$8, %eax
	leave
	ret
	.section	.text.mcpfau_ReadCAPSTAT1,"x"
.globl _mcpfau_ReadCAPSTAT1
_mcpfau_ReadCAPSTAT1:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$40, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	andl	$3, %eax
	leave
	ret
	.section	.text.mcpfau_SetCapInterval3,"x"
.globl _mcpfau_SetCapInterval3
_mcpfau_SetCapInterval3:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$20, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	movl	16(%ebp), %edx
	andl	$511, %edx
	andl	$-512, %eax
	orl	%edx, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetCapInterval2,"x"
.globl _mcpfau_SetCapInterval2
_mcpfau_SetCapInterval2:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$32, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	movl	16(%ebp), %edx
	andl	$511, %edx
	andl	$-512, %eax
	orl	%edx, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetCapInterval1,"x"
.globl _mcpfau_SetCapInterval1
_mcpfau_SetCapInterval1:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$44, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	movl	16(%ebp), %edx
	andl	$511, %edx
	andl	$-512, %eax
	orl	%edx, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetCapMode3,"x"
.globl _mcpfau_SetCapMode3
_mcpfau_SetCapMode3:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$20, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-983041, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetCapMode2,"x"
.globl _mcpfau_SetCapMode2
_mcpfau_SetCapMode2:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$32, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-983041, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcpfau_SetCapMode1,"x"
.globl _mcpfau_SetCapMode1
_mcpfau_SetCapMode1:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$44, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-983041, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mchall_ReadCapCtrlREG,"x"
.globl _mchall_ReadCapCtrlREG
_mchall_ReadCapCtrlREG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	leave
	ret
	.section	.text.mchall_ReadCtrlREG,"x"
.globl _mchall_ReadCtrlREG
_mchall_ReadCtrlREG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	leave
	ret
	.section	.text.mchall_ReadCAPFIFO,"x"
.globl _mchall_ReadCAPFIFO
_mchall_ReadCAPFIFO:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$12, %esp
	movl	16(%ebp), %ebx
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$48, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	movl	%eax, %edx
	addl	$16, %esp
	testl	%ebx, %ebx
	je	L217
	andl	$268435455, %eax
	movl	%eax, (%ebx)
L217:
	movl	%edx, %eax
	andl	$-268435456, %eax
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text.mchall_ReadCAPSTAT,"x"
.globl _mchall_ReadCAPSTAT
_mchall_ReadCAPSTAT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$40, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	andl	$3, %eax
	leave
	ret
	.section	.text.mchall_SetCapInterval,"x"
.globl _mchall_SetCapInterval
_mchall_SetCapInterval:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$44, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	movl	16(%ebp), %edx
	andl	$511, %edx
	andl	$-512, %eax
	orl	%edx, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mchall_SetCapMode,"x"
.globl _mchall_SetCapMode
_mchall_SetCapMode:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$44, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-458753, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mchall_ClearSTERR,"x"
.globl _mchall_ClearSTERR
_mchall_ClearSTERR:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$40, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	orl	$8, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mchall_CheckSTERR,"x"
.globl _mchall_CheckSTERR
_mchall_CheckSTERR:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$40, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	shrl	$3, %eax
	andl	$1, %eax
	leave
	ret
	.section	.text.mchall_ReadPINS,"x"
.globl _mchall_ReadPINS
_mchall_ReadPINS:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$40, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	shrl	$5, %eax
	andl	$7, %eax
	leave
	ret
	.section	.text.mchall_ReadDIR,"x"
.globl _mchall_ReadDIR
_mchall_ReadDIR:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$40, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	shrl	$4, %eax
	andl	$1, %eax
	leave
	ret
	.section	.text.mchall_ReadHallCnt,"x"
.globl _mchall_ReadHallCnt
_mchall_ReadHallCnt:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$12, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	leave
	ret
	.section	.text.mchall_SetDirPattern,"x"
.globl _mchall_SetDirPattern
_mchall_SetDirPattern:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$20, %esp
	movl	16(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %edi
	addl	$32, %edi
	movl	%esi, %ebx
	andl	$262140, %ebx
	sall	$14, %ebx
	sall	$6, %esi
	andl	$255, %esi
	pushl	%edi
	pushl	8(%ebp)
	call	_mc_inp
	addl	$12, %esp
	orl	%esi, %ebx
	andl	$65343, %eax
	orl	%eax, %ebx
	pushl	%ebx
	pushl	%edi
	pushl	8(%ebp)
	call	_mc_outp
	addl	$16, %esp
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text.mchall_SetResetMode,"x"
.globl _mchall_SetResetMode
_mchall_SetResetMode:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$32, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-52, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mchall_SetCntMode,"x"
.globl _mchall_SetCntMode
_mchall_SetCntMode:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$32, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	xorb	%ah, %ah
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcenc_ReadCapCtrlREG,"x"
.globl _mcenc_ReadCapCtrlREG
_mcenc_ReadCapCtrlREG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	leave
	ret
	.section	.text.mcenc_ReadIdxCondREG,"x"
.globl _mcenc_ReadIdxCondREG
_mcenc_ReadIdxCondREG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$36, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	leave
	ret
	.section	.text.mcenc_ReadCtrlREG,"x"
.globl _mcenc_ReadCtrlREG
_mcenc_ReadCtrlREG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$32, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	leave
	ret
	.section	.text.mcenc_ReadRLDTRIG,"x"
.globl _mcenc_ReadRLDTRIG
_mcenc_ReadRLDTRIG:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$44, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	andl	$3145728, %eax
	leave
	ret
	.section	.text.mcenc_SetRLDTRIG,"x"
.globl _mcenc_SetRLDTRIG
_mcenc_SetRLDTRIG:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$44, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-3145729, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcenc_ReadCAPFIFO,"x"
.globl _mcenc_ReadCAPFIFO
_mcenc_ReadCAPFIFO:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$12, %esp
	movl	16(%ebp), %ebx
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$48, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	movl	%eax, %edx
	addl	$16, %esp
	testl	%ebx, %ebx
	je	L252
	andl	$268435455, %eax
	movl	%eax, (%ebx)
L252:
	movl	%edx, %eax
	andl	$-268435456, %eax
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text.mcenc_ReadCAPSTAT,"x"
.globl _mcenc_ReadCAPSTAT
_mcenc_ReadCAPSTAT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$40, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	andl	$3, %eax
	leave
	ret
	.section	.text.mcenc_SetCapInterval,"x"
.globl _mcenc_SetCapInterval
_mcenc_SetCapInterval:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$44, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	movl	16(%ebp), %edx
	andl	$511, %edx
	andl	$-512, %eax
	orl	%edx, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcenc_SetCapMode,"x"
.globl _mcenc_SetCapMode
_mcenc_SetCapMode:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$44, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andl	$-983041, %eax
	orl	16(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcenc_CheckIDXSTAT,"x"
.globl _mcenc_CheckIDXSTAT
_mcenc_CheckIDXSTAT:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$40, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	shrl	$2, %eax
	andl	$1, %eax
	leave
	ret
	.section	.text.mcenc_ClearIDXERR,"x"
.globl _mcenc_ClearIDXERR
_mcenc_ClearIDXERR:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$40, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	orl	$8, %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcenc_CheckIDXERR,"x"
.globl _mcenc_CheckIDXERR
_mcenc_CheckIDXERR:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$40, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	shrl	$3, %eax
	andl	$1, %eax
	leave
	ret
	.section	.text.mcenc_SetIdxCond,"x"
.globl _mcenc_SetIdxCond
_mcenc_SetIdxCond:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %esi
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %ebx
	addl	$36, %ebx
	subl	$8, %esp
	pushl	%ebx
	pushl	%esi
	call	_mc_inp
	addl	$12, %esp
	andb	$24, %al
	orl	16(%ebp), %eax
	orl	20(%ebp), %eax
	pushl	%eax
	pushl	%ebx
	pushl	%esi
	call	_mc_outp
	addl	$16, %esp
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.mcenc_ReadPINS,"x"
.globl _mcenc_ReadPINS
_mcenc_ReadPINS:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$40, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	shrl	$5, %eax
	andl	$7, %eax
	leave
	ret
	.section	.text.mcenc_ReadDIR,"x"
.globl _mcenc_ReadDIR
_mcenc_ReadDIR:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$40, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	shrl	$4, %eax
	andl	$1, %eax
	leave
	ret
	.section	.text.mcenc_ReadPulCnt,"x"
.globl _mcenc_ReadPulCnt
_mcenc_ReadPulCnt:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	12(%ebp), %eax
	movl	__ZL15MCSIF_modOffset(,%eax,4), %eax
	addl	$12, %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_mc_inp
	leave
	r