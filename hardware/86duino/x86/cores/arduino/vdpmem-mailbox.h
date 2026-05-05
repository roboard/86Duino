#ifndef __VIRTUAL_DPMEM_MAILBOX_H
#define __VIRTUAL_DPMEM_MAILBOX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t VDPM_MAILBOX_t;

/* Enable Mailbox function. */
int vdpm_EnableMailbox();
/* Disable Mailbox function. */
int vdpm_DisableMailbox();

/* Create/Get Mailbox */
VDPM_MAILBOX_t vdpm_CreateMailbox(VDPM_HANDLE_t mem, uint32_t offset, size_t mbox_size);
VDPM_MAILBOX_t vdpm_GetMailbox(VDPM_HANDLE_t mem, uint32_t offset);
/* Get Mailbox Memory Size */
uint32_t vdpm_GetMailboxSize(VDPM_MAILBOX_t mbox);
/* Destory Mailbox */
void vdpm_DeleteMailbox(VDPM_MAILBOX_t mbox);

/* Set send completed callback function for Mailbox. */
bool vdpm_Mailbox_RegisterSendCompletedCB(VDPM_MAILBOX_t mbox, void (*callback)(VDPM_MAILBOX_t));
/* Set receive callback function for Mailbox. */
bool vdpm_Mailbox_RegisterReceivedCB(VDPM_MAILBOX_t mbox, void (*callback)(VDPM_MAILBOX_t));

/* Check the mailbox status. */
#define VDPM_MAILBOX_EMPTY  (0)
#define VDPM_MAILBOX_FULL   (1)
int vdpm_Mailbox_Status(VDPM_MAILBOX_t mbox);

/* Send mailbox. */
int vdpm_Mailbox_Send(VDPM_MAILBOX_t mbox, uint8_t *buf, int timeout);
/* Receive mailbox. */
int vdpm_Mailbox_Recv(VDPM_MAILBOX_t mbox, uint8_t *buf, int timeout);

/* Clear the mailbox. */
void vdpm_Mailbox_Reset(VDPM_MAILBOX_t mbox);

/* Check Mailbox with Fault */
int vdpm_VerifyMailbox(VDPM_MAILBOX_t mbox);
/* Recover Mailbox */
int vdpm_RecoverMailbox(VDPM_MAILBOX_t mbox);

#ifdef __cplusplus
}
#endif

#endif
