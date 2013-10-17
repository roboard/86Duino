#ifndef __COMMON_H
#define __COMMON_H

#include <stdlib.h>
#include "dmpcfg.h"


#ifdef __cplusplus
extern "C" {
#endif

#define ERR_NOERROR    (0)
DMPAPI(int)   dmpio_GetErrCode(void);
DMPAPI(char*) dmpio_GetErrMsg(void);

DMPAPI(bool) err_SetLogFile(char* logfile);
DMPAPI(void) err_CloseLogFile(void);

#ifdef __cplusplus
}
#endif 


#if defined(USE_COMMON)
#ifdef __cplusplus
extern "C" {
#endif

    //ERROR MESSAGE:
    DMPAPI_C(void) errmsg(char* fmt, ...);
    DMPAPI_C(void) errmsg_exit(char* fmt, ...);

    //common blackboard for ERROR responses
    DMPAPI_C(void) err_SetMsg(int errtype, char* fmt, ...);

    //TIMER:
    DMPAPI(unsigned long) timer_nowtime(void);
    DMPAPI(void) delay_ms(unsigned long delaytime);
    DMPAPI(void) delay_us(unsigned long delaytime);
    DMPAPI(unsigned long long int) getclocks64(void);

#ifdef __cplusplus
}
#endif 
#endif

#endif

