#ifndef __COPROC_CONFIG_H
#define __COPROC_CONFIG_H

/* CoProc Version. */
#define COPROC_VERSION_NUMBER   (0x00000001UL)
/* CoProc initial port number in IPC-NET. */
#define COPROC_INIT_PORT        (252)
/* CoProc Maximum Resource Table Entry. */
#define COPROC_MAX_RES_ENTRY    (16)

#if defined(__COPROC_SERVER)

    /* Virtual DPMEM Initialize Size. (Unit: KB) */
    #define COPROC_VDPM_SIZE              (16 * 1024UL)
    /* Accept Time Out. */
    #define COPROC_TIMEOUT_MS_ACCEPT      (5 * 60 * 1000UL)
    /* Send CoProc ID and Version Time Out. */
    #define COPROC_TIMEOUT_MS_SEND_VER    (10UL)
    /* In Ready State Time Out. */
    #define COPROC_TIMEOUT_MS_READY       (500UL)
    /* Send Resource Table Time Out. */
    #define COPROC_TIMEOUT_MS_SEND_RES    (100UL)
    
#elif defined(__COPROC_CLIENT)
    
    /* Connect Time Out. */
    #define COPROC_TIMEOUT_MS_CONNECT     (5 * 60 * 1000UL)
    /* Receive CoProc ID and Version Time Out. */
    #define COPROC_TIMEOUT_MS_RECV_VER    (10UL)
    /* Send Command Time Out. */
    #define COPROC_TIMEOUT_MS_SEND_CMD    (100UL)
    /* Receive Resource Table Time Out. */
    #define COPROC_TIMEOUT_MS_RECV_RES    (100UL)
    /* Wait Remote Close Time Out. */
    #define COPROC_TIMEOUT_MS_WAIT_CLOSE  (100UL)
    
#else
    #error No defined __COPROC_SERVER or __COPROC_CLIENT.
#endif


#endif
