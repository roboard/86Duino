#ifndef _OSABSTRACT_h
#define _OSABSTRACT_h

#if defined (DMP_LINUX)
	#include <pthread.h>
	#define OSSPIN                  pthread_spinlock_t
	#define OSSPININIT(var)         pthread_spin_init(&var, PTHREAD_PROCESS_SHARED)
	#define OSSPINLOCK(var)         pthread_spin_lock(&var)
	#define OSSPINTRYLOCK(var)      pthread_spin_trylock(&var)
	#define OSSPINUNLOCK(var)       pthread_spin_unlock(&var)
#else
	#define OSSPIN int
	#define OSSPININIT(var)
	#define OSSPINLOCK(var)
	#define OSSPINTRYLOCK(var)
	#define OSSPINUNLOCK(var)
#endif

DMPAPI(void) spinLockInit(void);

DMPAPI(int) lockGPIO(int n);
DMPAPI(int) tryLockGPIO(int n);
DMPAPI(int) unLockGPIO(int n);

DMPAPI(int) lockADC(void);
DMPAPI(int) tryLockADC(void);
DMPAPI(int) unLockADC(void);

DMPAPI(int) lockMCM(int mc, int md);
DMPAPI(int) tryLockMCM(int mc, int md);
DMPAPI(int) unLockMCM(int mc, int md);

DMPAPI(int) lockMCMGENAL(void);
DMPAPI(int) tryLockMCMGENAL(void);
DMPAPI(int) unLockMCMGENAL(void);

DMPAPI(int) lockMCMSIF(void);
DMPAPI(int) tryLockMCMSIF(void);
DMPAPI(int) unLockMCMSIF(void);

#define MCGENAL    (12)
#define MCSIF      (13)

DMPAPI(int) lockSPI(void);
DMPAPI(int) tryLockSPI(void);
DMPAPI(int) unLockSPI(void);

DMPAPI(int) lockI2C(void);
DMPAPI(int) tryLockI2C(void);
DMPAPI(int) unLockI2C(void);

DMPAPI(int) lockCMOS(void);
DMPAPI(int) tryLockCMOS(void);
DMPAPI(int) unLockCMOS(void);
#endif
