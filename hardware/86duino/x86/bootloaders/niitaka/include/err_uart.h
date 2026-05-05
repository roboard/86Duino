// Template of err_uart.h

static void dbur_Init(void) {

    // initialize the debug serial port

}


static void dbur_WriteString(char* dbgmsg) {  // should be usable in ISR

    // send the debug-message string, dbgmsg, to the debug serial port

} DPMI_END_OF_LOCKED_STATIC_FUNC(dbur_WriteString)


static unsigned int dbur_ReadByte(void) {  // should be usable in ISR
    unsigned int ch = 0;

    // read a byte from the debug serial port and return it
    // if no byte to read then return 0xffff

    return ch;
} DPMI_END_OF_LOCKED_STATIC_FUNC(dbur_ReadByte)


static void dbur_Close(void) {

    // close the debug serial port

}
