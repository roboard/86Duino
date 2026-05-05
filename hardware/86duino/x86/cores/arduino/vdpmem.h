#ifndef __VIRTUAL_DPMEM_H
#define __VIRTUAL_DPMEM_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Virtual DPMEM Debug Mode for Developer. */
// #define VDPM_DEBUG

#define VDPM_NULL   (0)
#define VDPM_PHY_ADDR(mem, offset)  (mem + offset)

typedef uint32_t VDPM_HANDLE_t;
typedef uint32_t VDPM_MUTEX_t;
typedef uint32_t VDPM_SYNCBUF_t;
typedef uint32_t VDPM_MSGQUEUE_t;
typedef uint32_t VDPM_MEMPOOL_t;
typedef uint32_t VDPM_TRACEBUF_t;

/* Virtual DPMEM Library initialize */
int vdpm_Initialize(uint32_t size);
/* Virtual DPMEM Library deinitialize */
void vdpm_Shutdown();
/* Virtual DPMEM Library reinitialize */
int vdpm_Reinitialize();
/* Virtual DPMEM Delete System Segment */
int vdpm_DestroyAll();

/* Get the Shared Memory Zone Handle */
VDPM_HANDLE_t vdpm_GetSMZoneHandle();
/* Get the Shared Memory Zone Size */
uint32_t vdpm_GetSMZoneSize();

/* Get Free Memory Size. */
uint32_t vdpm_GetMemFreeSize();

/* Alloc Virtual DPMEM */
VDPM_HANDLE_t vdpm_Malloc(uint32_t size);
/* Get Virtual DPMEM address of the Memory Handle */
uint32_t vdpm_GetMemAddr(VDPM_HANDLE_t vdpm_handle);
/* Get Memory Handle of the Virtual DPMEM address */
VDPM_HANDLE_t vdpm_GetMemHandle(uint32_t vdpm_addr);
/* Free the Virtual DPMEM */
int vdpm_Free(VDPM_HANDLE_t mem);

/* Get common Mutex: Index 0 ~ 31 */
VDPM_MUTEX_t vdpm_GetCommonMutex(uint32_t index);
/* Create/Get one Virtual DPMEM Mutex */
VDPM_MUTEX_t vdpm_CreateMutex(VDPM_HANDLE_t mem, uint32_t offset);
VDPM_MUTEX_t vdpm_GetMutex(VDPM_HANDLE_t mem, uint32_t offset);
/* Destory one Virtual DPMEM Mutex */
void vdpm_DeleteMutex(VDPM_MUTEX_t lock);
/* Virtual DPMEM Mutex Try Lock */
int vdpm_MutexTryLock(VDPM_MUTEX_t lock);
/* Virtual DPMEM Mutex Lock */
void vdpm_MutexLock(VDPM_MUTEX_t lock);
/* Virtual DPMEM Mutex Unlock */
void vdpm_MutexUnlock(VDPM_MUTEX_t lock);
/* Check Peterson's Algorithm Lock Owner */
int vdpm_MutexOwner(VDPM_MUTEX_t lock);
#define VDPM_MUTEX_NO_OWNER (0)
#define VDPM_MUTEX_IS_MINE  (1)
#define VDPM_MUTEX_IS_YOURS (2)

/* Create/Get one Sync Buffer Space */
VDPM_SYNCBUF_t vdpm_CreateSyncBuffer(VDPM_HANDLE_t mem, uint32_t offset, uint32_t buf1_addr, uint32_t buf2_addr, uint32_t buf3_addr);
VDPM_SYNCBUF_t vdpm_GetSyncBuffer(VDPM_HANDLE_t mem, uint32_t offset);
/* Destory one Sync Buffer Space */
void vdpm_DeleteSyncBuffer(VDPM_SYNCBUF_t sync);
/* Sync Buffer Write Begin/End */
uint32_t vdpm_SyncWrite_Begin(VDPM_SYNCBUF_t sync);
bool vdpm_SyncWrite_End(VDPM_SYNCBUF_t sync);
/* Sync Buffer Read Begin/End */
uint32_t vdpm_SyncRead_Begin(VDPM_SYNCBUF_t sync);
void vdpm_SyncRead_End(VDPM_SYNCBUF_t sync);
/* Check Sync Buffer with Fault */
int vdpm_VerifySyncBuffer(VDPM_SYNCBUF_t sync);
/* Recover Sync Buffer */
int vdpm_RecoverSyncBuffer(VDPM_SYNCBUF_t sync);

/* Create/Get Message Queue */
VDPM_MSGQUEUE_t vdpm_CreateMsgQueue(VDPM_HANDLE_t mem, uint32_t offset, size_t msg_size, size_t num_of_msg);
VDPM_MSGQUEUE_t vdpm_GetMsgQueue(VDPM_HANDLE_t mem, uint32_t offset);
/* Get Message Queue Memory Size */
uint32_t vdpm_GetMsgQueueSize(VDPM_MSGQUEUE_t msgQueue);
/* Destory Message Queue */
void vdpm_DeleteMsgQueue(VDPM_MSGQUEUE_t msgQueue);
/* Message Queue Push Begin/End */
uint32_t vdpm_PushMessage_Begin(VDPM_MSGQUEUE_t msgQueue);
bool vdpm_PushMessage_End(VDPM_MSGQUEUE_t msgQueue, uint32_t msg);
/* Message Queue Pop Begin/End */
uint32_t vdpm_PopMessage_Begin(VDPM_MSGQUEUE_t msgQueue);
bool vdpm_PopMessage_End(VDPM_MSGQUEUE_t msgQueue, uint32_t msg);
/* Check Message Queue is empty or not. */
bool vdpm_MsgQueueIsEmpty(VDPM_MSGQUEUE_t msgQueue);
/* Check Message Queue is full or not. */
bool vdpm_MsgQueueIsFull(VDPM_MSGQUEUE_t msgQueue);
/* Clear Message Queue */
bool vdpm_ClearMsgQueue(VDPM_MSGQUEUE_t msgQueue);
/* Check Message Queue with Fault */
int vdpm_VerifyMsgQueue(VDPM_MSGQUEUE_t msgQueue);
/* Recover Message Queue */
int vdpm_RecoverMsgQueue(VDPM_MSGQUEUE_t msgQueue);

/* Create/Get Memory Pool */
VDPM_MEMPOOL_t vdpm_CreateMemPool(VDPM_HANDLE_t mem, uint32_t offset, size_t block_size, size_t num_of_block);
VDPM_MEMPOOL_t vdpm_GetMemPool(VDPM_HANDLE_t mem, uint32_t offset);
/* Get Memory Pool Size */
uint32_t vdpm_GetMemPoolSize(VDPM_MEMPOOL_t pool);
/* Destory Memory Pool */
void vdpm_DeleteMemPool(VDPM_MEMPOOL_t pool);
/* Get Block from Memory Pool */
uint32_t vdpm_MemPool_Alloc(VDPM_MEMPOOL_t pool);
/* Free Block to Memory Pool */
bool vdpm_MemPool_Free(VDPM_MEMPOOL_t pool, uint32_t block);
/* Check Memory Pool with Fault */
int vdpm_VerifyMemPool(VDPM_MEMPOOL_t pool);
/* Recover Memory Pool */
int vdpm_RecoverMemPool(VDPM_MEMPOOL_t pool);

/* Fill block of Virtual Dual Port Memory  */
void vdpm_MemSet(VDPM_HANDLE_t mem, uint32_t offset, uint8_t value, size_t len);
/* Copy Buffer to Virtual Dual Port Memory */
void vdpm_MemWrite(VDPM_HANDLE_t mem, uint32_t offset, uint8_t *buf, size_t len);
/* Copy Virtual Dual Port Memory to Buffer */
void vdpm_MemRead(VDPM_HANDLE_t mem, uint32_t offset, uint8_t *buf, size_t len);

/* Virtual DPMEM Read String Function */
void vdpm_ReadString(VDPM_HANDLE_t mem, uint32_t offset, char *buf);
/* Virtual DPMEM Write String Function */
void vdpm_WriteString(VDPM_HANDLE_t mem, uint32_t offset, char *buf);

/* Virtual DPMEM Read Byte Function */
uint8_t vdpm_ReadByte(VDPM_HANDLE_t mem, uint32_t offset);
/* Virtual DPMEM Read Word Function */
uint16_t vdpm_ReadWord(VDPM_HANDLE_t mem, uint32_t offset);
/* Virtual DPMEM Read Double Word Function */
uint32_t vdpm_ReadDword(VDPM_HANDLE_t mem, uint32_t offset);
/* Virtual DPMEM Read Quad Word Function */
uint64_t vdpm_ReadQword(VDPM_HANDLE_t mem, uint32_t offset);
/* Virtual DPMEM Read Float Function */
float vdpm_ReadFloat(VDPM_HANDLE_t mem, uint32_t offset);
/* Virtual DPMEM Read Double Function */
double vdpm_ReadDouble(VDPM_HANDLE_t mem, uint32_t offset);

/* Virtual DPMEM Write Byte Function */
void vdpm_WriteByte(VDPM_HANDLE_t mem, uint32_t offset, uint8_t val);
/* Virtual DPMEM Write Word Function */
void vdpm_WriteWord(VDPM_HANDLE_t mem, uint32_t offset, uint16_t val);
/* Virtual DPMEM Write Double Word Function */
void vdpm_WriteDword(VDPM_HANDLE_t mem, uint32_t offset, uint32_t val);
/* Virtual DPMEM Write Quad Word Function */
void vdpm_WriteQword(VDPM_HANDLE_t mem, uint32_t offset, uint64_t val);
/* Virtual DPMEM Write Float Function */
void vdpm_WriteFloat(VDPM_HANDLE_t mem, uint32_t offset, float val);
/* Virtual DPMEM Write Double Function */
void vdpm_WriteDouble(VDPM_HANDLE_t mem, uint32_t offset, double val);

/* Get Common Trace Buffer Space */
VDPM_TRACEBUF_t vdpm_GetCommonTraceBuffer();
/* Create/Get Trace Buffer Space */
VDPM_TRACEBUF_t vdpm_CreateTraceBuffer(VDPM_HANDLE_t mem, uint32_t offset, size_t num_of_msg);
VDPM_TRACEBUF_t vdpm_GetTraceBuffer(VDPM_HANDLE_t mem, uint32_t offset);
/* Get Trace Buffer Size */
uint32_t vdpm_GetTraceBufferSize(VDPM_TRACEBUF_t trace);
/* Destory one Trace Buffer Space */
void vdpm_DeleteTraceBuffer(VDPM_TRACEBUF_t trace);
/* Trace Buffer Printf Function */
void vdpm_TraceBuffer_Printf(VDPM_TRACEBUF_t trace, const char *fmt, ...);
/* Trace Buffer Dump Function */
bool vdpm_TraceBuffer_Dump2Buf(VDPM_TRACEBUF_t trace, char* buf);  // sizeof(buf) must >= 128
bool vdpm_TraceBuffer_Dump(VDPM_TRACEBUF_t trace);
void vdpm_TraceBuffer_DumpAll(VDPM_TRACEBUF_t trace);
/* Check Trace Buffer with Fault */
int vdpm_VerifyTraceBuffer(VDPM_TRACEBUF_t trace);
/* Recover Trace Buffer */
int vdpm_RecoverTraceBuffer(VDPM_TRACEBUF_t trace);

#ifdef __cplusplus
}
#endif

#endif
