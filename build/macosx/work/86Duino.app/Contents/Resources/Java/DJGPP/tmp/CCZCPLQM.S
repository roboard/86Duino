	.file	"usb.cpp"
	.section	.text._ZL13SetEPnDLR_endv,"x"
__ZL13SetEPnDLR_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL15Set_Address_endv,"x"
__ZL15Set_Address_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL18Get_Descriptor_endv,"x"
__ZL18Get_Descriptor_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL18Set_Descriptor_endv,"x"
__ZL18Set_Descriptor_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL21Get_Configuration_endv,"x"
__ZL21Get_Configuration_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL21Set_Configuration_endv,"x"
__ZL21Set_Configuration_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL17Get_Interface_endv,"x"
__ZL17Get_Interface_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL17Set_Interface_endv,"x"
__ZL17Set_Interface_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL15Synch_Frame_endv,"x"
__ZL15Synch_Frame_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL24USB_Standard_Request_endv,"x"
__ZL24USB_Standard_Request_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL19Set_Line_Coding_endv,"x"
__ZL19Set_Line_Coding_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL19Get_Line_Coding_endv,"x"
__ZL19Get_Line_Coding_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL26Set_Control_Line_State_endv,"x"
__ZL26Set_Control_Line_State_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL14Send_Break_endv,"x"
__ZL14Send_Break_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL19USB_CDC_Request_endv,"x"
__ZL19USB_CDC_Request_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL20EP0_SetupHandler_endv,"x"
__ZL20EP0_SetupHandler_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL17EP0_InHandler_endv,"x"
__ZL17EP0_InHandler_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL18EP0_OutHandler_endv,"x"
__ZL18EP0_OutHandler_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL17EP1_InHandler_endv,"x"
__ZL17EP1_InHandler_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL17EP2_InHandler_endv,"x"
__ZL17EP2_InHandler_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL18EP2_OutHandler_endv,"x"
__ZL18EP2_OutHandler_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL13usb_Reset_endv,"x"
__ZL13usb_Reset_endv:
	pushl	%ebp
	movl	%esp, %ebp
	popl	%ebp
	ret
	.section	.text._ZL11USB_ISR_endv,"x"
__ZL11USB_ISR_endv:
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
	.ascii "%s: Init IO lib error!!\12\0"
LC1:
	.ascii "%s: Close IO lib error!!\12\0"
	.section	.text._ZL20set_gpio_config_addrt,"x"
__ZL20set_gpio_config_addrt:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	call	_io_Init
	testb	%al, %al
	jne	L50
	subl	$8, %esp
	pushl	$__ZZL20set_gpio_config_addrtE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	movb	$0, %dl
	addl	$16, %esp
	jmp	L51
L50:
	subl	$8, %esp
	pushl	$98
	pushl	$2
	call	_vx86_NBSB_Read16
	andl	$-2, %eax
	movw	%ax, __ZL16gpio_config_addr
	addl	$16, %esp
	testw	%ax, %ax
	jne	L52
	movw	$3328, __ZL16gpio_config_addr
	subl	$8, %esp
	pushl	$98
	pushl	$2
	call	_vx86_NBSB_Read16
	addl	$12, %esp
	orb	$13, %ah
	andl	$65535, %eax
	pushl	%eax
	pushl	$98
	pushl	$2
	call	_vx86_NBSB_Write16
	addl	$16, %esp
L52:
	subl	$8, %esp
	pushl	$98
	pushl	$2
	call	_vx86_NBSB_Read16
	addl	$12, %esp
	orl	$1, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$98
	pushl	$2
	call	_vx86_NBSB_Write16
	call	_io_Close
	addl	$16, %esp
	movb	$1, %dl
	testb	%al, %al
	jne	L51
	subl	$8, %esp
	pushl	$__ZZL20set_gpio_config_addrtE12__FUNCTION__
	pushl	$LC1
	call	__ZL9err_printPcz
	movb	$1, %dl
	addl	$16, %esp
L51:
	movb	%dl, %al
	leave
	ret
	.section .text
LC2:
	.ascii "%s: USB device is null.\12\0"
	.section	.text.usb_GetControlLineState,"x"
.globl _usb_GetControlLineState
_usb_GetControlLineState:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L56
	subl	$8, %esp
	pushl	$__ZZ23usb_GetControlLineStateE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movl	$-1, %eax
	addl	$16, %esp
	jmp	L57
L56:
	movl	72(%eax), %eax
L57:
	leave
	ret
	.section	.text.usb_TxReady,"x"
.globl _usb_TxReady
_usb_TxReady:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L60
	subl	$8, %esp
	pushl	$__ZZ11usb_TxReadyE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L61
L60:
	movl	52(%eax), %edx
	movl	8(%edx), %eax
	cmpl	12(%edx), %eax
	setb	%al
L61:
	leave
	ret
	.section	.text.usb_TxQueueEmpty,"x"
.globl _usb_TxQueueEmpty
_usb_TxQueueEmpty:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L64
	subl	$8, %esp
	pushl	$__ZZ16usb_TxQueueEmptyE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L65
L64:
	movl	52(%eax), %eax
	cmpl	$0, 8(%eax)
	sete	%al
L65:
	leave
	ret
	.section	.text.usb_TxQueueFull,"x"
.globl _usb_TxQueueFull
_usb_TxQueueFull:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L68
	subl	$8, %esp
	pushl	$__ZZ15usb_TxQueueFullE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L69
L68:
	movl	52(%eax), %edx
	movl	8(%edx), %eax
	cmpl	12(%edx), %eax
	setae	%al
L69:
	leave
	ret
	.section	.text.usb_QueryTxQueue,"x"
.globl _usb_QueryTxQueue
_usb_QueryTxQueue:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L72
	subl	$8, %esp
	pushl	$__ZZ16usb_QueryTxQueueE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movl	$0, %eax
	addl	$16, %esp
	jmp	L73
L72:
	movl	52(%eax), %eax
	movl	8(%eax), %eax
L73:
	leave
	ret
	.section	.text.usb_RxQueueEmpty,"x"
.globl _usb_RxQueueEmpty
_usb_RxQueueEmpty:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L76
	subl	$8, %esp
	pushl	$__ZZ16usb_RxQueueEmptyE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L77
L76:
	movl	48(%eax), %eax
	cmpl	$0, 8(%eax)
	sete	%al
L77:
	leave
	ret
	.section	.text.usb_RxQueueFull,"x"
.globl _usb_RxQueueFull
_usb_RxQueueFull:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L80
	subl	$8, %esp
	pushl	$__ZZ15usb_RxQueueFullE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L81
L80:
	movl	48(%eax), %edx
	movl	8(%edx), %eax
	cmpl	12(%edx), %eax
	setae	%al
L81:
	leave
	ret
	.section	.text.usb_QueryRxQueue,"x"
.globl _usb_QueryRxQueue
_usb_QueryRxQueue:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L84
	subl	$8, %esp
	pushl	$__ZZ16usb_QueryRxQueueE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movl	$0, %eax
	addl	$16, %esp
	jmp	L85
L84:
	movl	48(%eax), %eax
	movl	8(%eax), %eax
L85:
	leave
	ret
	.section	.text.usb_GetLineCoding,"x"
.globl _usb_GetLineCoding
_usb_GetLineCoding:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L88
	subl	$8, %esp
	pushl	$__ZZ17usb_GetLineCodingE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movl	$0, %eax
	addl	$16, %esp
	jmp	L89
L88:
	addl	$64, %eax
L89:
	leave
	ret
	.section	.text.usb_SetTimeOut,"x"
.globl _usb_SetTimeOut
_usb_SetTimeOut:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L92
	subl	$8, %esp
	pushl	$__ZZ14usb_SetTimeOutE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L94
L92:
	movl	12(%ebp), %edx
	movl	%edx, 32(%eax)
L94:
	leave
	ret
	.section	.text.usb_State,"x"
.globl _usb_State
_usb_State:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L96
	subl	$8, %esp
	pushl	$__ZZ9usb_StateE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movl	$0, %eax
	addl	$16, %esp
	jmp	L97
L96:
	movl	12(%eax), %eax
L97:
	leave
	ret
	.section	.text.usb_Ready,"x"
.globl _usb_Ready
_usb_Ready:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	testl	%eax, %eax
	jne	L100
	subl	$8, %esp
	pushl	$__ZZ9usb_ReadyE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L101
L100:
	subl	$12, %esp
	pushl	%eax
	call	_usb_State
	addl	$16, %esp
	cmpl	$7, %eax
	sete	%al
L101:
	leave
	ret
	.section	.text.usb_FlushWFIFO,"x"
.globl _usb_FlushWFIFO
_usb_FlushWFIFO:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	testl	%ebx, %ebx
	jne	L104
	subl	$8, %esp
	pushl	$__ZZ14usb_FlushWFIFOE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L108
L104:
	movl	52(%ebx), %eax
	cmpl	$0, 8(%eax)
	jne	L106
L109:
	subl	$8, %esp
	movl	188(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	addl	$16, %esp
	testl	%eax, %eax
	js	L109
	jmp	L108
L106:
L111:
	jmp	L111
L108:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZL9io_outpdwtm,"x"
__ZL9io_outpdwtm:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	%edx
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
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
	.section	.text.usb_FlushTxQueue,"x"
.globl _usb_FlushTxQueue
_usb_FlushTxQueue:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	testl	%ebx, %ebx
	jne	L119
	subl	$8, %esp
	pushl	$__ZZ16usb_FlushTxQueueE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L121
L119:
	call	_io_DisableINT
	subl	$12, %esp
	pushl	52(%ebx)
	call	_ClearQueue
	call	_io_RestoreINT
	addl	$16, %esp
L121:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text.usb_FlushRxQueue,"x"
.globl _usb_FlushRxQueue
_usb_FlushRxQueue:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	testl	%ebx, %ebx
	jne	L123
	subl	$8, %esp
	pushl	$__ZZ16usb_FlushRxQueueE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L125
L123:
	call	_io_DisableINT
	subl	$12, %esp
	pushl	48(%ebx)
	call	_ClearQueue
	call	_io_RestoreINT
	addl	$16, %esp
L125:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZL11USB_Connectv,"x"
__ZL11USB_Connectv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movw	__ZL15usb_on_off_data, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	movsbl	__ZL14usb_on_off_pin,%ecx
	movl	$-2, %edx
	roll	%cl, %edx
	andl	%edx, %eax
	andl	$255, %eax
	pushl	%eax
	movw	__ZL15usb_on_off_data, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
	leave
	ret
	.section	.text._ZL9TX_LED_ONv,"x"
__ZL9TX_LED_ONv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movw	__ZL11tx_led_data, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	movsbl	__ZL10tx_led_pin,%ecx
	movl	$-2, %edx
	roll	%cl, %edx
	andl	%edx, %eax
	andl	$255, %eax
	pushl	%eax
	movw	__ZL11tx_led_data, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	movl	$100, __ZL12tx_led_count
	addl	$16, %esp
	leave
	ret
	.section	.text._ZL10TX_LED_OFFv,"x"
__ZL10TX_LED_OFFv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movw	__ZL11tx_led_data, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	movsbl	__ZL10tx_led_pin,%ecx
	movl	$1, %edx
	sall	%cl, %edx
	orl	%edx, %eax
	andl	$255, %eax
	pushl	%eax
	movw	__ZL11tx_led_data, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
	leave
	ret
	.section	.text._ZL10RX_LED_OFFv,"x"
__ZL10RX_LED_OFFv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movw	__ZL11rx_led_data, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	movsbl	__ZL10rx_led_pin,%ecx
	movl	$1, %edx
	sall	%cl, %edx
	orl	%edx, %eax
	andl	$255, %eax
	pushl	%eax
	movw	__ZL11rx_led_data, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
	leave
	ret
	.section	.text._ZL9RX_LED_ONv,"x"
__ZL9RX_LED_ONv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movw	__ZL11rx_led_data, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	movsbl	__ZL10rx_led_pin,%ecx
	movl	$-2, %edx
	roll	%cl, %edx
	andl	%edx, %eax
	andl	$255, %eax
	pushl	%eax
	movw	__ZL11rx_led_data, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	movl	$100, __ZL12rx_led_count
	addl	$16, %esp
	leave
	ret
	.section .text
LC3:
	.ascii "%s: Free DMA buffer fail!!\12\0"
	.section	.text.usb_Close,"x"
.globl _usb_Close
_usb_Close:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	testl	%ebx, %ebx
	jne	L137
	subl	$8, %esp
	pushl	$__ZZ9usb_CloseE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L143
L137:
	cmpl	$0, 20(%ebx)
	je	L139
	call	_io_DisableINT
	subl	$8, %esp
	movl	80(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	andl	$254, %eax
	pushl	%eax
	movl	80(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$8, %esp
	pushl	%ebx
	pushl	4(%ebx)
	call	_irq_UninstallISR
	call	_io_RestoreINT
	call	_irq_Close
	addl	$12, %esp
	pushl	$2
	movl	80(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
L140:
	subl	$8, %esp
	movl	80(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$16, %esp
	testb	$2, %al
	jne	L140
	subl	$12, %esp
	pushl	__ZL10dma_handle
	call	___dpmi_free_dos_memory
	addl	$16, %esp
	cmpl	$-1, %eax
	jne	L141
	subl	$8, %esp
	pushl	$__ZZ9usb_CloseE12__FUNCTION__
	pushl	$LC3
	call	__ZL9err_printPcz
	addl	$16, %esp
L141:
	subl	$12, %esp
	pushl	112(%ebx)
	call	_ker_Mfree
	addl	$4, %esp
	pushl	116(%ebx)
	call	_ker_Mfree
	addl	$4, %esp
	pushl	120(%ebx)
	call	_ker_Mfree
	addl	$4, %esp
	pushl	160(%ebx)
	call	_ker_Mfree
	addl	$4, %esp
	pushl	204(%ebx)
	call	_ker_Mfree
	addl	$4, %esp
	pushl	208(%ebx)
	call	_ker_Mfree
	call	_io_Close
	addl	$16, %esp
	testb	%al, %al
	jne	L142
	subl	$8, %esp
	pushl	$__ZZ9usb_CloseE12__FUNCTION__
	pushl	$LC1
	call	__ZL9err_printPcz
	addl	$16, %esp
L142:
	movl	$2, 12(%ebx)
	movl	$0, 20(%ebx)
	subl	$8, %esp
	movw	__ZL15usb_on_off_data, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	movsbl	__ZL14usb_on_off_pin,%ecx
	movl	$1, %edx
	sall	%cl, %edx
	orl	%edx, %eax
	andl	$255, %eax
	pushl	%eax
	movw	__ZL15usb_on_off_data, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
L139:
	subl	$12, %esp
	pushl	48(%ebx)
	call	_DestoryQueue
	addl	$4, %esp
	pushl	52(%ebx)
	call	_DestoryQueue
	movl	%ebx, (%esp)
	call	_ker_Mfree
	addl	$16, %esp
L143:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section .text
	.p2align 2
LC4:
	.ascii "%s: init USB-DEV DETECT and ONOFF pins fail.\12\0"
	.section	.text.usb_SetUSBPins,"x"
.globl _usb_SetUSBPins
_usb_SetUSBPins:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	subl	$16, %esp
	movb	12(%ebp), %al
	movb	%al, -9(%ebp)
	movb	16(%ebp), %cl
	movb	%cl, -10(%ebp)
	movb	20(%ebp), %al
	movb	%al, -11(%ebp)
	movb	24(%ebp), %cl
	movb	%cl, -12(%ebp)
	cmpl	$0, 8(%ebp)
	jne	L146
	subl	$8, %esp
	pushl	$__ZZ14usb_SetUSBPinsE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L147
L146:
	subl	$12, %esp
	pushl	$3328
	call	__ZL20set_gpio_config_addrt
	addl	$16, %esp
	testb	%al, %al
	jne	L148
	subl	$8, %esp
	pushl	$__ZZ14usb_SetUSBPinsE12__FUNCTION__
	pushl	$LC4
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L147
L148:
	movsbl	-9(%ebp),%ebx
	subl	$8, %esp
	movw	__ZL16gpio_config_addr, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	addl	$12, %esp
	movl	$1, %edx
	movb	%bl, %cl
	sall	%cl, %edx
	orl	%edx, %eax
	pushl	%eax
	movw	__ZL16gpio_config_addr, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	leal	4(,%ebx,4), %esi
	addl	$8, %esp
	movl	%esi, %eax
	addw	__ZL16gpio_config_addr, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In16
	movw	%ax, __ZL15usb_detect_data
	addl	$16, %esp
	testw	%ax, %ax
	jne	L149
	movsbw	-9(%ebp),%ax
	subl	$3840, %eax
	movw	%ax, __ZL15usb_detect_data
	subl	$4, %esp
	andl	$65535, %eax
	pushl	%eax
	movl	%esi, %eax
	addw	__ZL16gpio_config_addr, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	addl	$16, %esp
L149:
	sall	$2, %ebx
	subl	$8, %esp
	movw	__ZL16gpio_config_addr, %ax
	leal	6(%ebx,%eax), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In16
	movw	%ax, __ZL14usb_detect_dir
	addl	$16, %esp
	testw	%ax, %ax
	jne	L150
	movsbw	-9(%ebp),%ax
	subl	$3830, %eax
	movw	%ax, __ZL14usb_detect_dir
	subl	$4, %esp
	andl	$65535, %eax
	pushl	%eax
	movw	__ZL16gpio_config_addr, %ax
	leal	6(%ebx,%eax), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	addl	$16, %esp
L150:
	subl	$8, %esp
	movw	__ZL14usb_detect_dir, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	movl	$1, %ebx
	movl	%ebx, %edx
	movb	-10(%ebp), %cl
	sall	%cl, %edx
	notl	%edx
	andl	%edx, %eax
	andl	$255, %eax
	pushl	%eax
	movw	__ZL14usb_detect_dir, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	movb	-10(%ebp), %al
	movb	%al, __ZL14usb_detect_pin
	movsbl	-11(%ebp),%esi
	addl	$8, %esp
	movw	__ZL16gpio_config_addr, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	addl	$12, %esp
	movl	%esi, %ecx
	sall	%cl, %ebx
	orl	%ebx, %eax
	pushl	%eax
	movw	__ZL16gpio_config_addr, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	leal	4(,%esi,4), %ebx
	addl	$8, %esp
	movl	%ebx, %eax
	addw	__ZL16gpio_config_addr, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In16
	movw	%ax, __ZL15usb_on_off_data
	addl	$16, %esp
	testw	%ax, %ax
	jne	L151
	movsbw	-11(%ebp),%ax
	subl	$3840, %eax
	movw	%ax, __ZL15usb_on_off_data
	subl	$4, %esp
	andl	$65535, %eax
	pushl	%eax
	movl	%ebx, %eax
	addw	__ZL16gpio_config_addr, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	addl	$16, %esp
L151:
	leal	0(,%esi,4), %ebx
	subl	$8, %esp
	movw	__ZL16gpio_config_addr, %ax
	leal	6(%ebx,%eax), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In16
	movw	%ax, __ZL14usb_on_off_dir
	addl	$16, %esp
	testw	%ax, %ax
	jne	L152
	movsbw	-11(%ebp),%ax
	subl	$3830, %eax
	movw	%ax, __ZL14usb_on_off_dir
	subl	$4, %esp
	andl	$65535, %eax
	pushl	%eax
	movw	__ZL16gpio_config_addr, %ax
	leal	6(%ebx,%eax), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	addl	$16, %esp
L152:
	subl	$8, %esp
	movw	__ZL14usb_on_off_dir, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movl	$1, %ebx
	movb	-12(%ebp), %cl
	sall	%cl, %ebx
	addl	$12, %esp
	orl	%ebx, %eax
	andl	$255, %eax
	pushl	%eax
	movw	__ZL14usb_on_off_dir, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$8, %esp
	movw	__ZL15usb_on_off_data, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	orl	%ebx, %eax
	andl	$255, %eax
	pushl	%eax
	movw	__ZL15usb_on_off_data, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	movb	-12(%ebp), %al
	movb	%al, __ZL14usb_on_off_pin
	call	__ZL11USB_Connectv
	movb	$1, %al
	addl	$16, %esp
L147:
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
	.section	.text.CreateUSBDevice,"x"
.globl _CreateUSBDevice
_CreateUSBDevice:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%ebx
	subl	$12, %esp
	pushl	$268
	call	_ker_Malloc
	movl	%eax, %ebx
	addl	$16, %esp
	movl	$0, %eax
	testl	%ebx, %ebx
	je	L156
	call	_io_Init
	testb	%al, %al
	jne	L157
	subl	$8, %esp
	pushl	$__ZZ15CreateUSBDeviceE12__FUNCTION__
	pushl	$LC0
	call	__ZL9err_printPcz
	movl	%ebx, (%esp)
	call	_ker_Mfree
	movl	$0, %eax
	addl	$16, %esp
	jmp	L156
L157:
	call	_vx86_GetUSBDevAddr
	movw	%ax, (%ebx)
	call	_vx86_GetUSBDevIRQ
	movl	%eax, 4(%ebx)
	cmpw	$0, (%ebx)
	je	L158
	testl	%eax, %eax
	jne	L159
L158:
	call	_io_Close
	subl	$12, %esp
	pushl	%ebx
	call	_ker_Mfree
	movl	$0, %eax
	addl	$16, %esp
	jmp	L156
L159:
	movb	$0, 2(%ebx)
	movb	$0, 8(%ebx)
	movl	$0, 12(%ebx)
	movb	$0, 16(%ebx)
	movl	$0, 20(%ebx)
	movl	$0, 24(%ebx)
	movb	$0, 28(%ebx)
	movb	$0, 29(%ebx)
	movb	$0, 30(%ebx)
	movl	$-1, 32(%ebx)
	movl	$0, 36(%ebx)
	movl	$0, 40(%ebx)
	movw	$0, 44(%ebx)
	movw	$0, 46(%ebx)
	subl	$12, %esp
	pushl	$4096
	call	_CreateQueue
	movl	%eax, 48(%ebx)
	addl	$16, %esp
	testl	%eax, %eax
	je	L160
	subl	$12, %esp
	pushl	$4096
	call	_CreateQueue
	movl	%eax, 52(%ebx)
	addl	$16, %esp
	testl	%eax, %eax
	je	L161
	movb	$0, 56(%ebx)
	movb	$0, 57(%ebx)
	movw	$0, 58(%ebx)
	movw	$0, 60(%ebx)
	movw	$0, 62(%ebx)
	movl	$0, 64(%ebx)
	movb	$0, 68(%ebx)
	movb	$0, 69(%ebx)
	movb	$0, 70(%ebx)
	movw	$0, 72(%ebx)
	movw	$0, 74(%ebx)
	movl	(%ebx), %eax
	movw	%ax, 78(%ebx)
	leal	2(%eax), %edx
	movw	%dx, 80(%ebx)
	leal	6(%eax), %edx
	movw	%dx, 82(%ebx)
	leal	8(%eax), %edx
	movw	%dx, 84(%ebx)
	leal	12(%eax), %edx
	movw	%dx, 86(%ebx)
	addl	$104, %eax
	movw	%ax, 88(%ebx)
	leal	92(%ebx), %edi
	movl	$44, %ecx
	movl	$0, %eax
	rep stosl
	movl	(%ebx), %eax
	leal	16(%eax), %edx
	movw	%dx, 92(%ebx)
	leal	18(%eax), %edx
	movw	%dx, 140(%ebx)
	leal	20(%eax), %edx
	movw	%dx, 138(%ebx)
	leal	22(%eax), %edx
	movw	%dx, 184(%ebx)
	leal	24(%eax), %edx
	movw	%dx, 182(%ebx)
	leal	26(%eax), %edx
	movw	%dx, 228(%ebx)
	leal	28(%eax), %edx
	movw	%dx, 226(%ebx)
	leal	32(%eax), %edx
	movw	%dx, 98(%ebx)
	leal	36(%eax), %edx
	movw	%dx, 102(%ebx)
	leal	40(%eax), %edx
	movw	%dx, 100(%ebx)
	leal	44(%eax), %edx
	movw	%dx, 146(%ebx)
	leal	48(%eax), %edx
	movw	%dx, 144(%ebx)
	leal	52(%eax), %edx
	movw	%dx, 190(%ebx)
	leal	56(%eax), %edx
	movw	%dx, 188(%ebx)
	leal	60(%eax), %edx
	movw	%dx, 234(%ebx)
	leal	64(%eax), %edx
	movw	%dx, 232(%ebx)
	leal	68(%eax), %edx
	movw	%dx, 104(%ebx)
	leal	72(%eax), %edx
	movw	%dx, 108(%ebx)
	leal	76(%eax), %edx
	movw	%dx, 106(%ebx)
	leal	80(%eax), %edx
	movw	%dx, 152(%ebx)
	leal	84(%eax), %edx
	movw	%dx, 150(%ebx)
	leal	88(%eax), %edx
	movw	%dx, 196(%ebx)
	leal	92(%eax), %edx
	movw	%dx, 194(%ebx)
	leal	96(%eax), %edx
	movw	%dx, 240(%ebx)
	addl	$100, %eax
	movw	%ax, 238(%ebx)
	subl	$12, %esp
	pushl	$3328
	call	__ZL20set_gpio_config_addrt
	addl	$8, %esp
	movw	__ZL16gpio_config_addr, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	addl	$12, %esp
	orb	$-128, %al
	pushl	%eax
	movw	__ZL16gpio_config_addr, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$8, %esp
	movw	__ZL16gpio_config_addr, %ax
	addl	$32, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In16
	movw	%ax, __ZL11tx_led_data
	addl	$16, %esp
	testw	%ax, %ax
	jne	L163
	movw	$-3833, __ZL11tx_led_data
	subl	$4, %esp
	pushl	$61703
	movw	__ZL16gpio_config_addr, %ax
	addl	$32, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	addl	$16, %esp
L163:
	subl	$8, %esp
	movw	__ZL16gpio_config_addr, %ax
	addl	$34, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In16
	movw	%ax, __ZL10tx_led_dir
	addl	$16, %esp
	testw	%ax, %ax
	jne	L164
	movw	$-3823, __ZL10tx_led_dir
	subl	$4, %esp
	pushl	$61713
	movw	__ZL16gpio_config_addr, %ax
	addl	$34, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	addl	$16, %esp
L164:
	subl	$8, %esp
	movw	__ZL10tx_led_dir, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	orl	$4, %eax
	andl	$255, %eax
	pushl	%eax
	movw	__ZL10tx_led_dir, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	movb	$2, __ZL10tx_led_pin
	addl	$8, %esp
	movw	__ZL16gpio_config_addr, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	addl	$12, %esp
	orb	$-128, %al
	pushl	%eax
	movw	__ZL16gpio_config_addr, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$8, %esp
	movw	__ZL16gpio_config_addr, %ax
	addl	$32, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In16
	movw	%ax, __ZL11rx_led_data
	addl	$16, %esp
	testw	%ax, %ax
	jne	L165
	movw	$-3833, __ZL11rx_led_data
	subl	$4, %esp
	pushl	$61703
	movw	__ZL16gpio_config_addr, %ax
	addl	$32, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	addl	$16, %esp
L165:
	subl	$8, %esp
	movw	__ZL16gpio_config_addr, %ax
	addl	$34, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In16
	movw	%ax, __ZL10rx_led_dir
	addl	$16, %esp
	testw	%ax, %ax
	jne	L166
	movw	$-3823, __ZL10rx_led_dir
	subl	$4, %esp
	pushl	$61713
	movw	__ZL16gpio_config_addr, %ax
	addl	$34, %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	addl	$16, %esp
L166:
	subl	$8, %esp
	movw	__ZL10rx_led_dir, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$12, %esp
	orl	$8, %eax
	andl	$255, %eax
	pushl	%eax
	movw	__ZL10rx_led_dir, %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	movb	$3, __ZL10rx_led_pin
	call	__ZL10TX_LED_OFFv
	call	__ZL10RX_LED_OFFv
	movl	%ebx, %eax
	addl	$16, %esp
	jmp	L156
L161:
	subl	$12, %esp
	pushl	48(%ebx)
	call	_DestoryQueue
	addl	$16, %esp
L160:
	call	_io_Close
	subl	$12, %esp
	pushl	%ebx
	call	_ker_Mfree
	movl	$0, %eax
	addl	$16, %esp
L156:
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZL9SetEPnDLRP10usb_deviceiim,"x"
__ZL9SetEPnDLRP10usb_deviceiim:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	12(%ebp), %edx
	movl	16(%ebp), %eax
	cmpl	$1, %eax
	je	L171
	cmpl	$2, %eax
	je	L172
	testl	%eax, %eax
	jne	L173
	subl	$4, %esp
	pushl	20(%ebp)
	leal	(%edx,%edx,4), %eax
	leal	(%edx,%eax,2), %eax
	movl	8(%ebp), %edx
	movl	100(%edx,%eax,4), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
	jmp	L173
L171:
	subl	$4, %esp
	pushl	20(%ebp)
	leal	(%edx,%edx,4), %eax
	leal	(%edx,%eax,2), %eax
	movl	8(%ebp), %ecx
	movw	102(%ecx,%eax,4), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
	jmp	L173
L172:
	subl	$4, %esp
	pushl	20(%ebp)
	leal	(%edx,%edx,4), %eax
	leal	(%edx,%eax,2), %eax
	movl	8(%ebp), %edx
	movw	98(%edx,%eax,4), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L173:
	leave
	ret
	.section	.text._ZL17Set_ConfigurationP10usb_device,"x"
__ZL17Set_ConfigurationP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	movl	12(%ebx), %eax
	cmpl	$3, %eax
	je	L182
	cmpb	$0, 56(%ebx)
	js	L182
	cmpb	$0, 59(%ebx)
	jne	L182
	cmpw	$0, 60(%ebx)
	jne	L182
	cmpw	$0, 62(%ebx)
	jne	L182
	cmpl	$4, %eax
	je	L176
	cmpl	$5, %eax
	jne	L182
	jmp	L183
L176:
	movb	58(%ebx), %al
	testb	%al, %al
	je	L178
	cmpb	$1, %al
	jne	L179
	movl	$5, 12(%ebx)
	pushl	$-2147483584
	pushl	$1
	pushl	$2
	pushl	%ebx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$32, %esp
	jmp	L182
L179:
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L182
L178:
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L182
L183:
	movb	58(%ebx), %al
	testb	%al, %al
	jne	L180
	movl	$4, 12(%ebx)
	pushl	$0
	pushl	$1
	pushl	$2
	pushl	%ebx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$32, %esp
	jmp	L182
L180:
	cmpb	$1, %al
	je	L181
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L182
L181:
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
L182:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZL11Set_AddressP10usb_device,"x"
__ZL11Set_AddressP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	cmpl	$5, 12(%ebx)
	je	L188
	cmpb	$0, 56(%ebx)
	js	L188
	cmpw	$127, 58(%ebx)
	ja	L188
	cmpw	$0, 60(%ebx)
	jne	L188
	cmpw	$0, 62(%ebx)
	jne	L188
	movb	58(%ebx), %al
	movb	%al, 2(%ebx)
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	movb	$1, 8(%ebx)
	addl	$16, %esp
	cmpb	$1, 2(%ebx)
	sbbl	%eax, %eax
	addl	$4, %eax
	movl	%eax, 12(%ebx)
L188:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section .text
LC5:
	.ascii "%s: IRQ init fail.\12\0"
	.p2align 2
LC6:
	.ascii "%s: Alloc endpoint buffers error!!\12\0"
	.section	.text.usb_Init,"x"
.globl _usb_Init
_usb_Init:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$20, %esp
	movl	8(%ebp), %ebx
	movb	$1, %al
	cmpl	$1, 20(%ebx)
	je	L191
	call	__ZL11USB_Connectv
	call	_irq_Init
	testb	%al, %al
	jne	L192
	subl	$8, %esp
	pushl	$__ZZ8usb_InitE12__FUNCTION__
	pushl	$LC5
	call	__ZL9err_printPcz
	call	_io_Close
	movb	$0, %al
	addl	$16, %esp
	jmp	L191
L192:
	cmpb	$0, __ZZ8usb_InitE6locked
	jne	L193
	subl	$8, %esp
	movl	$__ZL13SetEPnDLR_endv, %eax
	subl	$__ZL9SetEPnDLRP10usb_deviceiim, %eax
	pushl	%eax
	pushl	$__ZL9SetEPnDLRP10usb_deviceiim
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL15Set_Address_endv, %eax
	subl	$__ZL11Set_AddressP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL11Set_AddressP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL18Get_Descriptor_endv, %eax
	subl	$__ZL14Get_DescriptorP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL14Get_DescriptorP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL18Set_Descriptor_endv, %eax
	subl	$__ZL14Set_DescriptorP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL14Set_DescriptorP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL21Get_Configuration_endv, %eax
	subl	$__ZL17Get_ConfigurationP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL17Get_ConfigurationP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL21Set_Configuration_endv, %eax
	subl	$__ZL17Set_ConfigurationP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL17Set_ConfigurationP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL17Get_Interface_endv, %eax
	subl	$__ZL13Get_InterfaceP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL13Get_InterfaceP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL17Set_Interface_endv, %eax
	subl	$__ZL13Set_InterfaceP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL13Set_InterfaceP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL15Synch_Frame_endv, %eax
	subl	$__ZL11Synch_FrameP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL11Synch_FrameP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL24USB_Standard_Request_endv, %eax
	subl	$__ZL20USB_Standard_RequestP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL20USB_Standard_RequestP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL19Set_Line_Coding_endv, %eax
	subl	$__ZL15Set_Line_CodingP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL15Set_Line_CodingP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL19Get_Line_Coding_endv, %eax
	subl	$__ZL15Get_Line_CodingP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL15Get_Line_CodingP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL26Set_Control_Line_State_endv, %eax
	subl	$__ZL22Set_Control_Line_StateP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL22Set_Control_Line_StateP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL14Send_Break_endv, %eax
	subl	$__ZL10Send_BreakP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL10Send_BreakP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL19USB_CDC_Request_endv, %eax
	subl	$__ZL15USB_CDC_RequestP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL15USB_CDC_RequestP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL20EP0_SetupHandler_endv, %eax
	subl	$__ZL16EP0_SetupHandlerP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL16EP0_SetupHandlerP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL17EP0_InHandler_endv, %eax
	subl	$__ZL13EP0_InHandlerP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL13EP0_InHandlerP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL18EP0_OutHandler_endv, %eax
	subl	$__ZL14EP0_OutHandlerP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL14EP0_OutHandlerP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL17EP1_InHandler_endv, %eax
	subl	$__ZL13EP1_InHandlerP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL13EP1_InHandlerP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL17EP2_InHandler_endv, %eax
	subl	$__ZL13EP2_InHandlerP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL13EP2_InHandlerP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL18EP2_OutHandler_endv, %eax
	subl	$__ZL14EP2_OutHandlerP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL14EP2_OutHandlerP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL13usb_Reset_endv, %eax
	subl	$__ZL9usb_ResetP10usb_device, %eax
	pushl	%eax
	pushl	$__ZL9usb_ResetP10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL11USB_ISR_endv, %eax
	subl	$__ZL7USB_ISRiPv, %eax
	pushl	%eax
	pushl	$__ZL7USB_ISRiPv
	call	_dpmi_LockCode
	addl	$8, %esp
	pushl	$18
	pushl	$_desc_Device
	call	_dpmi_LockData
	addl	$8, %esp
	pushl	$67
	pushl	$_desc_Config_Set
	call	_dpmi_LockData
	addl	$8, %esp
	pushl	$514
	pushl	$_StringDescTable
	call	_dpmi_LockData
	addl	$8, %esp
	pushl	$514
	pushl	$_StringDescTable+514
	call	_dpmi_LockData
	addl	$8, %esp
	pushl	$514
	pushl	$_StringDescTable+1028
	call	_dpmi_LockData
	addl	$8, %esp
	pushl	$514
	pushl	$_StringDescTable+1542
	call	_dpmi_LockData
	addl	$8, %esp
	pushl	$514
	pushl	$_StringDescTable+2056
	call	_dpmi_LockData
	addl	$8, %esp
	pushl	$514
	pushl	$_StringDescTable+2570
	call	_dpmi_LockData
	addl	$8, %esp
	pushl	$514
	pushl	$_StringDescTable+3084
	call	_dpmi_LockData
	movb	$1, __ZZ8usb_InitE6locked
	addl	$16, %esp
L193:
	subl	$4, %esp
	pushl	$2
	movl	80(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
L194:
	subl	$8, %esp
	movl	80(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$16, %esp
	testb	$2, %al
	jne	L194
	subl	$12, %esp
	pushl	$8
	call	_ker_Malloc
	movl	%eax, 112(%ebx)
	addl	$16, %esp
	testl	%eax, %eax
	je	L195
	subl	$12, %esp
	pushl	$8
	call	_ker_Malloc
	movl	%eax, 116(%ebx)
	addl	$16, %esp
	testl	%eax, %eax
	je	L196
	subl	$12, %esp
	pushl	$8
	call	_ker_Malloc
	movl	%eax, 120(%ebx)
	addl	$16, %esp
	testl	%eax, %eax
	je	L197
	subl	$12, %esp
	pushl	$8
	call	_ker_Malloc
	movl	%eax, 160(%ebx)
	addl	$16, %esp
	testl	%eax, %eax
	je	L198
	subl	$12, %esp
	pushl	$64
	call	_ker_Malloc
	movl	%eax, 204(%ebx)
	addl	$16, %esp
	testl	%eax, %eax
	je	L199
	subl	$12, %esp
	pushl	$64
	call	_ker_Malloc
	movl	%eax, 208(%ebx)
	addl	$16, %esp
	testl	%eax, %eax
	je	L200
	subl	$8, %esp
	leal	-8(%ebp), %eax
	pushl	%eax
	pushl	$10
	call	___dpmi_allocate_dos_memory
	addl	$16, %esp
	cmpl	$-1, %eax
	je	L201
	sall	$4, %eax
	movl	%eax, __ZL8dma_addr
	movl	-8(%ebp), %eax
	movl	%eax, __ZL10dma_handle
	cmpl	$-1, %eax
	je	L200
	movl	__ZL8dma_addr, %edx
	movl	%edx, 124(%ebx)
	movl	__ZL8dma_addr, %eax
	addl	$8, %eax
	movl	%eax, 128(%ebx)
	movl	__ZL8dma_addr, %eax
	addl	$16, %eax
	movl	%eax, 132(%ebx)
	movl	__ZL8dma_addr, %eax
	addl	$24, %eax
	movl	%eax, 172(%ebx)
	movl	__ZL8dma_addr, %eax
	addl	$32, %eax
	movl	%eax, 216(%ebx)
	movl	__ZL8dma_addr, %eax
	addl	$96, %eax
	movl	%eax, 220(%ebx)
	movl	104(%ebx), %eax
	andl	$65535, %eax
	call	__ZL9io_outpdwtm
	movl	128(%ebx), %edx
	movw	106(%ebx), %ax
	andl	$65535, %eax
	call	__ZL9io_outpdwtm
	movl	132(%ebx), %edx
	movl	108(%ebx), %eax
	andl	$65535, %eax
	call	__ZL9io_outpdwtm
	movl	172(%ebx), %edx
	movw	150(%ebx), %ax
	andl	$65535, %eax
	call	__ZL9io_outpdwtm
	movl	216(%ebx), %edx
	movw	194(%ebx), %ax
	andl	$65535, %eax
	call	__ZL9io_outpdwtm
	movl	220(%ebx), %edx
	movl	196(%ebx), %eax
	andl	$65535, %eax
	call	__ZL9io_outpdwtm
	subl	$4, %esp
	pushl	$8200
	movl	92(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	addl	$12, %esp
	pushl	$14344
	movw	138(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	addl	$12, %esp
	pushl	$12352
	movw	182(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	addl	$12, %esp
	pushl	$12352
	movl	184(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	pushl	$-2147483648
	pushl	$2
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$32, %esp
	movw	78(%ebx), %ax
	andl	$65535, %eax
	movl	$128, %edx
	call	__ZL8io_outpbth
L202:
	subl	$8, %esp
	movw	78(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$16, %esp
	testb	%al, %al
	jns	L202
	movw	86(%ebx), %ax
	andl	$65535, %eax
	movl	$-1, %edx
	call	__ZL9io_outpdwtm
	call	_io_DisableINT
	movl	80(%ebx), %eax
	andl	$65535, %eax
	call	__ZL7io_inpbt
	movl	%eax, %edx
	andl	$254, %edx
	movl	80(%ebx), %eax
	andl	$65535, %eax
	call	__ZL8io_outpbth
	subl	$8, %esp
	pushl	$1
	pushl	4(%ebx)
	call	_irq_Setting
	addl	$12, %esp
	pushl	%ebx
	pushl	$__ZL7USB_ISRiPv
	pushl	4(%ebx)
	call	_irq_InstallISR
	movl	84(%ebx), %eax
	andl	$65535, %eax
	movl	$30495, %edx
	call	__ZL9io_outpdwtm
	movl	80(%ebx), %eax
	andl	$65535, %eax
	call	__ZL7io_inpbt
	orl	$1, %eax
	movl	$0, %edx
	movb	%al, %dl
	movl	80(%ebx), %eax
	andl	$65535, %eax
	call	__ZL8io_outpbth
	call	_io_RestoreINT
	movl	$2, 12(%ebx)
	movl	$1, 20(%ebx)
	movb	$1, %al
	addl	$16, %esp
	jmp	L191
L200:
	subl	$12, %esp
	pushl	204(%ebx)
	call	_ker_Mfree
	addl	$16, %esp
L199:
	subl	$12, %esp
	pushl	160(%ebx)
	call	_ker_Mfree
	addl	$16, %esp
L198:
	subl	$12, %esp
	pushl	120(%ebx)
	call	_ker_Mfree
	addl	$16, %esp
L197:
	subl	$12, %esp
	pushl	116(%ebx)
	call	_ker_Mfree
	addl	$16, %esp
L196:
	subl	$12, %esp
	pushl	112(%ebx)
	call	_ker_Mfree
	addl	$16, %esp
L195:
	subl	$8, %esp
	pushl	$__ZZ8usb_InitE12__FUNCTION__
	pushl	$LC6
	call	__ZL9err_printPcz
	call	_irq_Close
	call	_io_Close
	movb	$0, %al
	addl	$16, %esp
L191:
	movl	-4(%ebp), %ebx
	leave
	ret
L201:
	movl	$-1, __ZL10dma_handle
	jmp	L200
	.section	.text._ZL15Set_Line_CodingP10usb_device,"x"
__ZL15Set_Line_CodingP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %esi
	cmpb	$0, 56(%esi)
	js	L210
	leal	64(%esi), %eax
	movl	%eax, 40(%esi)
	movw	62(%esi), %ax
	movw	%ax, 46(%esi)
	movb	$1, 29(%esi)
	subl	$4, %esp
	pushl	$-2147483640
	movw	102(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movl	$7, 12(%esi)
	addl	$4, %esp
	pushl	52(%esi)
	call	_QueueEmpty
	addl	$16, %esp
	testb	%al, %al
	jne	L210
	cmpb	$0, 30(%esi)
	jne	L210
	movl	52(%esi), %eax
	cmpl	$0, 8(%eax)
	je	L210
	call	__ZL9TX_LED_ONv
	movl	$0, %edi
L208:
	movl	%edi, %ebx
	addl	204(%esi), %ebx
	subl	$12, %esp
	pushl	52(%esi)
	call	_PopQueue
	movb	%al, (%ebx)
	incl	%edi
	movl	52(%esi), %eax
	addl	$16, %esp
	cmpl	$0, 8(%eax)
	je	L211
	cmpl	$63, %edi
	jle	L208
L211:
	subl	$4, %esp
	pushl	216(%esi)
	pushl	$64
	pushl	204(%esi)
	call	_dosmemput
	movl	%edi, %eax
	orl	$-2147483648, %eax
	pushl	%eax
	pushl	$0
	pushl	$2
	pushl	%esi
	call	__ZL9SetEPnDLRP10usb_deviceiim
	movb	$1, 30(%esi)
	addl	$32, %esp
L210:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZL22Set_Control_Line_StateP10usb_device,"x"
__ZL22Set_Control_Line_StateP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %edx
	cmpb	$0, 56(%edx)
	js	L215
	movw	58(%edx), %ax
	movw	%ax, 72(%edx)
	subl	$4, %esp
	pushl	$-1610612736
	movl	100(%edx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L215:
	leave
	ret
	.section	.text._ZL10Send_BreakP10usb_device,"x"
__ZL10Send_BreakP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %edx
	cmpb	$0, 56(%edx)
	js	L218
	movw	58(%edx), %ax
	movw	%ax, 76(%edx)
	subl	$4, %esp
	pushl	$-1610612736
	movl	100(%edx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L218:
	leave
	ret
	.section	.text._ZL13EP1_InHandlerP10usb_device,"x"
__ZL13EP1_InHandlerP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	call	__ZL9TX_LED_ONv
	cmpb	$0, __ZZL13EP1_InHandlerP10usb_deviceE9Completed
	je	L220
	movl	$__ZZL13EP1_InHandlerP10usb_deviceE12notification, 160(%ebx)
	subl	$4, %esp
	pushl	172(%ebx)
	pushl	$8
	pushl	$__ZZL13EP1_InHandlerP10usb_deviceE12notification
	call	_dosmemput
	addl	$12, %esp
	pushl	$-2147483640
	movl	144(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	$0, __ZZL13EP1_InHandlerP10usb_deviceE9Completed
	addl	$16, %esp
	jmp	L222
L220:
	movl	160(%ebx), %edx
	movw	74(%ebx), %ax
	movb	%al, (%edx)
	movl	160(%ebx), %edx
	movl	$0, %eax
	movb	75(%ebx), %al
	movb	%al, 1(%edx)
	subl	$4, %esp
	pushl	172(%ebx)
	pushl	$8
	pushl	160(%ebx)
	call	_dosmemput
	addl	$12, %esp
	pushl	$-2147483646
	movl	144(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	$1, __ZZL13EP1_InHandlerP10usb_deviceE9Completed
	addl	$16, %esp
L222:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZL14EP2_OutHandlerP10usb_device,"x"
__ZL14EP2_OutHandlerP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %esi
	call	__ZL9RX_LED_ONv
	subl	$8, %esp
	movw	190(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	movl	%eax, %ebx
	addl	$12, %esp
	pushl	208(%esi)
	pushl	$64
	pushl	220(%esi)
	call	_dosmemget
	movl	%ebx, %edi
	andl	$65535, %edi
	addl	$16, %esp
	testl	%edi, %edi
	jle	L224
	movl	$0, %ebx
L225:
	subl	$8, %esp
	movl	208(%esi), %eax
	movb	(%eax,%ebx), %al
	andl	$255, %eax
	pushl	%eax
	pushl	48(%esi)
	call	_PushQueue
	incl	%ebx
	addl	$16, %esp
	cmpl	%edi, %ebx
	jl	L225
L224:
	movl	48(%esi), %edx
	movl	12(%edx), %eax
	subl	$1024, %eax
	cmpl	%eax, 8(%edx)
	jae	L227
	subl	$4, %esp
	pushl	$-2147483584
	movw	190(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L227:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZL9usb_ResetP10usb_device,"x"
__ZL9usb_ResetP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	movb	$0, %al
	cmpl	$0, 20(%ebx)
	je	L231
	movb	$0, 2(%ebx)
	movb	$0, 8(%ebx)
	subl	$4, %esp
	pushl	$128
	movw	78(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	pushl	$-2147483648
	movw	98(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$12, %esp
	pushl	$0
	movl	100(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$12, %esp
	pushl	$0
	movw	102(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$12, %esp
	pushl	$0
	movl	144(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$12, %esp
	pushl	$0
	movw	190(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$12, %esp
	pushl	$0
	movl	188(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$4, %esp
	pushl	48(%ebx)
	call	_ClearQueue
	addl	$4, %esp
	pushl	52(%ebx)
	call	_ClearQueue
	movl	$3, 12(%ebx)
	movb	$1, %al
	addl	$16, %esp
L231:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text.usb_Read,"x"
.globl _usb_Read
_usb_Read:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %ebx
	testl	%ebx, %ebx
	jne	L234
	subl	$8, %esp
	pushl	$__ZZ8usb_ReadE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movl	$65535, %edx
	addl	$16, %esp
	jmp	L235
L234:
	cmpl	$-1, 32(%ebx)
	je	L236
	call	_timer_nowtime
	movl	%eax, %esi
L239:
	movl	48(%ebx), %eax
	cmpl	$0, 8(%eax)
	jne	L237
	call	_timer_nowtime
	subl	%esi, %eax
	cmpl	32(%ebx), %eax
	jae	L238
	jmp	L239
L236:
	movl	48(%ebx), %eax
	cmpl	$0, 8(%eax)
	je	L240
L237:
	call	_io_DisableINT
	subl	$12, %esp
	pushl	48(%ebx)
	call	_PopQueue
	movl	%eax, %esi
	call	_io_RestoreINT
	addl	$8, %esp
	movw	190(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	addl	$16, %esp
	testl	%eax, %eax
	js	L241
	movl	48(%ebx), %edx
	movl	12(%edx), %eax
	subl	$1024, %eax
	cmpl	%eax, 8(%edx)
	jae	L241
	subl	$4, %esp
	pushl	$-2147483584
	movw	190(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L241:
	movl	%esi, %edx
	andl	$255, %edx
L235:
	movl	%edx, %eax
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
L238:
	movl	48(%ebx), %eax
	movl	$65535, %edx
	cmpl	$0, 8(%eax)
	je	L235
	jmp	L237
L240:
L243:
	jmp	L243
	.section	.text.usb_Receive,"x"
.globl _usb_Receive
_usb_Receive:
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
	jle	L246
L249:
	subl	$12, %esp
	pushl	8(%ebp)
	call	_usb_Read
	addl	$16, %esp
	cmpl	$65535, %eax
	je	L246
	movb	%al, (%edi,%ebx)
	incl	%ebx
	cmpl	%ebx, %esi
	jg	L249
L246:
	movl	%ebx, %eax
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZL13EP2_InHandlerP10usb_device,"x"
__ZL13EP2_InHandlerP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %edi
	cmpb	$0, 30(%edi)
	jne	L255
	movl	52(%edi), %eax
	cmpl	$0, 8(%eax)
	je	L255
	call	__ZL9TX_LED_ONv
	movl	$0, %esi
L253:
	movl	%esi, %ebx
	addl	204(%edi), %ebx
	subl	$12, %esp
	pushl	52(%edi)
	call	_PopQueue
	movb	%al, (%ebx)
	incl	%esi
	movl	52(%edi), %eax
	addl	$16, %esp
	cmpl	$0, 8(%eax)
	je	L256
	cmpl	$63, %esi
	jle	L253
L256:
	subl	$4, %esp
	pushl	216(%edi)
	pushl	$64
	pushl	204(%edi)
	call	_dosmemput
	addl	$12, %esp
	movl	%esi, %eax
	orl	$-2147483648, %eax
	pushl	%eax
	movl	188(%edi), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	$1, 30(%edi)
	addl	$16, %esp
L255:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZL14Set_DescriptorP10usb_device,"x"
__ZL14Set_DescriptorP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %ecx
	cmpb	$0, 56(%ecx)
	js	L265
	movb	59(%ecx), %al
	cmpb	$2, %al
	je	L262
	cmpb	$3, %al
	je	L263
	cmpb	$1, %al
	jne	L266
	cmpb	$0, 58(%ecx)
	jne	L265
	cmpw	$0, 60(%ecx)
	jne	L265
	movl	$_desc_Device, 40(%ecx)
	movw	62(%ecx), %ax
	movw	%ax, 46(%ecx)
	movb	$1, 29(%ecx)
	pushl	$-2147483640
	pushl	$1
	pushl	$0
	pushl	%ecx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L265
L262:
	cmpb	$0, 58(%ecx)
	jne	L265
	cmpw	$0, 60(%ecx)
	jne	L265
	movl	$_desc_Config_Set, 40(%ecx)
	movw	62(%ecx), %ax
	movw	%ax, 46(%ecx)
	movb	$1, 29(%ecx)
	pushl	$-2147483640
	pushl	$1
	pushl	$0
	pushl	%ecx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L265
L263:
	movb	58(%ecx), %dl
	cmpb	$6, %dl
	ja	L264
	movl	$0, %eax
	movb	%dl, %al
	movl	%eax, %edx
	sall	$9, %edx
	leal	_StringDescTable(%edx,%eax,2), %eax
	movl	%eax, 40(%ecx)
	movw	62(%ecx), %ax
	movw	%ax, 46(%ecx)
	movb	$1, 29(%ecx)
	pushl	$-2147483640
	pushl	$1
	pushl	$0
	pushl	%ecx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L265
L264:
	pushl	$-1073741824
	pushl	$1
	pushl	$0
	pushl	%ecx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L265
L266:
	subl	$4, %esp
	pushl	$-1073741824
	movw	102(%ecx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L265:
	leave
	ret
	.section	.text.usb_SetSerialState,"x"
.globl _usb_SetSerialState
_usb_SetSerialState:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	movl	12(%ebp), %eax
	movw	%ax, 74(%ebx)
	call	__ZL9TX_LED_ONv
	cmpb	$0, __ZZL13EP1_InHandlerP10usb_deviceE9Completed
	je	L268
	movl	$__ZZL13EP1_InHandlerP10usb_deviceE12notification, 160(%ebx)
	subl	$4, %esp
	pushl	172(%ebx)
	pushl	$8
	pushl	$__ZZL13EP1_InHandlerP10usb_deviceE12notification
	call	_dosmemput
	addl	$12, %esp
	pushl	$-2147483640
	movl	144(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	$0, __ZZL13EP1_InHandlerP10usb_deviceE9Completed
	addl	$16, %esp
	jmp	L270
L268:
	movl	160(%ebx), %edx
	movw	74(%ebx), %ax
	movb	%al, (%edx)
	movl	160(%ebx), %edx
	movl	$0, %eax
	movb	75(%ebx), %al
	movb	%al, 1(%edx)
	subl	$4, %esp
	pushl	172(%ebx)
	pushl	$8
	pushl	160(%ebx)
	call	_dosmemput
	addl	$12, %esp
	pushl	$-2147483646
	movl	144(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	$1, __ZZL13EP1_InHandlerP10usb_deviceE9Completed
	addl	$16, %esp
L270:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZL13EP0_InHandlerP10usb_device,"x"
__ZL13EP0_InHandlerP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %edi
	cmpb	$0, 28(%edi)
	je	L279
	call	__ZL9TX_LED_ONv
	movl	__ZZL13EP0_InHandlerP10usb_deviceE1j, %ebx
	movl	44(%edi), %eax
	andl	$65535, %eax
	movl	$0, %esi
	cmpl	%ebx, %eax
	jg	L280
	jmp	L283
L277:
	movl	36(%edi), %eax
	movl	116(%edi), %ecx
	movb	(%eax), %dl
	movb	%dl, (%ecx,%esi)
	incl	%eax
	movl	%eax, 36(%edi)
	incl	%esi
	incl	%ebx
	cmpl	$8, %esi
	jne	L280
	movl	%ebx, __ZZL13EP0_InHandlerP10usb_deviceE1j
	jmp	L276
L280:
	movl	44(%edi), %eax
	andl	$65535, %eax
	cmpl	%eax, %ebx
	jl	L277
	movl	%ebx, __ZZL13EP0_InHandlerP10usb_deviceE1j
L276:
	subl	$4, %esp
	pushl	128(%edi)
	pushl	$8
	pushl	116(%edi)
	call	_dosmemput
	addl	$12, %esp
	movl	%esi, %eax
	orl	$-2147483648, %eax
	pushl	%eax
	movl	100(%edi), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
	jmp	L279
L283:
	movw	62(%edi), %ax
	andl	$65535, %eax
	cmpl	%eax, %ebx
	jge	L278
	testb	$7, %bl
	jne	L278
	pushl	$-2147483648
	pushl	$0
	pushl	$0
	pushl	%edi
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
L278:
	movl	$0, __ZZL13EP0_InHandlerP10usb_deviceE1j
	movw	$0, 44(%edi)
	movb	$0, 28(%edi)
	subl	$4, %esp
	pushl	$-1610612736
	movw	102(%edi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L279:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZL11Synch_FrameP10usb_device,"x"
__ZL11Synch_FrameP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$20, %esp
	movl	8(%ebp), %ebx
	cmpb	$0, 56(%ebx)
	jns	L288
	cmpw	$0, 58(%ebx)
	jne	L288
	cmpw	$2, 62(%ebx)
	jne	L288
	movl	12(%ebx), %eax
	cmpl	$4, %eax
	je	L286
	cmpl	$5, %eax
	jne	L288
	jmp	L289
L286:
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L288
L289:
	subl	$8, %esp
	movw	82(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In16
	andl	$4095, %eax
	movw	%ax, -6(%ebp)
	leal	-6(%ebp), %eax
	movl	%eax, 36(%ebx)
	movw	$2, 44(%ebx)
	movb	$1, 28(%ebx)
	movl	%ebx, (%esp)
	call	__ZL13EP0_InHandlerP10usb_device
	addl	$16, %esp
L288:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZL13Get_InterfaceP10usb_device,"x"
__ZL13Get_InterfaceP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	cmpb	$0, 56(%eax)
	jns	L297
	cmpw	$0, 58(%eax)
	jne	L297
	cmpw	$1, 62(%eax)
	jne	L297
	movl	60(%eax), %edx
	testw	%dx, %dx
	je	L293
	cmpw	$1, %dx
	jne	L298
	jmp	L299
L293:
	movl	12(%eax), %edx
	cmpl	$4, %edx
	jne	L295
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%eax
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L297
L295:
	cmpl	$5, %edx
	jne	L297
	movl	$_desc_Config_Set+11, 36(%eax)
	movw	$1, 44(%eax)
	movb	$1, 28(%eax)
	subl	$12, %esp
	pushl	%eax
	call	__ZL13EP0_InHandlerP10usb_device
	addl	$16, %esp
	jmp	L297
L299:
	movl	12(%eax), %edx
	cmpl	$4, %edx
	jne	L296
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%eax
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L297
L296:
	cmpl	$5, %edx
	jne	L297
	movl	$_desc_Config_Set+46, 36(%eax)
	movw	$1, 44(%eax)
	movb	$1, 28(%eax)
	subl	$12, %esp
	pushl	%eax
	call	__ZL13EP0_InHandlerP10usb_device
	addl	$16, %esp
	jmp	L297
L298:
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%eax
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
L297:
	leave
	ret
	.section	.text._ZL17Get_ConfigurationP10usb_device,"x"
__ZL17Get_ConfigurationP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	cmpb	$0, 56(%eax)
	jns	L302
	cmpw	$0, 58(%eax)
	jne	L302
	cmpw	$0, 60(%eax)
	jne	L302
	cmpw	$1, 62(%eax)
	jne	L302
	movl	$_desc_Config_Set+5, 36(%eax)
	movw	$1, 44(%eax)
	movb	$1, 28(%eax)
	subl	$12, %esp
	pushl	%eax
	call	__ZL13EP0_InHandlerP10usb_device
	addl	$16, %esp
L302:
	leave
	ret
	.section	.text._ZL14Get_DescriptorP10usb_device,"x"
__ZL14Get_DescriptorP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %ecx
	cmpb	$0, 56(%ecx)
	jns	L314
	movb	59(%ecx), %al
	cmpb	$2, %al
	je	L307
	cmpb	$3, %al
	je	L308
	cmpb	$1, %al
	jne	L315
	cmpb	$0, 58(%ecx)
	jne	L314
	cmpw	$0, 60(%ecx)
	jne	L314
	movl	$_desc_Device, 36(%ecx)
	movl	$0, %eax
	movb	_desc_Device, %al
	movw	%ax, 44(%ecx)
	movb	$1, 28(%ecx)
	movw	62(%ecx), %dx
	cmpw	%dx, %ax
	jbe	L309
	movw	%dx, 44(%ecx)
L309:
	cmpl	$0, 24(%ecx)
	jne	L310
	movw	$8, 44(%ecx)
	movl	$1, 24(%ecx)
L310:
	cmpb	$0, 28(%ecx)
	je	L314
	subl	$12, %esp
	pushl	%ecx
	call	__ZL13EP0_InHandlerP10usb_device
	addl	$16, %esp
	jmp	L314
L307:
	cmpb	$0, 58(%ecx)
	jne	L314
	cmpw	$0, 60(%ecx)
	jne	L314
	movl	$_desc_Config_Set, 36(%ecx)
	movl	$0, %eax
	movb	_desc_Config_Set+3, %al
	sall	$8, %eax
	movl	$0, %edx
	movb	_desc_Config_Set+2, %dl
	addl	%edx, %eax
	movw	%ax, 44(%ecx)
	movb	$1, 28(%ecx)
	movw	62(%ecx), %dx
	cmpw	%dx, %ax
	jbe	L311
	movw	%dx, 44(%ecx)
	cmpb	$0, 28(%ecx)
	je	L314
L311:
	subl	$12, %esp
	pushl	%ecx
	call	__ZL13EP0_InHandlerP10usb_device
	addl	$16, %esp
	jmp	L314
L308:
	movb	58(%ecx), %dl
	cmpb	$6, %dl
	ja	L312
	movl	$0, %eax
	movb	%dl, %al
	movl	%eax, %edx
	sall	$9, %edx
	leal	(%edx,%eax,2), %eax
	leal	_StringDescTable(%eax), %edx
	movl	%edx, 36(%ecx)
	movb	_StringDescTable(%eax), %al
	andl	$255, %eax
	movw	%ax, 44(%ecx)
	movb	$1, 28(%ecx)
	movw	62(%ecx), %dx
	cmpw	%dx, %ax
	jbe	L313
	movw	%dx, 44(%ecx)
	cmpb	$0, 28(%ecx)
	je	L314
L313:
	subl	$12, %esp
	pushl	%ecx
	call	__ZL13EP0_InHandlerP10usb_device
	addl	$16, %esp
	jmp	L314
L312:
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%ecx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L314
L315:
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%ecx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
L314:
	leave
	ret
	.section	.text._ZL14EP0_OutHandlerP10usb_device,"x"
__ZL14EP0_OutHandlerP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %esi
	cmpb	$0, 29(%esi)
	je	L323
	call	__ZL9RX_LED_ONv
	subl	$8, %esp
	movw	102(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	movl	%eax, %ebx
	movw	46(%esi), %ax
	andl	$65535, %eax
	addl	$16, %esp
	cmpl	__ZZL14EP0_OutHandlerP10usb_deviceE1j, %eax
	jle	L318
	subl	$4, %esp
	pushl	120(%esi)
	pushl	$8
	pushl	132(%esi)
	call	_dosmemget
	movl	%ebx, %edi
	andl	$65535, %edi
	addl	$16, %esp
	testl	%edi, %edi
	jle	L319
	movl	__ZZL14EP0_OutHandlerP10usb_deviceE1j, %ebx
	movw	46(%esi), %ax
	andl	$65535, %eax
	cmpl	%ebx, %eax
	jle	L318
	movl	$0, %ecx
L321:
	movl	40(%esi), %eax
	movl	120(%esi), %edx
	movb	(%edx,%ecx), %dl
	movb	%dl, (%eax)
	incl	%eax
	movl	%eax, 40(%esi)
	incl	%ecx
	incl	%ebx
	cmpl	%edi, %ecx
	jge	L320
	movw	46(%esi), %ax
	andl	$65535, %eax
	cmpl	%ebx, %eax
	jg	L321
	jmp	L318
L320:
	movl	%ebx, __ZZL14EP0_OutHandlerP10usb_deviceE1j
L319:
	movw	46(%esi), %ax
	andl	$65535, %eax
	cmpl	__ZZL14EP0_OutHandlerP10usb_deviceE1j, %eax
	jle	L318
	subl	$4, %esp
	pushl	$-2147483640
	movw	102(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
	jmp	L323
L318:
	movl	$0, __ZZL14EP0_OutHandlerP10usb_deviceE1j
	movw	$0, 46(%esi)
	movb	$0, 29(%esi)
	subl	$4, %esp
	pushl	$-1610612736
	movl	100(%esi), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L323:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text.usb_Send,"x"
.globl _usb_Send
_usb_Send:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %esi
	testl	%esi, %esi
	je	L327
	movl	$0, %edi
	cmpl	$0, 16(%ebp)
	jg	L341
	jmp	L329
L327:
	subl	$8, %esp
	pushl	$__ZZ8usb_SendE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movl	$0, %edi
	addl	$16, %esp
	jmp	L329
L341:
	movl	16(%ebp), %eax
	decl	%eax
	movl	%eax, -24(%ebp)
L346:
	cmpl	$-1, 32(%esi)
	je	L330
	call	_timer_nowtime
	movl	%eax, %ebx
L333:
	movl	52(%esi), %edx
	movl	8(%edx), %eax
	cmpl	12(%edx), %eax
	jb	L331
	call	_timer_nowtime
	subl	%ebx, %eax
	cmpl	32(%esi), %eax
	jae	L332
	jmp	L333
L351:
	cmpb	$0, 30(%esi)
	jne	L329
	cmpl	$0, 8(%edx)
	je	L329
	call	__ZL9TX_LED_ONv
	movl	$0, -20(%ebp)
L334:
	movl	-20(%ebp), %ebx
	addl	204(%esi), %ebx
	subl	$12, %esp
	pushl	52(%esi)
	call	_PopQueue
	movb	%al, (%ebx)
	incl	-20(%ebp)
	movl	52(%esi), %eax
	addl	$16, %esp
	cmpl	$0, 8(%eax)
	je	L344
	cmpl	$63, -20(%ebp)
	jle	L334
L344:
	subl	$4, %esp
	pushl	216(%esi)
	pushl	$64
	pushl	204(%esi)
	call	_dosmemput
	movl	-20(%ebp), %eax
	orl	$-2147483648, %eax
	pushl	%eax
	pushl	$0
	pushl	$2
	pushl	%esi
	call	__ZL9SetEPnDLRP10usb_deviceiim
	movb	$1, 30(%esi)
	addl	$32, %esp
	jmp	L329
L330:
	movl	52(%esi), %edx
	movl	8(%edx), %eax
	cmpl	12(%edx), %eax
	jae	L336
L331:
	call	_io_DisableINT
	subl	$8, %esp
	movl	12(%ebp), %edx
	movl	$0, %eax
	movb	(%edx,%edi), %al
	pushl	%eax
	pushl	52(%esi)
	call	_PushQueue
	addl	$16, %esp
	cmpl	%edi, -24(%ebp)
	je	L337
	movl	52(%esi), %edx
	movl	8(%edx), %eax
	cmpl	12(%edx), %eax
	jb	L338
L337:
	cmpl	$7, 12(%esi)
	jne	L338
	cmpb	$0, 30(%esi)
	jne	L338
	movl	52(%esi), %eax
	cmpl	$0, 8(%eax)
	je	L338
	call	__ZL9TX_LED_ONv
	movl	$0, -16(%ebp)
L339:
	movl	-16(%ebp), %ebx
	addl	204(%esi), %ebx
	subl	$12, %esp
	pushl	52(%esi)
	call	_PopQueue
	movb	%al, (%ebx)
	incl	-16(%ebp)
	movl	52(%esi), %eax
	addl	$16, %esp
	cmpl	$0, 8(%eax)
	je	L345
	cmpl	$63, -16(%ebp)
	jle	L339
L345:
	subl	$4, %esp
	pushl	216(%esi)
	pushl	$64
	pushl	204(%esi)
	call	_dosmemput
	addl	$12, %esp
	movl	-16(%ebp), %eax
	orl	$-2147483648, %eax
	pushl	%eax
	movl	188(%esi), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	$1, 30(%esi)
	addl	$16, %esp
L338:
	call	_io_RestoreINT
	incl	%edi
	cmpl	%edi, 16(%ebp)
	jg	L346
L329:
	movl	%edi, %eax
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
L332:
	movl	52(%esi), %edx
	movl	8(%edx), %eax
	cmpl	12(%edx), %eax
	jb	L331
	jmp	L351
L336:
L350:
	jmp	L350
	.section	.text.usb_Write,"x"
.globl _usb_Write
_usb_Write:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	12(%ebp), %eax
	movb	%al, -4(%ebp)
	pushl	$1
	leal	-4(%ebp), %eax
	pushl	%eax
	pushl	8(%ebp)
	call	_usb_Send
	addl	$16, %esp
	cmpl	$1, %eax
	sete	%al
	leave
	ret
	.section	.text._ZL13Set_InterfaceP10usb_device,"x"
__ZL13Set_InterfaceP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %edx
	cmpb	$0, 56(%edx)
	js	L361
	cmpw	$0, 62(%edx)
	jne	L361
	movl	60(%edx), %eax
	testw	%ax, %ax
	je	L357
	cmpw	$1, %ax
	jne	L362
	jmp	L363
L357:
	movl	12(%edx), %eax
	cmpl	$4, %eax
	jne	L359
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%edx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L361
L359:
	cmpl	$5, %eax
	jne	L361
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%edx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L361
L363:
	movl	12(%edx), %eax
	cmpl	$4, %eax
	jne	L360
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%edx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L361
L360:
	cmpl	$5, %eax
	jne	L361
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%edx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L361
L362:
	subl	$4, %esp
	pushl	$-1073741824
	movl	100(%edx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L361:
	leave
	ret
	.section	.text._ZL16EP0_SetupHandlerP10usb_device,"x"
__ZL16EP0_SetupHandlerP10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$8, %esp
	movl	8(%ebp), %ebx
	pushl	112(%ebx)
	pushl	$8
	pushl	124(%ebx)
	call	_dosmemget
	movl	112(%ebx), %ecx
	movb	(%ecx), %al
	movb	%al, 56(%ebx)
	movb	1(%ecx), %al
	movb	%al, 57(%ebx)
	movl	$0, %eax
	movb	3(%ecx), %al
	sall	$8, %eax
	movl	$0, %edx
	movb	2(%ecx), %dl
	addl	%edx, %eax
	movw	%ax, 58(%ebx)
	movl	$0, %eax
	movb	5(%ecx), %al
	sall	$8, %eax
	movl	$0, %edx
	movb	4(%ecx), %dl
	addl	%edx, %eax
	movw	%ax, 60(%ebx)
	movl	$0, %eax
	movb	7(%ecx), %al
	sall	$8, %eax
	movl	$0, %edx
	movb	6(%ecx), %dl
	addl	%edx, %eax
	movw	%ax, 62(%ebx)
	addl	$16, %esp
	cmpb	$0, 16(%ebx)
	je	L365
	subl	$8, %esp
	movl	92(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	addl	$12, %esp
	andl	$57343, %eax
	pushl	%eax
	movl	92(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$8, %esp
	movl	92(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	addl	$12, %esp
	orb	$32, %ah
	pushl	%eax
	movl	92(%ebx), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	$0, 16(%ebx)
	addl	$16, %esp
L365:
	movb	56(%ebx), %dl
	cmpb	$33, %dl
	je	L368
	cmpb	$33, %dl
	ja	L369
	cmpb	$2, %dl
	ja	L366
	jmp	L367
L369:
	cmpb	$-128, %dl
	jb	L366
	cmpb	$-126, %dl
	jbe	L367
	cmpb	$-95, %dl
	jne	L366
	jmp	L368
L367:
	movb	57(%ebx), %al
	leal	-5(%eax), %ecx
	cmpb	$7, %cl
	ja	L366
	movl	$0, %eax
	movb	%cl, %al
	jmp	*L378(,%eax,4)
	.p2align 2
	.section	.rodata._ZL16EP0_SetupHandlerP10usb_device,""
	.p2align 2
L378:
	.long	L370
	.long	L371
	.long	L372
	.long	L373
	.long	L374
	.long	L375
	.long	L376
	.long	L377
	.section	.text._ZL16EP0_SetupHandlerP10usb_device,"x"
L370:
	cmpl	$5, 12(%ebx)
	je	L366
	testb	%dl, %dl
	js	L366
	cmpw	$127, 58(%ebx)
	ja	L366
	cmpw	$0, 60(%ebx)
	jne	L366
	cmpw	$0, 62(%ebx)
	jne	L366
	movb	58(%ebx), %al
	movb	%al, 2(%ebx)
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	movb	$1, 8(%ebx)
	addl	$16, %esp
	cmpb	$1, 2(%ebx)
	sbbl	%eax, %eax
	addl	$4, %eax
	movl	%eax, 12(%ebx)
	jmp	L366
L371:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL14Get_DescriptorP10usb_device
	addl	$16, %esp
	jmp	L366
L372:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL14Set_DescriptorP10usb_device
	addl	$16, %esp
	jmp	L366
L373:
	testb	%dl, %dl
	jns	L366
	cmpw	$0, 58(%ebx)
	jne	L366
	cmpw	$0, 60(%ebx)
	jne	L366
	cmpw	$1, 62(%ebx)
	jne	L366
	movl	$_desc_Config_Set+5, 36(%ebx)
	movw	$1, 44(%ebx)
	movb	$1, 28(%ebx)
	subl	$12, %esp
	pushl	%ebx
	call	__ZL13EP0_InHandlerP10usb_device
	addl	$16, %esp
	jmp	L366
L374:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL17Set_ConfigurationP10usb_device
	addl	$16, %esp
	jmp	L366
L375:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL13Get_InterfaceP10usb_device
	addl	$16, %esp
	jmp	L366
L376:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL13Set_InterfaceP10usb_device
	addl	$16, %esp
	jmp	L366
L377:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL11Synch_FrameP10usb_device
	addl	$16, %esp
	jmp	L366
L368:
	movb	57(%ebx), %al
	cmpb	$33, %al
	je	L382
	cmpb	$33, %al
	ja	L385
	cmpb	$32, %al
	jne	L366
	jmp	L387
L385:
	cmpb	$34, %al
	je	L383
	cmpb	$35, %al
	jne	L366
	jmp	L388
L387:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL15Set_Line_CodingP10usb_device
	addl	$16, %esp
	jmp	L366
L382:
	testb	%dl, %dl
	jns	L366
	leal	64(%ebx), %eax
	movl	%eax, 36(%ebx)
	movw	62(%ebx), %ax
	movw	%ax, 44(%ebx)
	movb	$1, 28(%ebx)
	subl	$12, %esp
	pushl	%ebx
	call	__ZL13EP0_InHandlerP10usb_device
	addl	$16, %esp
	jmp	L366
L383:
	testb	%dl, %dl
	js	L366
	movw	58(%ebx), %ax
	movw	%ax, 72(%ebx)
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
	jmp	L366
L388:
	testb	%dl, %dl
	js	L366
	movw	58(%ebx), %ax
	movw	%ax, 76(%ebx)
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRP10usb_deviceiim
	addl	$16, %esp
L366:
	subl	$4, %esp
	pushl	$-2147483648
	movw	98(%ebx), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZL7USB_ISRiPv,"x"
__ZL7USB_ISRiPv:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$20, %esp
	movl	12(%ebp), %esi
	movw	86(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	movl	%eax, -16(%ebp)
	addl	$16, %esp
	movl	$0, %eax
	cmpl	$0, -16(%ebp)
	je	L391
	testb	$1, -16(%ebp)
	je	L392
	subl	$4, %esp
	pushl	$130847
	movw	86(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
	cmpl	$0, 20(%esi)
	je	L392
	movb	$0, 2(%esi)
	movb	$0, 8(%esi)
	subl	$4, %esp
	pushl	$128
	movw	78(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	pushl	$-2147483648
	movw	98(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$12, %esp
	pushl	$0
	movl	100(%esi), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$12, %esp
	pushl	$0
	movw	102(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$12, %esp
	pushl	$0
	movl	144(%esi), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$12, %esp
	pushl	$0
	movw	190(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$12, %esp
	pushl	$0
	movl	188(%esi), %eax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$4, %esp
	pushl	48(%esi)
	call	_ClearQueue
	addl	$4, %esp
	pushl	52(%esi)
	call	_ClearQueue
	movl	$3, 12(%esi)
	addl	$16, %esp
L392:
	testb	$2, -16(%ebp)
	je	L393
	subl	$4, %esp
	pushl	$2
	movw	86(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
	cmpb	$0, 8(%esi)
	je	L394
	movb	2(%esi), %dl
	orl	$-128, %edx
	andl	$255, %edx
	movw	78(%esi), %ax
	andl	$65535, %eax
	call	__ZL8io_outpbth
	movb	$0, 8(%esi)
L394:
	movl	__ZL12tx_led_count, %eax
	testl	%eax, %eax
	je	L395
	decl	%eax
	movl	%eax, __ZL12tx_led_count
	testl	%eax, %eax
	jne	L395
	call	__ZL10TX_LED_OFFv
L395:
	movl	__ZL12rx_led_count, %eax
	testl	%eax, %eax
	je	L393
	decl	%eax
	movl	%eax, __ZL12rx_led_count
	testl	%eax, %eax
	jne	L393
	call	__ZL10RX_LED_OFFv
L393:
	testb	$4, -16(%ebp)
	je	L396
	subl	$4, %esp
	pushl	$4
	movw	86(%esi), %ax
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movl	$0, 24(%esi)
	movl	$2, 12(%esi)
	call	__ZL10TX_LED_OFFv
	call	__ZL10RX_LED_OFFv
	addl	$16, %esp
L396:
	testb	$8, -16(%ebp)
	je	L397
	subl	$4, %esp
	pushl	$8
	movw	86(