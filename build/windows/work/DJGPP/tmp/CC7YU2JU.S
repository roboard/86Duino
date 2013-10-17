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
	movw	72(%eax), %ax
L57:
	leave
	ret
	.section	.text.usb_TxReady,"x"
.globl _usb_TxReady
_usb_TxReady:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %ecx
	testl	%ecx, %ecx
	jne	L60
	subl	$8, %esp
	pushl	$__ZZ11usb_TxReadyE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L61
L60:
	movl	52(%ecx), %eax
	movl	8(%eax), %edx
	movl	52(%ecx), %eax
	movl	12(%eax), %eax
	cmpl	%eax, %edx
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
	movl	8(%eax), %eax
	testl	%eax, %eax
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
	movl	8(%ebp), %ecx
	testl	%ecx, %ecx
	jne	L68
	subl	$8, %esp
	pushl	$__ZZ15usb_TxQueueFullE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L69
L68:
	movl	52(%ecx), %eax
	movl	8(%eax), %edx
	movl	52(%ecx), %eax
	movl	12(%eax), %eax
	cmpl	%eax, %edx
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
	movl	8(%eax), %eax
	testl	%eax, %eax
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
	movl	8(%ebp), %ecx
	testl	%ecx, %ecx
	jne	L80
	subl	$8, %esp
	pushl	$__ZZ15usb_RxQueueFullE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movb	$0, %al
	addl	$16, %esp
	jmp	L81
L80:
	movl	48(%ecx), %eax
	movl	8(%eax), %edx
	movl	48(%ecx), %eax
	movl	12(%eax), %eax
	cmpl	%eax, %edx
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
	jne	L109
	subl	$8, %esp
	pushl	$__ZZ14usb_FlushWFIFOE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	addl	$16, %esp
	jmp	L107
L109:
	movl	52(%ebx), %eax
	movl	8(%eax), %eax
	testl	%eax, %eax
	jne	L109
L108:
	movw	188(%ebx), %ax
	subl	$8, %esp
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	addl	$16, %esp
	testl	%eax, %eax
	js	L108
L107:
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
	movl	52(%ebx), %eax
	subl	$12, %esp
	pushl	%eax
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
	movl	48(%ebx), %eax
	subl	$12, %esp
	pushl	%eax
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
	movl	20(%ebx), %eax
	testl	%eax, %eax
	je	L139
	call	_io_DisableINT
	movw	80(%ebx), %ax
	subl	$8, %esp
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	movw	80(%ebx), %dx
	addl	$12, %esp
	andl	$254, %eax
	pushl	%eax
	andl	$65535, %edx
	pushl	%edx
	pushl	$0
	call	_io_Out8
	movl	4(%ebx), %eax
	addl	$8, %esp
	pushl	%ebx
	pushl	%eax
	call	_irq_UninstallISR
	call	_io_RestoreINT
	call	_irq_Close
	movw	80(%ebx), %ax
	addl	$12, %esp
	pushl	$2
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
L140:
	movw	80(%ebx), %ax
	subl	$8, %esp
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
	movl	112(%ebx), %eax
	subl	$12, %esp
	pushl	%eax
	call	_ker_Mfree
	movl	116(%ebx), %eax
	movl	%eax, (%esp)
	call	_ker_Mfree
	movl	120(%ebx), %eax
	movl	%eax, (%esp)
	call	_ker_Mfree
	movl	160(%ebx), %eax
	movl	%eax, (%esp)
	call	_ker_Mfree
	movl	204(%ebx), %eax
	movl	%eax, (%esp)
	call	_ker_Mfree
	movl	208(%ebx), %eax
	movl	%eax, (%esp)
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
	movl	48(%ebx), %eax
	subl	$12, %esp
	pushl	%eax
	call	_DestoryQueue
	movl	52(%ebx), %eax
	movl	%eax, (%esp)
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
	pushl	%ebx
	subl	$16, %esp
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
	movw	(%ebx), %ax
	testw	%ax, %ax
	je	L158
	movl	4(%ebx), %eax
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
L168:
	subl	$12, %esp
	pushl	$4096
	call	_CreateQueue
	movl	%eax, 52(%ebx)
	movl	52(%ebx), %eax
	addl	$16, %esp
	testl	%eax, %eax
	je	L160
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
	movw	(%ebx), %ax
	movw	%ax, 78(%ebx)
	movw	(%ebx), %ax
	addl	$2, %eax
	movw	%ax, 80(%ebx)
	movw	(%ebx), %ax
	addl	$6, %eax
	movw	%ax, 82(%ebx)
	movw	(%ebx), %ax
	addl	$8, %eax
	movw	%ax, 84(%ebx)
	movw	(%ebx), %ax
	addl	$12, %eax
	movw	%ax, 86(%ebx)
	movw	(%ebx), %ax
	addl	$104, %eax
	movw	%ax, 88(%ebx)
	leal	92(%ebx), %eax
	subl	$4, %esp
	pushl	$176
	pushl	$0
	pushl	%eax
	call	_memset
	movw	(%ebx), %ax
	addl	$16, %eax
	movw	%ax, 92(%ebx)
	movw	(%ebx), %ax
	addl	$18, %eax
	movw	%ax, 140(%ebx)
	movw	(%ebx), %ax
	addl	$20, %eax
	movw	%ax, 138(%ebx)
	movw	(%ebx), %ax
	addl	$22, %eax
	movw	%ax, 184(%ebx)
	movw	(%ebx), %ax
	addl	$24, %eax
	movw	%ax, 182(%ebx)
	movw	(%ebx), %ax
	addl	$26, %eax
	movw	%ax, 228(%ebx)
	movw	(%ebx), %ax
	addl	$28, %eax
	movw	%ax, 226(%ebx)
	movw	(%ebx), %ax
	addl	$32, %eax
	movw	%ax, 98(%ebx)
	movw	(%ebx), %ax
	addl	$36, %eax
	movw	%ax, 102(%ebx)
	movw	(%ebx), %ax
	addl	$40, %eax
	movw	%ax, 100(%ebx)
	movw	(%ebx), %ax
	addl	$44, %eax
	movw	%ax, 146(%ebx)
	movw	(%ebx), %ax
	addl	$48, %eax
	movw	%ax, 144(%ebx)
	movw	(%ebx), %ax
	addl	$52, %eax
	movw	%ax, 190(%ebx)
	movw	(%ebx), %ax
	addl	$56, %eax
	movw	%ax, 188(%ebx)
	movw	(%ebx), %ax
	addl	$60, %eax
	movw	%ax, 234(%ebx)
	movw	(%ebx), %ax
	addl	$64, %eax
	movw	%ax, 232(%ebx)
	movw	(%ebx), %ax
	addl	$68, %eax
	movw	%ax, 104(%ebx)
	movw	(%ebx), %ax
	addl	$72, %eax
	movw	%ax, 108(%ebx)
	movw	(%ebx), %ax
	addl	$76, %eax
	movw	%ax, 106(%ebx)
	movw	(%ebx), %ax
	addl	$80, %eax
	movw	%ax, 152(%ebx)
	movw	(%ebx), %ax
	addl	$84, %eax
	movw	%ax, 150(%ebx)
	movw	(%ebx), %ax
	addl	$88, %eax
	movw	%ax, 196(%ebx)
	movw	(%ebx), %ax
	addl	$92, %eax
	movw	%ax, 194(%ebx)
	movw	(%ebx), %ax
	addl	$96, %eax
	movw	%ax, 240(%ebx)
	movw	(%ebx), %ax
	addl	$100, %eax
	movw	%ax, 238(%ebx)
	movl	$3328, (%esp)
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
	jne	L161
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
L161:
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
	jne	L162
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
L162:
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
	jne	L163
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
L163:
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
	jne	L164
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
L164:
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
L160:
	movl	48(%ebx), %eax
	subl	$12, %esp
	pushl	%eax
	call	_DestoryQueue
	addl	$16, %esp
L165:
	call	_io_Close
	subl	$12, %esp
	pushl	%ebx
	call	_ker_Mfree
	movl	$0, %eax
	addl	$16, %esp
L156:
	movl	-4(%ebp), %ebx
	leave
	ret
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
	movl	48(%ebx), %eax
	addl	$16, %esp
	testl	%eax, %eax
	jne	L168
	jmp	L165
	.section	.text._ZL9SetEPnDLRPV10usb_deviceiim,"x"
__ZL9SetEPnDLRPV10usb_deviceiim:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	12(%ebp), %edx
	movl	16(%ebp), %eax
	cmpl	$1, %eax
	je	L172
	cmpl	$2, %eax
	je	L173
	testl	%eax, %eax
	jne	L174
	leal	(%edx,%edx,4), %eax
	leal	(%edx,%eax,2), %eax
	movl	8(%ebp), %edx
	leal	80(%edx,%eax,4), %eax
	movw	20(%eax), %ax
	subl	$4, %esp
	pushl	20(%ebp)
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
	jmp	L174
L172:
	leal	(%edx,%edx,4), %eax
	leal	(%edx,%eax,2), %eax
	movl	8(%ebp), %ecx
	leal	80(%ecx,%eax,4), %eax
	movw	22(%eax), %ax
	subl	$4, %esp
	pushl	20(%ebp)
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
	jmp	L174
L173:
	leal	(%edx,%edx,4), %eax
	leal	(%edx,%eax,2), %eax
	movl	8(%ebp), %edx
	leal	80(%edx,%eax,4), %eax
	movw	18(%eax), %ax
	subl	$4, %esp
	pushl	20(%ebp)
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L174:
	leave
	ret
	.section	.text._ZL17Set_ConfigurationPV10usb_device,"x"
__ZL17Set_ConfigurationPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	movl	12(%ebx), %eax
	cmpl	$3, %eax
	je	L184
	movb	56(%ebx), %al
	testb	%al, %al
	js	L184
	movb	59(%ebx), %al
	testb	%al, %al
	jne	L184
	movw	60(%ebx), %ax
	testw	%ax, %ax
	jne	L184
	movw	62(%ebx), %ax
	testw	%ax, %ax
	jne	L184
	jmp	L186
L178:
	movb	58(%ebx), %al
	testb	%al, %al
	je	L179
	movb	58(%ebx), %al
	cmpb	$1, %al
	jne	L180
	movl	$5, 12(%ebx)
	pushl	$-2147483584
	pushl	$1
	pushl	$2
	pushl	%ebx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$32, %esp
	jmp	L184
L180:
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L184
L179:
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L184
L181:
	movb	58(%ebx), %al
	testb	%al, %al
	jne	L182
	movl	$4, 12(%ebx)
	pushl	$0
	pushl	$1
	pushl	$2
	pushl	%ebx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$32, %esp
	jmp	L184
L182:
	movb	58(%ebx), %al
	cmpb	$1, %al
	je	L183
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L184
L183:
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L184
L186:
	movl	12(%ebx), %eax
	cmpl	$4, %eax
	je	L178
	cmpl	$5, %eax
	je	L181
L184:
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
	movl	20(%ebx), %eax
	movb	$1, %dl
	cmpl	$1, %eax
	je	L189
	call	__ZL11USB_Connectv
	call	_irq_Init
	testb	%al, %al
	jne	L190
	subl	$8, %esp
	pushl	$__ZZ8usb_InitE12__FUNCTION__
	pushl	$LC5
	call	__ZL9err_printPcz
	call	_io_Close
	movb	$0, %dl
	addl	$16, %esp
	jmp	L189
L190:
	cmpb	$0, __ZZ8usb_InitE6locked
	jne	L191
	subl	$8, %esp
	movl	$__ZL13SetEPnDLR_endv, %eax
	subl	$__ZL9SetEPnDLRPV10usb_deviceiim, %eax
	pushl	%eax
	pushl	$__ZL9SetEPnDLRPV10usb_deviceiim
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL15Set_Address_endv, %eax
	subl	$__ZL11Set_AddressPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL11Set_AddressPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL18Get_Descriptor_endv, %eax
	subl	$__ZL14Get_DescriptorPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL14Get_DescriptorPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL18Set_Descriptor_endv, %eax
	subl	$__ZL14Set_DescriptorPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL14Set_DescriptorPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL21Get_Configuration_endv, %eax
	subl	$__ZL17Get_ConfigurationPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL17Get_ConfigurationPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL21Set_Configuration_endv, %eax
	subl	$__ZL17Set_ConfigurationPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL17Set_ConfigurationPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL17Get_Interface_endv, %eax
	subl	$__ZL13Get_InterfacePV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL13Get_InterfacePV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL17Set_Interface_endv, %eax
	subl	$__ZL13Set_InterfacePV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL13Set_InterfacePV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL15Synch_Frame_endv, %eax
	subl	$__ZL11Synch_FramePV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL11Synch_FramePV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL24USB_Standard_Request_endv, %eax
	subl	$__ZL20USB_Standard_RequestPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL20USB_Standard_RequestPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL19Set_Line_Coding_endv, %eax
	subl	$__ZL15Set_Line_CodingPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL15Set_Line_CodingPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL19Get_Line_Coding_endv, %eax
	subl	$__ZL15Get_Line_CodingPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL15Get_Line_CodingPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL26Set_Control_Line_State_endv, %eax
	subl	$__ZL22Set_Control_Line_StatePV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL22Set_Control_Line_StatePV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL14Send_Break_endv, %eax
	subl	$__ZL10Send_BreakPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL10Send_BreakPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL19USB_CDC_Request_endv, %eax
	subl	$__ZL15USB_CDC_RequestPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL15USB_CDC_RequestPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL20EP0_SetupHandler_endv, %eax
	subl	$__ZL16EP0_SetupHandlerPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL16EP0_SetupHandlerPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL17EP0_InHandler_endv, %eax
	subl	$__ZL13EP0_InHandlerPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL13EP0_InHandlerPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL18EP0_OutHandler_endv, %eax
	subl	$__ZL14EP0_OutHandlerPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL14EP0_OutHandlerPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL17EP1_InHandler_endv, %eax
	subl	$__ZL13EP1_InHandlerPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL13EP1_InHandlerPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL17EP2_InHandler_endv, %eax
	subl	$__ZL13EP2_InHandlerPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL13EP2_InHandlerPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL18EP2_OutHandler_endv, %eax
	subl	$__ZL14EP2_OutHandlerPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL14EP2_OutHandlerPV10usb_device
	call	_dpmi_LockCode
	addl	$8, %esp
	movl	$__ZL13usb_Reset_endv, %eax
	subl	$__ZL9usb_ResetPV10usb_device, %eax
	pushl	%eax
	pushl	$__ZL9usb_ResetPV10usb_device
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
L191:
	movw	80(%ebx), %ax
	subl	$4, %esp
	pushl	$2
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	addl	$16, %esp
L192:
	movw	80(%ebx), %ax
	subl	$8, %esp
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$16, %esp
	testb	$2, %al
	jne	L192
	subl	$12, %esp
	pushl	$8
	call	_ker_Malloc
	movl	%eax, 112(%ebx)
	movl	112(%ebx), %eax
	addl	$16, %esp
	testl	%eax, %eax
	je	L193
	subl	$12, %esp
	pushl	$8
	call	_ker_Malloc
	movl	%eax, 116(%ebx)
	movl	116(%ebx), %eax
	addl	$16, %esp
	testl	%eax, %eax
	je	L194
	subl	$12, %esp
	pushl	$8
	call	_ker_Malloc
	movl	%eax, 120(%ebx)
	movl	120(%ebx), %eax
	addl	$16, %esp
	testl	%eax, %eax
	je	L195
	subl	$12, %esp
	pushl	$8
	call	_ker_Malloc
	movl	%eax, 160(%ebx)
	movl	160(%ebx), %eax
	addl	$16, %esp
	testl	%eax, %eax
	je	L196
	subl	$12, %esp
	pushl	$64
	call	_ker_Malloc
	movl	%eax, 204(%ebx)
	movl	204(%ebx), %eax
	addl	$16, %esp
	testl	%eax, %eax
	je	L197
	subl	$12, %esp
	pushl	$64
	call	_ker_Malloc
	movl	%eax, 208(%ebx)
	movl	208(%ebx), %eax
	addl	$16, %esp
	testl	%eax, %eax
	je	L198
	subl	$8, %esp
	leal	-8(%ebp), %eax
	pushl	%eax
	pushl	$10
	call	___dpmi_allocate_dos_memory
	addl	$16, %esp
	cmpl	$-1, %eax
	je	L199
	sall	$4, %eax
	movl	%eax, __ZL8dma_addr
	movl	-8(%ebp), %eax
	movl	%eax, __ZL10dma_handle
	cmpl	$-1, %eax
	je	L198
	movl	__ZL8dma_addr, %eax
	movl	%eax, 124(%ebx)
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
	movl	124(%ebx), %edx
	movw	104(%ebx), %ax
	andl	$65535, %eax
	call	__ZL9io_outpdwtm
	movl	128(%ebx), %edx
	movw	106(%ebx), %ax
	andl	$65535, %eax
	call	__ZL9io_outpdwtm
	movl	132(%ebx), %edx
	movw	108(%ebx), %ax
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
	movw	196(%ebx), %ax
	andl	$65535, %eax
	call	__ZL9io_outpdwtm
	movw	92(%ebx), %ax
	subl	$4, %esp
	pushl	$8200
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	movw	138(%ebx), %ax
	addl	$12, %esp
	pushl	$14344
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	movw	182(%ebx), %ax
	addl	$12, %esp
	pushl	$12352
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	movw	184(%ebx), %ax
	addl	$12, %esp
	pushl	$12352
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out16
	pushl	$-2147483648
	pushl	$2
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	movw	78(%ebx), %ax
	addl	$32, %esp
	andl	$65535, %eax
	movl	$128, %edx
	call	__ZL8io_outpbth
L200:
	movw	78(%ebx), %ax
	subl	$8, %esp
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In8
	addl	$16, %esp
	testb	%al, %al
	jns	L200
	movw	86(%ebx), %ax
	andl	$65535, %eax
	movl	$-1, %edx
	call	__ZL9io_outpdwtm
	call	_io_DisableINT
	movw	80(%ebx), %ax
	andl	$65535, %eax
	call	__ZL7io_inpbt
	movb	%al, %dl
	movw	80(%ebx), %ax
	andl	$254, %edx
	andl	$65535, %eax
	call	__ZL8io_outpbth
	movl	4(%ebx), %eax
	subl	$8, %esp
	pushl	$1
	pushl	%eax
	call	_irq_Setting
	movl	4(%ebx), %eax
	addl	$12, %esp
	pushl	%ebx
	pushl	$__ZL7USB_ISRiPv
	pushl	%eax
	call	_irq_InstallISR
	movw	84(%ebx), %ax
	andl	$65535, %eax
	movl	$30495, %edx
	call	__ZL9io_outpdwtm
	movw	80(%ebx), %ax
	andl	$65535, %eax
	call	__ZL7io_inpbt
	movw	80(%ebx), %cx
	orl	$1, %eax
	movl	$0, %edx
	movb	%al, %dl
	movl	%ecx, %eax
	andl	$65535, %eax
	call	__ZL8io_outpbth
	call	_io_RestoreINT
	movl	$2, 12(%ebx)
	movl	$1, 20(%ebx)
	movb	$1, %dl
	addl	$16, %esp
	jmp	L189
L198:
	movl	204(%ebx), %eax
	subl	$12, %esp
	pushl	%eax
	call	_ker_Mfree
	addl	$16, %esp
L197:
	movl	160(%ebx), %eax
	subl	$12, %esp
	pushl	%eax
	call	_ker_Mfree
	addl	$16, %esp
L196:
	movl	120(%ebx), %eax
	subl	$12, %esp
	pushl	%eax
	call	_ker_Mfree
	addl	$16, %esp
L195:
	movl	116(%ebx), %eax
	subl	$12, %esp
	pushl	%eax
	call	_ker_Mfree
	addl	$16, %esp
L194:
	movl	112(%ebx), %eax
	subl	$12, %esp
	pushl	%eax
	call	_ker_Mfree
	addl	$16, %esp
L193:
	subl	$8, %esp
	pushl	$__ZZ8usb_InitE12__FUNCTION__
	pushl	$LC6
	call	__ZL9err_printPcz
	call	_irq_Close
	call	_io_Close
	movb	$0, %dl
	addl	$16, %esp
L189:
	movb	%dl, %al
	movl	-4(%ebp), %ebx
	leave
	ret
L199:
	movl	$-1, __ZL10dma_handle
	jmp	L198
	.section	.text._ZL22Set_Control_Line_StatePV10usb_device,"x"
__ZL22Set_Control_Line_StatePV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %edx
	movb	56(%edx), %al
	testb	%al, %al
	js	L207
	movw	58(%edx), %ax
	movw	%ax, 72(%edx)
	movw	72(%edx), %ax
	testb	$1, %al
	jne	L206
	movl	$5, 12(%edx)
L206:
	movw	100(%edx), %ax
	subl	$4, %esp
	pushl	$-1610612736
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L207:
	leave
	ret
	.section	.text._ZL10Send_BreakPV10usb_device,"x"
__ZL10Send_BreakPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %edx
	movb	56(%edx), %al
	testb	%al, %al
	js	L210
	movw	58(%edx), %ax
	movw	%ax, 76(%edx)
	movw	100(%edx), %ax
	subl	$4, %esp
	pushl	$-1610612736
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L210:
	leave
	ret
	.section	.text._ZL15Set_Line_CodingPV10usb_device,"x"
__ZL15Set_Line_CodingPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %esi
	movb	56(%esi), %al
	testb	%al, %al
	js	L215
	leal	64(%esi), %eax
	movl	%eax, 40(%esi)
	movw	62(%esi), %ax
	movw	%ax, 46(%esi)
	movb	$1, 29(%esi)
	movw	102(%esi), %ax
	subl	$4, %esp
	pushl	$-2147483640
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movl	$7, 12(%esi)
	movl	52(%esi), %eax
	movl	%eax, (%esp)
	call	_QueueEmpty
	addl	$16, %esp
	testb	%al, %al
	jne	L215
	movb	30(%esi), %al
	testb	%al, %al
	jne	L215
	movl	52(%esi), %eax
	movl	8(%eax), %eax
	testl	%eax, %eax
	je	L215
	call	__ZL9TX_LED_ONv
	movl	$0, %edi
L214:
	movl	204(%esi), %ebx
	movl	52(%esi), %eax
	subl	$12, %esp
	pushl	%eax
	call	_PopQueue
	movb	%al, (%ebx,%edi)
	incl	%edi
	movl	52(%esi), %eax
	movl	8(%eax), %eax
	addl	$16, %esp
	testl	%eax, %eax
	je	L213
	cmpl	$64, %edi
	jne	L214
L213:
	movl	216(%esi), %eax
	movl	204(%esi), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$64
	pushl	%edx
	call	_dosmemput
	movl	%edi, %eax
	orl	$-2147483648, %eax
	pushl	%eax
	pushl	$0
	pushl	$2
	pushl	%esi
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	movb	$1, 30(%esi)
	addl	$32, %esp
L215:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZL13EP1_InHandlerPV10usb_device,"x"
__ZL13EP1_InHandlerPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	call	__ZL9TX_LED_ONv
	cmpb	$0, __ZZL13EP1_InHandlerPV10usb_deviceE9Completed
	je	L217
	movl	$__ZZL13EP1_InHandlerPV10usb_deviceE12notification, 160(%ebx)
	movl	172(%ebx), %eax
	movl	160(%ebx), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$8
	pushl	%edx
	call	_dosmemput
	movw	144(%ebx), %ax
	addl	$12, %esp
	pushl	$-2147483640
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	$0, __ZZL13EP1_InHandlerPV10usb_deviceE9Completed
	addl	$16, %esp
	jmp	L219
L217:
	movl	160(%ebx), %edx
	movw	74(%ebx), %ax
	movb	%al, (%edx)
	movl	160(%ebx), %edx
	movw	74(%ebx), %ax
	shrw	$8, %ax
	movb	%al, 1(%edx)
	movl	172(%ebx), %eax
	movl	160(%ebx), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$8
	pushl	%edx
	call	_dosmemput
	movw	144(%ebx), %ax
	addl	$12, %esp
	pushl	$-2147483646
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	$1, __ZZL13EP1_InHandlerPV10usb_deviceE9Completed
	addl	$16, %esp
L219:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZL9usb_ResetPV10usb_device,"x"
__ZL9usb_ResetPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	movl	20(%ebx), %eax
	movb	$0, %dl
	testl	%eax, %eax
	je	L222
	movb	$0, 2(%ebx)
	movb	$0, 8(%ebx)
	movw	78(%ebx), %ax
	subl	$4, %esp
	pushl	$128
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	movw	98(%ebx), %ax
	addl	$12, %esp
	pushl	$-2147483648
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movw	100(%ebx), %ax
	addl	$12, %esp
	pushl	$0
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movw	102(%ebx), %ax
	addl	$12, %esp
	pushl	$0
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movw	144(%ebx), %ax
	addl	$12, %esp
	pushl	$0
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movw	190(%ebx), %ax
	addl	$12, %esp
	pushl	$0
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movw	188(%ebx), %ax
	addl	$12, %esp
	pushl	$0
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movl	48(%ebx), %eax
	movl	%eax, (%esp)
	call	_ClearQueue
	movl	52(%ebx), %eax
	movl	%eax, (%esp)
	call	_ClearQueue
	movl	$3, 12(%ebx)
	movb	$1, %dl
	addl	$16, %esp
L222:
	movb	%dl, %al
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZL14EP2_OutHandlerPV10usb_device,"x"
__ZL14EP2_OutHandlerPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %esi
	call	__ZL9RX_LED_ONv
	movw	190(%esi), %ax
	subl	$8, %esp
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	movl	%eax, %ebx
	movl	208(%esi), %eax
	movl	220(%esi), %edx
	addl	$12, %esp
	pushl	%eax
	pushl	$64
	pushl	%edx
	call	_dosmemget
	movl	%ebx, %edi
	andl	$65535, %edi
	addl	$16, %esp
	testl	%edi, %edi
	jle	L225
	movl	$0, %ebx
L226:
	movl	208(%esi), %eax
	movl	48(%esi), %edx
	subl	$8, %esp
	movb	(%eax,%ebx), %al
	andl	$255, %eax
	pushl	%eax
	pushl	%edx
	call	_PushQueue
	incl	%ebx
	addl	$16, %esp
	cmpl	%edi, %ebx
	jl	L226
L225:
	movl	48(%esi), %eax
	movl	8(%eax), %edx
	movl	48(%esi), %eax
	movl	12(%eax), %eax
	subl	$1024, %eax
	cmpl	%eax, %edx
	jae	L228
	movw	190(%esi), %ax
	subl	$4, %esp
	pushl	$-2147483584
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L228:
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
	je	L231
	movl	$0, -16(%ebp)
	cmpl	$0, 16(%ebp)
	jg	L243
	jmp	L233
L231:
	subl	$8, %esp
	pushl	$__ZZ8usb_SendE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movl	$0, -16(%ebp)
	addl	$16, %esp
	jmp	L233
L243:
	movl	16(%ebp), %eax
	decl	%eax
	movl	%eax, -20(%ebp)
L247:
	movl	32(%esi), %eax
	cmpl	$-1, %eax
	je	L248
	call	_timer_nowtime
	movl	%eax, %ebx
L236:
	movl	52(%esi), %eax
	movl	8(%eax), %edx
	movl	52(%esi), %eax
	movl	12(%eax), %eax
	cmpl	%eax, %edx
	jb	L235
	call	_timer_nowtime
	movl	32(%esi), %edx
	subl	%ebx, %eax
	cmpl	%edx, %eax
	jae	L235
	jmp	L236
L251:
	movb	30(%esi), %al
	testb	%al, %al
	jne	L233
	movl	52(%esi), %eax
	movl	8(%eax), %eax
	testl	%eax, %eax
	je	L233
	call	__ZL9TX_LED_ONv
	movl	$0, %edi
L238:
	movl	204(%esi), %ebx
	movl	52(%esi), %eax
	subl	$12, %esp
	pushl	%eax
	call	_PopQueue
	movb	%al, (%ebx,%edi)
	incl	%edi
	movl	52(%esi), %eax
	movl	8(%eax), %eax
	addl	$16, %esp
	testl	%eax, %eax
	je	L237
	cmpl	$64, %edi
	je	L237
	jmp	L238
L248:
	movl	52(%esi), %eax
	movl	8(%eax), %edx
	movl	52(%esi), %eax
	movl	12(%eax), %eax
	cmpl	%eax, %edx
	jae	L248
L245:
	call	_io_DisableINT
	movl	52(%esi), %edx
	subl	$8, %esp
	movl	12(%ebp), %ebx
	movl	-16(%ebp), %ecx
	movl	$0, %eax
	movb	(%ebx,%ecx), %al
	pushl	%eax
	pushl	%edx
	call	_PushQueue
	addl	$16, %esp
	movl	-16(%ebp), %eax
	cmpl	%eax, -20(%ebp)
	je	L239
	movl	52(%esi), %eax
	movl	8(%eax), %edx
	movl	52(%esi), %eax
	movl	12(%eax), %eax
	cmpl	%eax, %edx
	jb	L240
L239:
	movl	12(%esi), %eax
	cmpl	$7, %eax
	jne	L240
	movb	30(%esi), %al
	testb	%al, %al
	jne	L240
	movl	52(%esi), %eax
	movl	8(%eax), %eax
	testl	%eax, %eax
	je	L240
	call	__ZL9TX_LED_ONv
	movl	$0, %edi
L242:
	movl	204(%esi), %ebx
	movl	52(%esi), %eax
	subl	$12, %esp
	pushl	%eax
	call	_PopQueue
	movb	%al, (%ebx,%edi)
	incl	%edi
	movl	52(%esi), %eax
	movl	8(%eax), %eax
	addl	$16, %esp
	testl	%eax, %eax
	je	L241
	cmpl	$64, %edi
	je	L241
	jmp	L242
L240:
	call	_io_RestoreINT
	incl	-16(%ebp)
	movl	-16(%ebp), %ecx
	cmpl	%ecx, 16(%ebp)
	jg	L247
L233:
	movl	-16(%ebp), %eax
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
L235:
	movl	52(%esi), %eax
	movl	8(%eax), %edx
	movl	52(%esi), %eax
	movl	12(%eax), %eax
	cmpl	%eax, %edx
	jb	L245
	jmp	L251
L237:
	movl	216(%esi), %eax
	movl	204(%esi), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$64
	pushl	%edx
	call	_dosmemput
	movl	%edi, %eax
	orl	$-2147483648, %eax
	pushl	%eax
	pushl	$0
	pushl	$2
	pushl	%esi
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	movb	$1, 30(%esi)
	addl	$32, %esp
	jmp	L233
L241:
	movl	216(%esi), %eax
	movl	204(%esi), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$64
	pushl	%edx
	call	_dosmemput
	movw	188(%esi), %dx
	addl	$12, %esp
	movl	%edi, %eax
	orl	$-2147483648, %eax
	pushl	%eax
	andl	$65535, %edx
	pushl	%edx
	pushl	$0
	call	_io_Out32
	movb	$1, 30(%esi)
	addl	$16, %esp
	jmp	L240
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
	.section	.text.usb_Read,"x"
.globl _usb_Read
_usb_Read:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %ebx
	testl	%ebx, %ebx
	jne	L255
	subl	$8, %esp
	pushl	$__ZZ8usb_ReadE12__FUNCTION__
	pushl	$LC2
	call	__ZL9err_printPcz
	movl	$65535, %edx
	addl	$16, %esp
	jmp	L256
L255:
	movl	32(%ebx), %eax
	cmpl	$-1, %eax
	je	L263
	call	_timer_nowtime
	movl	%eax, %esi
L259:
	movl	48(%ebx), %eax
	movl	8(%eax), %eax
	testl	%eax, %eax
	jne	L258
	call	_timer_nowtime
	movl	32(%ebx), %edx
	subl	%esi, %eax
	cmpl	%edx, %eax
	jae	L258
	jmp	L259
L263:
	movl	48(%ebx), %eax
	movl	8(%eax), %eax
	testl	%eax, %eax
	je	L263
L261:
	call	_io_DisableINT
	movl	48(%ebx), %eax
	subl	$12, %esp
	pushl	%eax
	call	_PopQueue
	movl	%eax, %esi
	call	_io_RestoreINT
	movw	190(%ebx), %ax
	addl	$8, %esp
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	addl	$16, %esp
	testl	%eax, %eax
	js	L260
	movl	48(%ebx), %eax
	movl	8(%eax), %edx
	movl	48(%ebx), %eax
	movl	12(%eax), %eax
	subl	$1024, %eax
	cmpl	%eax, %edx
	jae	L260
	movw	190(%ebx), %ax
	subl	$4, %esp
	pushl	$-2147483584
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L260:
	movl	%esi, %edx
	andl	$255, %edx
L256:
	movl	%edx, %eax
	leal	-8(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
L258:
	movl	48(%ebx), %eax
	movl	8(%eax), %eax
	movl	$65535, %edx
	testl	%eax, %eax
	jne	L261
	jmp	L256
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
	jle	L267
L270:
	subl	$12, %esp
	pushl	8(%ebp)
	call	_usb_Read
	addl	$16, %esp
	cmpl	$65535, %eax
	je	L267
	movb	%al, (%edi,%ebx)
	incl	%ebx
	cmpl	%ebx, %esi
	jg	L270
L267:
	movl	%ebx, %eax
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZL13EP2_InHandlerPV10usb_device,"x"
__ZL13EP2_InHandlerPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %edi
	movb	30(%edi), %al
	testb	%al, %al
	jne	L276
	movl	52(%edi), %eax
	movl	8(%eax), %eax
	testl	%eax, %eax
	je	L276
	call	__ZL9TX_LED_ONv
	movl	$0, %esi
L275:
	movl	204(%edi), %ebx
	movl	52(%edi), %eax
	subl	$12, %esp
	pushl	%eax
	call	_PopQueue
	movb	%al, (%ebx,%esi)
	incl	%esi
	movl	52(%edi), %eax
	movl	8(%eax), %eax
	addl	$16, %esp
	testl	%eax, %eax
	je	L274
	cmpl	$64, %esi
	jne	L275
L274:
	movl	216(%edi), %eax
	movl	204(%edi), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$64
	pushl	%edx
	call	_dosmemput
	movw	188(%edi), %dx
	addl	$12, %esp
	movl	%esi, %eax
	orl	$-2147483648, %eax
	pushl	%eax
	andl	$65535, %edx
	pushl	%edx
	pushl	$0
	call	_io_Out32
	movb	$1, 30(%edi)
	addl	$16, %esp
L276:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZL14Set_DescriptorPV10usb_device,"x"
__ZL14Set_DescriptorPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %ecx
	movb	56(%ecx), %al
	testb	%al, %al
	js	L284
	movb	59(%ecx), %al
	cmpb	$2, %al
	je	L281
	cmpb	$3, %al
	je	L282
	cmpb	$1, %al
	jne	L285
	movb	58(%ecx), %al
	testb	%al, %al
	jne	L284
	movw	60(%ecx), %ax
	testw	%ax, %ax
	jne	L284
	movl	$_desc_Device, 40(%ecx)
	movw	62(%ecx), %ax
	movw	%ax, 46(%ecx)
	movb	$1, 29(%ecx)
	pushl	$-2147483640
	pushl	$1
	pushl	$0
	pushl	%ecx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L284
L281:
	movb	58(%ecx), %al
	testb	%al, %al
	jne	L284
	movw	60(%ecx), %ax
	testw	%ax, %ax
	jne	L284
	movl	$_desc_Config_Set, 40(%ecx)
	movw	62(%ecx), %ax
	movw	%ax, 46(%ecx)
	movb	$1, 29(%ecx)
	pushl	$-2147483640
	pushl	$1
	pushl	$0
	pushl	%ecx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L284
L282:
	movb	58(%ecx), %al
	cmpb	$6, %al
	ja	L283
	movb	58(%ecx), %al
	andl	$255, %eax
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
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L284
L283:
	pushl	$-1073741824
	pushl	$1
	pushl	$0
	pushl	%ecx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L284
L285:
	movw	102(%ecx), %ax
	subl	$4, %esp
	pushl	$-1073741824
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L284:
	leave
	ret
	.section	.text._ZL11Set_AddressPV10usb_device,"x"
__ZL11Set_AddressPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	movl	12(%ebx), %eax
	cmpl	$5, %eax
	je	L291
	movb	56(%ebx), %al
	testb	%al, %al
	js	L291
	movw	58(%ebx), %ax
	cmpw	$127, %ax
	ja	L291
	movw	60(%ebx), %ax
	testw	%ax, %ax
	jne	L291
	movw	62(%ebx), %ax
	testw	%ax, %ax
	jne	L291
	movb	58(%ebx), %al
	movb	%al, 2(%ebx)
	movw	100(%ebx), %ax
	subl	$4, %esp
	pushl	$-1610612736
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	$1, 8(%ebx)
	movb	2(%ebx), %al
	addl	$16, %esp
	cmpb	$1, %al
	sbbl	%eax, %eax
	addl	$4, %eax
	movl	%eax, 12(%ebx)
L291:
	movl	-4(%ebp), %ebx
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
	cmpb	$0, __ZZL13EP1_InHandlerPV10usb_deviceE9Completed
	je	L293
	movl	$__ZZL13EP1_InHandlerPV10usb_deviceE12notification, 160(%ebx)
	movl	172(%ebx), %eax
	movl	160(%ebx), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$8
	pushl	%edx
	call	_dosmemput
	movw	144(%ebx), %ax
	addl	$12, %esp
	pushl	$-2147483640
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	$0, __ZZL13EP1_InHandlerPV10usb_deviceE9Completed
	addl	$16, %esp
	jmp	L295
L293:
	movl	160(%ebx), %edx
	movw	74(%ebx), %ax
	movb	%al, (%edx)
	movl	160(%ebx), %edx
	movw	74(%ebx), %ax
	shrw	$8, %ax
	movb	%al, 1(%edx)
	movl	172(%ebx), %eax
	movl	160(%ebx), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$8
	pushl	%edx
	call	_dosmemput
	movw	144(%ebx), %ax
	addl	$12, %esp
	pushl	$-2147483646
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	$1, __ZZL13EP1_InHandlerPV10usb_deviceE9Completed
	addl	$16, %esp
L295:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZL13EP0_InHandlerPV10usb_device,"x"
__ZL13EP0_InHandlerPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %esi
	movb	28(%esi), %al
	testb	%al, %al
	je	L303
	call	__ZL9TX_LED_ONv
	movw	44(%esi), %ax
	movl	__ZZL13EP0_InHandlerPV10usb_deviceE1j, %ebx
	andl	$65535, %eax
	cmpl	%ebx, %eax
	jle	L298
	movl	$0, %edi
L300:
	movw	44(%esi), %ax
	andl	$65535, %eax
	cmpl	%ebx, %eax
	jle	L299
	movl	116(%esi), %eax
	movl	36(%esi), %edx
	movb	(%edx), %cl
	movb	%cl, (%eax,%edi)
	incl	%edx
	movl	%edx, 36(%esi)
	incl	%edi
	incl	%ebx
	cmpl	$8, %edi
	jne	L300
	movl	%ebx, __ZZL13EP0_InHandlerPV10usb_deviceE1j
	jmp	L301
L298:
	movw	62(%esi), %ax
	andl	$65535, %eax
	cmpl	%eax, %ebx
	jge	L302
	testb	$7, %bl
	jne	L302
	pushl	$-2147483648
	pushl	$0
	pushl	$0
	pushl	%esi
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
L302:
	movl	$0, __ZZL13EP0_InHandlerPV10usb_deviceE1j
	movw	$0, 44(%esi)
	movb	$0, 28(%esi)
	movw	102(%esi), %ax
	subl	$4, %esp
	pushl	$-1610612736
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
	jmp	L303
L299:
	movl	%ebx, __ZZL13EP0_InHandlerPV10usb_deviceE1j
L301:
	movl	128(%esi), %eax
	movl	116(%esi), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$8
	pushl	%edx
	call	_dosmemput
	movw	100(%esi), %dx
	addl	$12, %esp
	movl	%edi, %eax
	orl	$-2147483648, %eax
	pushl	%eax
	andl	$65535, %edx
	pushl	%edx
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L303:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZL11Synch_FramePV10usb_device,"x"
__ZL11Synch_FramePV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$20, %esp
	movl	8(%ebp), %ebx
	movb	56(%ebx), %al
	testb	%al, %al
	jns	L310
	movw	58(%ebx), %ax
	testw	%ax, %ax
	jne	L310
	movw	62(%ebx), %ax
	cmpw	$2, %ax
	jne	L310
	jmp	L312
L308:
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L310
L309:
	movw	82(%ebx), %ax
	subl	$8, %esp
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
	movb	28(%ebx), %al
	addl	$16, %esp
	testb	%al, %al
	je	L310
	subl	$12, %esp
	pushl	%ebx
	call	__ZL13EP0_InHandlerPV10usb_device
	addl	$16, %esp
	jmp	L310
L312:
	movl	12(%ebx), %eax
	cmpl	$4, %eax
	je	L308
	cmpl	$5, %eax
	je	L309
L310:
	movl	-4(%ebp), %ebx
	leave
	ret
	.section	.text._ZL13Get_InterfacePV10usb_device,"x"
__ZL13Get_InterfacePV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %edx
	movb	56(%edx), %al
	testb	%al, %al
	jns	L321
	movw	58(%edx), %ax
	testw	%ax, %ax
	jne	L321
	movw	62(%edx), %ax
	cmpw	$1, %ax
	jne	L321
	jmp	L324
L316:
	movl	12(%edx), %eax
	cmpl	$4, %eax
	jne	L317
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%edx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L321
L317:
	movl	12(%edx), %eax
	cmpl	$5, %eax
	jne	L321
	movl	$_desc_Config_Set+11, 36(%edx)
	movw	$1, 44(%edx)
	movb	$1, 28(%edx)
	movb	28(%edx), %al
	testb	%al, %al
	je	L321
	subl	$12, %esp
	pushl	%edx
	call	__ZL13EP0_InHandlerPV10usb_device
	addl	$16, %esp
	jmp	L321
L325:
	movl	12(%edx), %eax
	cmpl	$4, %eax
	jne	L319
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%edx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L321
L319:
	movl	12(%edx), %eax
	cmpl	$5, %eax
	jne	L321
	movl	$_desc_Config_Set+46, 36(%edx)
	movw	$1, 44(%edx)
	movb	$1, 28(%edx)
	movb	28(%edx), %al
	testb	%al, %al
	je	L321
	subl	$12, %esp
	pushl	%edx
	call	__ZL13EP0_InHandlerPV10usb_device
	addl	$16, %esp
	jmp	L321
L323:
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%edx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L321
L324:
	movw	60(%edx), %ax
	testw	%ax, %ax
	je	L316
	cmpw	$1, %ax
	jne	L323
	jmp	L325
L321:
	leave
	ret
	.section	.text._ZL17Get_ConfigurationPV10usb_device,"x"
__ZL17Get_ConfigurationPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %edx
	movb	56(%edx), %al
	testb	%al, %al
	jns	L330
	movw	58(%edx), %ax
	testw	%ax, %ax
	jne	L330
	movw	60(%edx), %ax
	testw	%ax, %ax
	jne	L330
	movw	62(%edx), %ax
	cmpw	$1, %ax
	jne	L330
	jmp	L332
L329:
	subl	$12, %esp
	pushl	%edx
	call	__ZL13EP0_InHandlerPV10usb_device
	addl	$16, %esp
	jmp	L330
L332:
	movl	$_desc_Config_Set+5, 36(%edx)
	movw	$1, 44(%edx)
	movb	$1, 28(%edx)
	movb	28(%edx), %al
	testb	%al, %al
	jne	L329
L330:
	leave
	ret
	.section	.text._ZL14Get_DescriptorPV10usb_device,"x"
__ZL14Get_DescriptorPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %ecx
	movb	56(%ecx), %al
	testb	%al, %al
	jns	L344
	movb	59(%ecx), %al
	cmpb	$2, %al
	je	L337
	cmpb	$3, %al
	je	L338
	cmpb	$1, %al
	jne	L345
	movb	58(%ecx), %al
	testb	%al, %al
	jne	L344
	movw	60(%ecx), %ax
	testw	%ax, %ax
	jne	L344
	movl	$_desc_Device, 36(%ecx)
	movl	36(%ecx), %eax
	movb	(%eax), %al
	andl	$255, %eax
	movw	%ax, 44(%ecx)
	movb	$1, 28(%ecx)
	movw	44(%ecx), %dx
	movw	62(%ecx), %ax
	cmpw	%ax, %dx
	jbe	L339
	movw	62(%ecx), %ax
	movw	%ax, 44(%ecx)
L339:
	movl	24(%ecx), %eax
	testl	%eax, %eax
	jne	L340
	movw	$8, 44(%ecx)
	movl	$1, 24(%ecx)
L340:
	movb	28(%ecx), %al
	testb	%al, %al
	je	L344
	subl	$12, %esp
	pushl	%ecx
	call	__ZL13EP0_InHandlerPV10usb_device
	addl	$16, %esp
	jmp	L344
L337:
	movb	58(%ecx), %al
	testb	%al, %al
	jne	L344
	movw	60(%ecx), %ax
	testw	%ax, %ax
	jne	L344
	movl	$_desc_Config_Set, 36(%ecx)
	movl	36(%ecx), %eax
	movl	36(%ecx), %edx
	movb	3(%eax), %al
	andl	$255, %eax
	sall	$8, %eax
	movb	2(%edx), %dl
	andl	$255, %edx
	addl	%edx, %eax
	movw	%ax, 44(%ecx)
	movb	$1, 28(%ecx)
	movw	44(%ecx), %dx
	movw	62(%ecx), %ax
	cmpw	%ax, %dx
	jbe	L341
	movw	62(%ecx), %ax
	movw	%ax, 44(%ecx)
L341:
	movb	28(%ecx), %al
	testb	%al, %al
	je	L344
	subl	$12, %esp
	pushl	%ecx
	call	__ZL13EP0_InHandlerPV10usb_device
	addl	$16, %esp
	jmp	L344
L338:
	movb	58(%ecx), %al
	cmpb	$6, %al
	ja	L342
	movb	58(%ecx), %al
	andl	$255, %eax
	movl	%eax, %edx
	sall	$9, %edx
	leal	_StringDescTable(%edx,%eax,2), %eax
	movl	%eax, 36(%ecx)
	movl	36(%ecx), %eax
	movb	(%eax), %al
	andl	$255, %eax
	movw	%ax, 44(%ecx)
	movb	$1, 28(%ecx)
	movw	44(%ecx), %dx
	movw	62(%ecx), %ax
	cmpw	%ax, %dx
	jbe	L343
	movw	62(%ecx), %ax
	movw	%ax, 44(%ecx)
L343:
	movb	28(%ecx), %al
	testb	%al, %al
	je	L344
	subl	$12, %esp
	pushl	%ecx
	call	__ZL13EP0_InHandlerPV10usb_device
	addl	$16, %esp
	jmp	L344
L342:
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%ecx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L344
L345:
	pushl	$-1073741824
	pushl	$0
	pushl	$0
	pushl	%ecx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
L344:
	leave
	ret
	.section	.text._ZL14EP0_OutHandlerPV10usb_device,"x"
__ZL14EP0_OutHandlerPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	8(%ebp), %ebx
	movb	29(%ebx), %al
	testb	%al, %al
	je	L352
	call	__ZL9RX_LED_ONv
	movw	102(%ebx), %ax
	subl	$8, %esp
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	movl	%eax, %esi
	movw	46(%ebx), %ax
	andl	$65535, %eax
	addl	$16, %esp
	cmpl	__ZZL14EP0_OutHandlerPV10usb_deviceE1j, %eax
	jle	L348
	movl	120(%ebx), %eax
	movl	132(%ebx), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$8
	pushl	%edx
	call	_dosmemget
	movl	%esi, %edi
	andl	$65535, %edi
	addl	$16, %esp
	testl	%edi, %edi
	jle	L348
	movw	46(%ebx), %ax
	movl	__ZZL14EP0_OutHandlerPV10usb_deviceE1j, %edx
	andl	$65535, %eax
	cmpl	%eax, %edx
	jge	L348
	leal	1(%edx), %ecx
	movl	$0, %esi
	jmp	L349
L350:
	movw	46(%ebx), %ax
	incl	%ecx
	andl	$65535, %eax
	cmpl	%edx, %eax
	jg	L349
	movl	%edx, __ZZL14EP0_OutHandlerPV10usb_deviceE1j
	jmp	L348
L349:
	movl	40(%ebx), %eax
	movl	120(%ebx), %edx
	movb	(%edx,%esi), %dl
	movb	%dl, (%eax)
	incl	%eax
	movl	%eax, 40(%ebx)
	incl	%esi
	movl	%ecx, %edx
	cmpl	%edi, %esi
	jl	L350
	movl	%ecx, __ZZL14EP0_OutHandlerPV10usb_deviceE1j
L348:
	movw	46(%ebx), %ax
	andl	$65535, %eax
	cmpl	__ZZL14EP0_OutHandlerPV10usb_deviceE1j, %eax
	jle	L351
	movw	102(%ebx), %ax
	subl	$4, %esp
	pushl	$-2147483640
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
	jmp	L352
L351:
	movl	$0, __ZZL14EP0_OutHandlerPV10usb_deviceE1j
	movw	$0, 46(%ebx)
	movb	$0, 29(%ebx)
	movw	100(%ebx), %ax
	subl	$4, %esp
	pushl	$-1610612736
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L352:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.section	.text._ZL13Set_InterfacePV10usb_device,"x"
__ZL13Set_InterfacePV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %edx
	movb	56(%edx), %al
	testb	%al, %al
	js	L361
	movw	62(%edx), %ax
	testw	%ax, %ax
	jne	L361
	movw	60(%edx), %ax
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
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L361
L359:
	movl	12(%edx), %eax
	cmpl	$5, %eax
	jne	L361
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%edx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
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
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L361
L360:
	movl	12(%edx), %eax
	cmpl	$5, %eax
	jne	L361
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%edx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L361
L362:
	movw	100(%edx), %ax
	subl	$4, %esp
	pushl	$-1073741824
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L361:
	leave
	ret
	.section	.text._ZL16EP0_SetupHandlerPV10usb_device,"x"
__ZL16EP0_SetupHandlerPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$8, %esp
	movl	8(%ebp), %ebx
	movl	112(%ebx), %eax
	movl	124(%ebx), %edx
	pushl	%eax
	pushl	$8
	pushl	%edx
	call	_dosmemget
	movl	112(%ebx), %eax
	movb	(%eax), %al
	movb	%al, 56(%ebx)
	movl	112(%ebx), %eax
	movb	1(%eax), %al
	movb	%al, 57(%ebx)
	movl	112(%ebx), %eax
	movl	112(%ebx), %edx
	movb	3(%eax), %al
	andl	$255, %eax
	sall	$8, %eax
	movb	2(%edx), %dl
	andl	$255, %edx
	addl	%edx, %eax
	movw	%ax, 58(%ebx)
	movl	112(%ebx), %eax
	movl	112(%ebx), %edx
	movb	5(%eax), %al
	andl	$255, %eax
	sall	$8, %eax
	movb	4(%edx), %dl
	andl	$255, %edx
	addl	%edx, %eax
	movw	%ax, 60(%ebx)
	movl	112(%ebx), %eax
	movl	112(%ebx), %edx
	movb	7(%eax), %al
	andl	$255, %eax
	sall	$8, %eax
	movb	6(%edx), %dl
	andl	$255, %edx
	addl	%edx, %eax
	movw	%ax, 62(%ebx)
	movb	16(%ebx), %al
	addl	$16, %esp
	testb	%al, %al
	je	L365
	movw	92(%ebx), %ax
	subl	$8, %esp
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	movw	92(%ebx), %dx
	addl	$12, %esp
	andl	$57343, %eax
	pushl	%eax
	andl	$65535, %edx
	pushl	%edx
	pushl	$0
	call	_io_Out32
	movw	92(%ebx), %ax
	addl	$8, %esp
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	movw	92(%ebx), %dx
	addl	$12, %esp
	orb	$32, %ah
	pushl	%eax
	andl	$65535, %edx
	pushl	%edx
	pushl	$0
	call	_io_Out32
	movb	$0, 16(%ebx)
	addl	$16, %esp
L365:
	movb	56(%ebx), %al
	cmpb	$33, %al
	je	L368
	cmpb	$33, %al
	ja	L369
	cmpb	$2, %al
	ja	L366
	jmp	L367
L369:
	cmpb	$-128, %al
	jb	L366
	cmpb	$-126, %al
	jbe	L367
	cmpb	$-95, %al
	jne	L366
	jmp	L368
L367:
	movb	57(%ebx), %al
	leal	-5(%eax), %edx
	cmpb	$7, %dl
	ja	L366
	movl	$0, %eax
	movb	%dl, %al
	jmp	*L378(,%eax,4)
	.p2align 2
	.section	.rodata._ZL16EP0_SetupHandlerPV10usb_device,""
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
	.section	.text._ZL16EP0_SetupHandlerPV10usb_device,"x"
L370:
	movl	12(%ebx), %eax
	cmpl	$5, %eax
	je	L366
	movb	56(%ebx), %al
	testb	%al, %al
	js	L366
	movw	58(%ebx), %ax
	cmpw	$127, %ax
	ja	L366
	movw	60(%ebx), %ax
	testw	%ax, %ax
	jne	L366
	movw	62(%ebx), %ax
	testw	%ax, %ax
	jne	L366
	jmp	L393
L371:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL14Get_DescriptorPV10usb_device
	addl	$16, %esp
	jmp	L366
L372:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL14Set_DescriptorPV10usb_device
	addl	$16, %esp
	jmp	L366
L373:
	movb	56(%ebx), %al
	testb	%al, %al
	jns	L366
	movw	58(%ebx), %ax
	testw	%ax, %ax
	jne	L366
	movw	60(%ebx), %ax
	testw	%ax, %ax
	jne	L366
	movw	62(%ebx), %ax
	cmpw	$1, %ax
	jne	L366
	jmp	L394
L397:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL13EP0_InHandlerPV10usb_device
	addl	$16, %esp
	jmp	L366
L374:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL17Set_ConfigurationPV10usb_device
	addl	$16, %esp
	jmp	L366
L375:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL13Get_InterfacePV10usb_device
	addl	$16, %esp
	jmp	L366
L376:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL13Set_InterfacePV10usb_device
	addl	$16, %esp
	jmp	L366
L377:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL11Synch_FramePV10usb_device
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
	jmp	L395
L385:
	cmpb	$34, %al
	je	L383
	cmpb	$35, %al
	jne	L366
	jmp	L396
L395:
	subl	$12, %esp
	pushl	%ebx
	call	__ZL15Set_Line_CodingPV10usb_device
	addl	$16, %esp
	jmp	L366
L382:
	movb	56(%ebx), %al
	testb	%al, %al
	jns	L366
	leal	64(%ebx), %eax
	movl	%eax, 36(%ebx)
	movw	62(%ebx), %ax
	movw	%ax, 44(%ebx)
	movb	$1, 28(%ebx)
	movb	28(%ebx), %al
	testb	%al, %al
	je	L366
	subl	$12, %esp
	pushl	%ebx
	call	__ZL13EP0_InHandlerPV10usb_device
	addl	$16, %esp
	jmp	L366
L383:
	movb	56(%ebx), %al
	testb	%al, %al
	js	L366
	movw	58(%ebx), %ax
	movw	%ax, 72(%ebx)
	movw	72(%ebx), %ax
	testb	$1, %al
	jne	L386
	movl	$5, 12(%ebx)
L386:
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L366
L396:
	movb	56(%ebx), %al
	testb	%al, %al
	js	L366
	movw	58(%ebx), %ax
	movw	%ax, 76(%ebx)
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
L366:
	movw	98(%ebx), %ax
	subl	$4, %esp
	pushl	$-2147483648
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movl	-4(%ebp), %ebx
	leave
	ret
L393:
	movb	58(%ebx), %al
	movb	%al, 2(%ebx)
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%ebx
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	movb	$1, 8(%ebx)
	movb	2(%ebx), %al
	addl	$16, %esp
	cmpb	$1, %al
	sbbl	%eax, %eax
	addl	$4, %eax
	movl	%eax, 12(%ebx)
	jmp	L366
L394:
	movl	$_desc_Config_Set+5, 36(%ebx)
	movw	$1, 44(%ebx)
	movb	$1, 28(%ebx)
	movb	28(%ebx), %al
	testb	%al, %al
	je	L366
	jmp	L397
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
	je	L400
	testb	$1, -16(%ebp)
	je	L401
	movw	86(%esi), %ax
	subl	$4, %esp
	pushl	$130847
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movl	20(%esi), %eax
	addl	$16, %esp
	testl	%eax, %eax
	je	L401
	movb	$0, 2(%esi)
	movb	$0, 8(%esi)
	movw	78(%esi), %ax
	subl	$4, %esp
	pushl	$128
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out8
	movw	98(%esi), %ax
	addl	$12, %esp
	pushl	$-2147483648
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movw	100(%esi), %ax
	addl	$12, %esp
	pushl	$0
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movw	102(%esi), %ax
	addl	$12, %esp
	pushl	$0
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movw	144(%esi), %ax
	addl	$12, %esp
	pushl	$0
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movw	190(%esi), %ax
	addl	$12, %esp
	pushl	$0
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movw	188(%esi), %ax
	addl	$12, %esp
	pushl	$0
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movl	48(%esi), %eax
	movl	%eax, (%esp)
	call	_ClearQueue
	movl	52(%esi), %eax
	movl	%eax, (%esp)
	call	_ClearQueue
	movl	$3, 12(%esi)
	addl	$16, %esp
L401:
	testb	$2, -16(%ebp)
	je	L402
	movw	86(%esi), %ax
	subl	$4, %esp
	pushl	$2
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	8(%esi), %al
	addl	$16, %esp
	testb	%al, %al
	je	L403
	movb	2(%esi), %dl
	movw	78(%esi), %ax
	orl	$-128, %edx
	andl	$255, %edx
	andl	$65535, %eax
	call	__ZL8io_outpbth
	movb	$0, 8(%esi)
L403:
	movl	__ZL12tx_led_count, %eax
	testl	%eax, %eax
	je	L404
	decl	%eax
	movl	%eax, __ZL12tx_led_count
	testl	%eax, %eax
	jne	L404
	call	__ZL10TX_LED_OFFv
L404:
	movl	__ZL12rx_led_count, %eax
	testl	%eax, %eax
	je	L402
	decl	%eax
	movl	%eax, __ZL12rx_led_count
	testl	%eax, %eax
	jne	L402
	call	__ZL10RX_LED_OFFv
L402:
	testb	$4, -16(%ebp)
	je	L405
	movw	86(%esi), %ax
	subl	$4, %esp
	pushl	$4
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movl	$0, 24(%esi)
	movl	$2, 12(%esi)
	call	__ZL10TX_LED_OFFv
	call	__ZL10RX_LED_OFFv
	addl	$16, %esp
L405:
	testb	$8, -16(%ebp)
	je	L406
	movw	86(%esi), %ax
	subl	$4, %esp
	pushl	$8
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L406:
	testb	$16, -16(%ebp)
	je	L407
	movw	86(%esi), %ax
	subl	$4, %esp
	pushl	$16
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L407:
	testl	$256, -16(%ebp)
	je	L408
	movw	86(%esi), %ax
	subl	$4, %esp
	pushl	$256
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movl	112(%esi), %eax
	movl	124(%esi), %edx
	addl	$12, %esp
	pushl	%eax
	pushl	$8
	pushl	%edx
	call	_dosmemget
	movl	112(%esi), %eax
	movb	(%eax), %al
	movb	%al, 56(%esi)
	movl	112(%esi), %eax
	movb	1(%eax), %al
	movb	%al, 57(%esi)
	movl	112(%esi), %eax
	movl	112(%esi), %edx
	movb	3(%eax), %al
	andl	$255, %eax
	sall	$8, %eax
	movb	2(%edx), %dl
	andl	$255, %edx
	addl	%edx, %eax
	movw	%ax, 58(%esi)
	movl	112(%esi), %eax
	movl	112(%esi), %edx
	movb	5(%eax), %al
	andl	$255, %eax
	sall	$8, %eax
	movb	4(%edx), %dl
	andl	$255, %edx
	addl	%edx, %eax
	movw	%ax, 60(%esi)
	movl	112(%esi), %eax
	movl	112(%esi), %edx
	movb	7(%eax), %al
	andl	$255, %eax
	sall	$8, %eax
	movb	6(%edx), %dl
	andl	$255, %edx
	addl	%edx, %eax
	movw	%ax, 62(%esi)
	movb	16(%esi), %al
	addl	$16, %esp
	testb	%al, %al
	je	L409
	movw	92(%esi), %ax
	subl	$8, %esp
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	movw	92(%esi), %dx
	addl	$12, %esp
	andl	$57343, %eax
	pushl	%eax
	andl	$65535, %edx
	pushl	%edx
	pushl	$0
	call	_io_Out32
	movw	92(%esi), %ax
	addl	$8, %esp
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	movw	92(%esi), %dx
	addl	$12, %esp
	orb	$32, %ah
	pushl	%eax
	andl	$65535, %edx
	pushl	%edx
	pushl	$0
	call	_io_Out32
	movb	$0, 16(%esi)
	addl	$16, %esp
L409:
	movb	56(%esi), %al
	cmpb	$33, %al
	je	L412
	cmpb	$33, %al
	ja	L413
	cmpb	$2, %al
	ja	L410
	jmp	L411
L413:
	cmpb	$-128, %al
	jb	L410
	cmpb	$-126, %al
	jbe	L411
	cmpb	$-95, %al
	jne	L410
	jmp	L412
L411:
	movb	57(%esi), %al
	leal	-5(%eax), %edx
	cmpb	$7, %dl
	ja	L410
	movl	$0, %eax
	movb	%dl, %al
	jmp	*L422(,%eax,4)
	.p2align 2
	.section	.rodata._ZL7USB_ISRiPv,""
	.p2align 2
L422:
	.long	L414
	.long	L415
	.long	L416
	.long	L417
	.long	L418
	.long	L419
	.long	L420
	.long	L421
	.section	.text._ZL7USB_ISRiPv,"x"
L414:
	movl	12(%esi), %eax
	cmpl	$5, %eax
	je	L410
	movb	56(%esi), %al
	testb	%al, %al
	js	L410
	movw	58(%esi), %ax
	cmpw	$127, %ax
	ja	L410
	movw	60(%esi), %ax
	testw	%ax, %ax
	jne	L410
	movw	62(%esi), %ax
	testw	%ax, %ax
	jne	L410
	jmp	L460
L415:
	subl	$12, %esp
	pushl	%esi
	call	__ZL14Get_DescriptorPV10usb_device
	addl	$16, %esp
	jmp	L410
L416:
	subl	$12, %esp
	pushl	%esi
	call	__ZL14Set_DescriptorPV10usb_device
	addl	$16, %esp
	jmp	L410
L417:
	movb	56(%esi), %al
	testb	%al, %al
	jns	L410
	movw	58(%esi), %ax
	testw	%ax, %ax
	jne	L410
	movw	60(%esi), %ax
	testw	%ax, %ax
	jne	L410
	movw	62(%esi), %ax
	cmpw	$1, %ax
	jne	L410
	jmp	L461
L464:
	subl	$12, %esp
	pushl	%esi
	call	__ZL13EP0_InHandlerPV10usb_device
	addl	$16, %esp
	jmp	L410
L418:
	subl	$12, %esp
	pushl	%esi
	call	__ZL17Set_ConfigurationPV10usb_device
	addl	$16, %esp
	jmp	L410
L419:
	subl	$12, %esp
	pushl	%esi
	call	__ZL13Get_InterfacePV10usb_device
	addl	$16, %esp
	jmp	L410
L420:
	subl	$12, %esp
	pushl	%esi
	call	__ZL13Set_InterfacePV10usb_device
	addl	$16, %esp
	jmp	L410
L421:
	subl	$12, %esp
	pushl	%esi
	call	__ZL11Synch_FramePV10usb_device
	addl	$16, %esp
	jmp	L410
L412:
	movb	57(%esi), %al
	cmpb	$33, %al
	je	L426
	cmpb	$33, %al
	ja	L429
	cmpb	$32, %al
	jne	L410
	jmp	L462
L429:
	cmpb	$34, %al
	je	L427
	cmpb	$35, %al
	jne	L410
	jmp	L463
L462:
	subl	$12, %esp
	pushl	%esi
	call	__ZL15Set_Line_CodingPV10usb_device
	addl	$16, %esp
	jmp	L410
L426:
	movb	56(%esi), %al
	testb	%al, %al
	jns	L410
	leal	64(%esi), %eax
	movl	%eax, 36(%esi)
	movw	62(%esi), %ax
	movw	%ax, 44(%esi)
	movb	$1, 28(%esi)
	movb	28(%esi), %al
	testb	%al, %al
	je	L410
	subl	$12, %esp
	pushl	%esi
	call	__ZL13EP0_InHandlerPV10usb_device
	addl	$16, %esp
	jmp	L410
L427:
	movb	56(%esi), %al
	testb	%al, %al
	js	L410
	movw	58(%esi), %ax
	movw	%ax, 72(%esi)
	movw	72(%esi), %ax
	testb	$1, %al
	jne	L430
	movl	$5, 12(%esi)
L430:
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%esi
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L410
L463:
	movb	56(%esi), %al
	testb	%al, %al
	js	L410
	movw	58(%esi), %ax
	movw	%ax, 76(%esi)
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%esi
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
L410:
	movw	98(%esi), %ax
	subl	$4, %esp
	pushl	$-2147483648
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L408:
	testl	$512, -16(%ebp)
	je	L431
	movw	86(%esi), %ax
	subl	$4, %esp
	pushl	$512
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	29(%esi), %al
	addl	$16, %esp
	testb	%al, %al
	je	L431
	call	__ZL9RX_LED_ONv
	movw	102(%esi), %ax
	subl	$8, %esp
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	movl	%eax, %ebx
	movw	46(%esi), %ax
	andl	$65535, %eax
	addl	$16, %esp
	cmpl	__ZZL14EP0_OutHandlerPV10usb_deviceE1j, %eax
	jle	L432
	movl	120(%esi), %eax
	movl	132(%esi), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$8
	pushl	%edx
	call	_dosmemget
	andl	$65535, %ebx
	addl	$16, %esp
	testl	%ebx, %ebx
	jle	L432
	movw	46(%esi), %ax
	andl	$65535, %eax
	movl	$0, %ecx
	cmpl	__ZZL14EP0_OutHandlerPV10usb_deviceE1j, %eax
	jg	L433
	jmp	L432
L434:
	movw	46(%esi), %ax
	andl	$65535, %eax
	cmpl	%eax, %edx
	jge	L432
L433:
	movl	40(%esi), %eax
	movl	120(%esi), %edx
	movb	(%edx,%ecx), %dl
	movb	%dl, (%eax)
	incl	%eax
	movl	%eax, 40(%esi)
	incl	%ecx
	movl	__ZZL14EP0_OutHandlerPV10usb_deviceE1j, %eax
	leal	1(%eax), %edx
	movl	%edx, __ZZL14EP0_OutHandlerPV10usb_deviceE1j
	cmpl	%ebx, %ecx
	jl	L434
L432:
	movw	46(%esi), %ax
	andl	$65535, %eax
	cmpl	__ZZL14EP0_OutHandlerPV10usb_deviceE1j, %eax
	jle	L435
	pushl	$-2147483640
	pushl	$1
	pushl	$0
	pushl	%esi
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
	jmp	L431
L435:
	movl	$0, __ZZL14EP0_OutHandlerPV10usb_deviceE1j
	movw	$0, 46(%esi)
	movb	$0, 29(%esi)
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%esi
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
L431:
	testl	$1024, -16(%ebp)
	je	L436
	movw	86(%esi), %ax
	subl	$4, %esp
	pushl	$1024
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	28(%esi), %al
	addl	$16, %esp
	testb	%al, %al
	je	L436
	call	__ZL9TX_LED_ONv
	movw	44(%esi), %ax
	movl	__ZZL13EP0_InHandlerPV10usb_deviceE1j, %edx
	andl	$65535, %eax
	cmpl	%edx, %eax
	jle	L437
	movl	$0, %ebx
L439:
	movw	44(%esi), %ax
	andl	$65535, %eax
	cmpl	__ZZL13EP0_InHandlerPV10usb_deviceE1j, %eax
	jle	L438
	movl	116(%esi), %eax
	movl	36(%esi), %edx
	movb	(%edx), %cl
	movb	%cl, (%eax,%ebx)
	incl	%edx
	movl	%edx, 36(%esi)
	incl	%ebx
	incl	__ZZL13EP0_InHandlerPV10usb_deviceE1j
	cmpl	$8, %ebx
	je	L438
	jmp	L439
L437:
	movw	62(%esi), %ax
	andl	$65535, %eax
	cmpl	%eax, %edx
	jge	L440
	testb	$7, %dl
	jne	L440
	pushl	$-2147483648
	pushl	$0
	pushl	$0
	pushl	%esi
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
L440:
	movl	$0, __ZZL13EP0_InHandlerPV10usb_deviceE1j
	movw	$0, 44(%esi)
	movb	$0, 28(%esi)
	movw	102(%esi), %ax
	subl	$4, %esp
	pushl	$-1610612736
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L436:
	testl	$2048, -16(%ebp)
	je	L441
	movw	86(%esi), %ax
	subl	$4, %esp
	pushl	$2048
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L441:
	testl	$4096, -16(%ebp)
	je	L442
	movw	86(%esi), %ax
	subl	$4, %esp
	pushl	$4096
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	call	__ZL9TX_LED_ONv
	addl	$16, %esp
	cmpb	$0, __ZZL13EP1_InHandlerPV10usb_deviceE9Completed
	je	L443
	movl	$__ZZL13EP1_InHandlerPV10usb_deviceE12notification, 160(%esi)
	movl	172(%esi), %eax
	movl	160(%esi), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$8
	pushl	%edx
	call	_dosmemput
	pushl	$-2147483640
	pushl	$0
	pushl	$1
	pushl	%esi
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	movb	$0, __ZZL13EP1_InHandlerPV10usb_deviceE9Completed
	addl	$32, %esp
	jmp	L442
L443:
	movl	160(%esi), %edx
	movw	74(%esi), %ax
	movb	%al, (%edx)
	movl	160(%esi), %edx
	movw	74(%esi), %ax
	shrw	$8, %ax
	movb	%al, 1(%edx)
	movl	172(%esi), %eax
	movl	160(%esi), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$8
	pushl	%edx
	call	_dosmemput
	pushl	$-2147483646
	pushl	$0
	pushl	$1
	pushl	%esi
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	movb	$1, __ZZL13EP1_InHandlerPV10usb_deviceE9Completed
	addl	$32, %esp
L442:
	testl	$8192, -16(%ebp)
	je	L444
	movw	86(%esi), %ax
	subl	$4, %esp
	pushl	$8192
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	call	__ZL9RX_LED_ONv
	movw	190(%esi), %ax
	addl	$8, %esp
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_In32
	movl	%eax, %ebx
	movl	208(%esi), %eax
	movl	220(%esi), %edx
	addl	$12, %esp
	pushl	%eax
	pushl	$64
	pushl	%edx
	call	_dosmemget
	movl	%ebx, %edi
	andl	$65535, %edi
	addl	$16, %esp
	testl	%edi, %edi
	jle	L445
	movl	$0, %ebx
L446:
	movl	208(%esi), %eax
	movl	48(%esi), %edx
	subl	$8, %esp
	movb	(%eax,%ebx), %al
	andl	$255, %eax
	pushl	%eax
	pushl	%edx
	call	_PushQueue
	incl	%ebx
	addl	$16, %esp
	cmpl	%ebx, %edi
	jg	L446
L445:
	movl	48(%esi), %eax
	movl	8(%eax), %edx
	movl	48(%esi), %eax
	movl	12(%eax), %eax
	subl	$1024, %eax
	cmpl	%eax, %edx
	jae	L444
	pushl	$-2147483584
	pushl	$1
	pushl	$2
	pushl	%esi
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	addl	$16, %esp
L444:
	testl	$16384, -16(%ebp)
	je	L447
	movw	86(%esi), %ax
	subl	$4, %esp
	pushl	$16384
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	movb	$0, 30(%esi)
	movb	30(%esi), %al
	addl	$16, %esp
	testb	%al, %al
	jne	L447
	movl	52(%esi), %eax
	movl	8(%eax), %eax
	testl	%eax, %eax
	je	L447
	call	__ZL9TX_LED_ONv
	movl	$0, %edi
L449:
	movl	204(%esi), %ebx
	movl	52(%esi), %eax
	subl	$12, %esp
	pushl	%eax
	call	_PopQueue
	movb	%al, (%ebx,%edi)
	incl	%edi
	movl	52(%esi), %eax
	movl	8(%eax), %eax
	addl	$16, %esp
	testl	%eax, %eax
	je	L448
	cmpl	$64, %edi
	je	L448
	jmp	L449
L447:
	cmpw	$0, -16(%ebp)
	jns	L450
	movw	86(%esi), %ax
	subl	$4, %esp
	pushl	$32768
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L450:
	testl	$65536, -16(%ebp)
	je	L451
	movw	86(%esi), %ax
	subl	$4, %esp
	pushl	$65536
	andl	$65535, %eax
	pushl	%eax
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
L451:
	movl	64(%esi), %eax
	cmpl	$1200, %eax
	jne	L452
	movw	72(%esi), %ax
	testb	$1, %al
	jne	L452
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
	addl	$12, %esp
	pushl	$90
	pushl	$61978
	pushl	$0
	call	_io_Out8
	addl	$12, %esp
	pushl	$254
	pushl	$100
	pushl	$0
	call	_io_Out8
	movl	$1, %eax
	addl	$16, %esp
	jmp	L400
L452:
	movl	$1, %eax
L400:
	leal	-12(%ebp), %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
L460:
	movb	58(%esi), %al
	movb	%al, 2(%esi)
	pushl	$-1610612736
	pushl	$0
	pushl	$0
	pushl	%esi
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	movb	$1, 8(%esi)
	movb	2(%esi), %al
	addl	$16, %esp
	cmpb	$1, %al
	sbbl	%eax, %eax
	addl	$4, %eax
	movl	%eax, 12(%esi)
	jmp	L410
L461:
	movl	$_desc_Config_Set+5, 36(%esi)
	movw	$1, 44(%esi)
	movb	$1, 28(%esi)
	movb	28(%esi), %al
	testb	%al, %al
	je	L410
	jmp	L464
L438:
	movl	128(%esi), %eax
	movl	116(%esi), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$8
	pushl	%edx
	call	_dosmemput
	movw	100(%esi), %dx
	addl	$12, %esp
	movl	%ebx, %eax
	orl	$-2147483648, %eax
	pushl	%eax
	andl	$65535, %edx
	pushl	%edx
	pushl	$0
	call	_io_Out32
	addl	$16, %esp
	jmp	L436
L448:
	movl	216(%esi), %eax
	movl	204(%esi), %edx
	subl	$4, %esp
	pushl	%eax
	pushl	$64
	pushl	%edx
	call	_dosmemput
	movl	%edi, %eax
	orl	$-2147483648, %eax
	pushl	%eax
	pushl	$0
	pushl	$2
	pushl	%esi
	call	__ZL9SetEPnDLRPV10usb_deviceiim
	movb	$1, 30(%esi)
	addl	$32, %esp
	jmp	L447
	.section	.text._ZL15USB_CDC_RequestPV10usb_device,"x"
__ZL15USB_CDC_RequestPV10usb_device:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	movl	8(