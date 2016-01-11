/*
  OSAbstract.cpp - A OSAbstract Generator Library for 86Duino
  Copyright (c) 2015 Android Lin <acen@dmp.com.tw>. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "dmpcfg.h"
#include "OSAbstract.h"
#include "string.h"
#include "stdio.h"

#if defined (DMP_LINUX)

#define GPIONUM    (10)
pthread_spinlock_t LKGPIO0;
pthread_spinlock_t LKGPIO1;
pthread_spinlock_t LKGPIO2;
pthread_spinlock_t LKGPIO3;
pthread_spinlock_t LKGPIO4;
pthread_spinlock_t LKGPIO5;
pthread_spinlock_t LKGPIO6;
pthread_spinlock_t LKGPIO7;
pthread_spinlock_t LKGPIO8;
pthread_spinlock_t LKGPIO9;

pthread_spinlock_t gpio_lock_array[GPIONUM];

pthread_spinlock_t LKADC;

#define MCM_MCMD_NUM    (12)
pthread_spinlock_t LKMC0MD0;
pthread_spinlock_t LKMC0MD1;
pthread_spinlock_t LKMC0MD2;
pthread_spinlock_t LKMC1MD0;
pthread_spinlock_t LKMC1MD1;
pthread_spinlock_t LKMC1MD2;
pthread_spinlock_t LKMC2MD0;
pthread_spinlock_t LKMC2MD1;
pthread_spinlock_t LKMC2MD2;
pthread_spinlock_t LKMC3MD0;
pthread_spinlock_t LKMC3MD1;
pthread_spinlock_t LKMC3MD2;
pthread_spinlock_t LKMCGENAL;
pthread_spinlock_t LKMCSIF;

pthread_spinlock_t mcm_lock_arary[MCM_MCMD_NUM];

pthread_spinlock_t LKSPI;
pthread_spinlock_t LKI2C;
pthread_spinlock_t LKCMOS;

#endif

DMPAPI(void) spinLockInit(void) {
#if defined (DMP_LINUX)
	int err = 0;

	gpio_lock_array[0] = LKGPIO0; gpio_lock_array[1] = LKGPIO1; gpio_lock_array[2] = LKGPIO2;
	gpio_lock_array[3] = LKGPIO3; gpio_lock_array[4] = LKGPIO4; gpio_lock_array[5] = LKGPIO5;
	gpio_lock_array[6] = LKGPIO6; gpio_lock_array[7] = LKGPIO7; gpio_lock_array[8] = LKGPIO8;
	gpio_lock_array[9] = LKGPIO9;

	mcm_lock_arary[0] = LKMC0MD0; mcm_lock_arary[1] = LKMC0MD1; mcm_lock_arary[2] = LKMC0MD2;
	mcm_lock_arary[3] = LKMC1MD0; mcm_lock_arary[4] = LKMC1MD1; mcm_lock_arary[5] = LKMC1MD2;
	mcm_lock_arary[6] = LKMC2MD0; mcm_lock_arary[7] = LKMC2MD1; mcm_lock_arary[8] = LKMC2MD2;
	mcm_lock_arary[9] = LKMC3MD0; mcm_lock_arary[10] = LKMC3MD1; mcm_lock_arary[11] = LKMC3MD2;

	for(int i=0; i<GPIONUM; i++)
	{
		err = pthread_spin_init(&gpio_lock_array[i], PTHREAD_PROCESS_SHARED);
		if(err != 0) printf("spinLockGPIO: %s\n", strerror(err));
	}

	err = pthread_spin_init(&LKADC, PTHREAD_PROCESS_SHARED);
	if(err != 0) printf("spinLockADC: %s\n", strerror(err));

	for(int i=0; i<MCM_MCMD_NUM; i++)
	{
		err = pthread_spin_init(&mcm_lock_arary[i], PTHREAD_PROCESS_SHARED);
		if(err != 0) printf("spinLockMCMD: %s\n", strerror(err));
	}

	err = pthread_spin_init(&LKMCGENAL, PTHREAD_PROCESS_SHARED);
	if(err != 0) printf("spinLockMCGENAL: %s\n", strerror(err));

	err = pthread_spin_init(&LKMCSIF, PTHREAD_PROCESS_SHARED);
	if(err != 0) printf("spinLockMCSIF: %s\n", strerror(err));

	err = pthread_spin_init(&LKSPI, PTHREAD_PROCESS_SHARED);
	if(err != 0) printf("spinLockSPI: %s\n", strerror(err));

	err = pthread_spin_init(&LKI2C, PTHREAD_PROCESS_SHARED);
	if(err != 0) printf("spinLockI2C: %s\n", strerror(err));

	err = pthread_spin_init(&LKCMOS, PTHREAD_PROCESS_SHARED);
	if(err != 0) printf("spinLockCMOS: %s\n", strerror(err));
	// ... Other spinlock

#endif
}

DMPAPI(int) lockGPIO(int n) {
#if defined (DMP_LINUX)
	return pthread_spin_lock(&gpio_lock_array[n]);
#endif
	return 0;
}

DMPAPI(int) tryLockGPIO(int n) {
#if defined (DMP_LINUX)
	return pthread_spin_trylock(&gpio_lock_array[n]);
#endif
	return 0;
}

DMPAPI(int) unLockGPIO(int n) {
#if defined (DMP_LINUX)
	return pthread_spin_unlock(&gpio_lock_array[n]);
#endif
	return 0;
}

DMPAPI(int) lockADC(void) {
#if defined (DMP_LINUX)
	return pthread_spin_lock(&LKADC);
#endif
	return 0;
}

DMPAPI(int) tryLockADC(void) {
#if defined (DMP_LINUX)
	return pthread_spin_trylock(&LKADC);
#endif
	return 0;
}

DMPAPI(int) unLockADC(void) {
#if defined (DMP_LINUX)
	return pthread_spin_unlock(&LKADC);
#endif
	return 0;
}

DMPAPI(int) lockMCM(int mc, int md) {
#if defined (DMP_LINUX)
	int index;
	if(mc < 0 || mc > 3 || md < 0 || md > 2) return 0xffff;

	index = mc*3 + md;
	return pthread_spin_lock(&mcm_lock_arary[index]);
#endif
	return 0;
}

DMPAPI(int) tryLockMCM(int mc, int md) {
#if defined (DMP_LINUX)
	int index;
	if(mc < 0 || mc > 3 || md < 0 || md > 2) return 0xffff;

	index = mc*3 + md;
	return pthread_spin_trylock(&mcm_lock_arary[index]);
#endif
	return 0;
}

DMPAPI(int) unLockMCM(int mc, int md) {
#if defined (DMP_LINUX)
    int index;
	if(mc < 0 || mc > 3 || md < 0 || md > 2) return 0xffff;

	index = mc*3 + md;
	return pthread_spin_unlock(&mcm_lock_arary[index]);
#endif
	return 0;
}

DMPAPI(int) lockMCMGENAL(void) {
#if defined (DMP_LINUX)
	return pthread_spin_lock(&LKMCGENAL);
#endif
	return 0;
}

DMPAPI(int) tryLockMCMGENAL(void) {
#if defined (DMP_LINUX)
	return pthread_spin_trylock(&LKMCGENAL);
#endif
	return 0;
}

DMPAPI(int) unLockMCMGENAL(void) {
#if defined (DMP_LINUX)
	return pthread_spin_unlock(&LKMCGENAL);
#endif
	return 0;
}

DMPAPI(int) lockMCMSIF(void) {
#if defined (DMP_LINUX)
	return pthread_spin_lock(&LKMCSIF);
#endif
	return 0;
}

DMPAPI(int) tryLockMCMSIF(void) {
#if defined (DMP_LINUX)
	return pthread_spin_trylock(&LKMCSIF);
#endif
	return 0;
}

DMPAPI(int) unLockMCMSIF(void) {
#if defined (DMP_LINUX)
	return pthread_spin_unlock(&LKMCSIF);
#endif
	return 0;
}

DMPAPI(int) lockSPI(void) {
#if defined (DMP_LINUX)
	return pthread_spin_lock(&LKSPI);
#endif
	return 0;
}

DMPAPI(int) tryLockSPI(void) {
#if defined (DMP_LINUX)
	return pthread_spin_trylock(&LKSPI);
#endif
	return 0;
}

DMPAPI(int) unLockSPI(void) {
#if defined (DMP_LINUX)
	return pthread_spin_unlock(&LKSPI);
#endif
	return 0;
}

DMPAPI(int) lockI2C(void) {
#if defined (DMP_LINUX)
	return pthread_spin_lock(&LKI2C);
#endif
	return 0;
}

DMPAPI(int) tryLockI2C(void) {
#if defined (DMP_LINUX)
	return pthread_spin_trylock(&LKI2C);
#endif
	return 0;
}

DMPAPI(int) unLockI2C(void) {
#if defined (DMP_LINUX)
	return pthread_spin_unlock(&LKI2C);
#endif
	return 0;
}

DMPAPI(int) lockCMOS(void) {
#if defined (DMP_LINUX)
	return pthread_spin_unlock(&LKCMOS);
#endif
	return 0;
}

DMPAPI(int) tryLockCMOS(void) {
#if defined (DMP_LINUX)
	return pthread_spin_trylock(&LKCMOS);
#endif
	return 0;
}

DMPAPI(int) unLockCMOS(void) {
#if defined (DMP_LINUX)
	return pthread_spin_unlock(&LKCMOS);
#endif
	return 0;
}
