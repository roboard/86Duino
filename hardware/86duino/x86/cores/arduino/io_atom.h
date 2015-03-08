/*******************************************************************************

  io_atom.h - Part of DM&P Vortex86 Base I/O Library
  Copyright (c) 2014 AAA <aaa@dmp.com.tw>. All right reserved.

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

  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)

*******************************************************************************/


/************************  Atomic Variable System  ****************************/
#if defined(DMP_DOS_DJGPP) || defined(DMP_LINUX)

    // compiler memory barrier
    #define ATOMIC_MEM_BARRIER() __asm__ __volatile__("" : : : "memory")
    //#define ATOMIC_MEM_BARRIER() __asm__ __volatile__("mfence" : : : "memory")

    typedef volatile int ATOMIC_INT_t;

    #define ATOMIC_INT_INIT(val)    (val)
    #define ATOMIC_INT_GET(x)       (*(x))
    #define ATOMIC_INT_SET(x,val)   do { *(x) = (val); } while(0)
    #define ATOMIC_INT_TEST(x,val)  ((*(x)) == (val))

    __dmp_inline(void) ATOMIC_INT_INC(ATOMIC_INT_t* x) {
        __asm__ volatile (
            " lock;                     \n\t"
            " incl %0;                  \n\t"
            : "=m" (*x)
            : "m" (*x)
            : "memory");
    }

    __dmp_inline(void) ATOMIC_INT_DEC(ATOMIC_INT_t* x) {
        __asm__ volatile (
            " lock;                     \n\t"
            " decl %0;                  \n\t"
            : "=m" (*x)
            : "m" (*x)
            : "memory");
    }

    __dmp_inline(bool) ATOMIC_INT_INC_TEST(ATOMIC_INT_t* x) {
        unsigned char result;

        __asm__ volatile (
            " lock;                     \n\t"
            " incl %0;                  \n\t"
            " sete %1;                  \n\t"
            : "=m" (*x), "=q" (result)
            : "m" (*x)
            : "memory");
        
        if (result != 0) return true; else return false;  // return true if ++x == 0
    }

    __dmp_inline(bool) ATOMIC_INT_DEC_TEST(ATOMIC_INT_t* x) {
        unsigned char result;

        __asm__ volatile (
            " lock;                     \n\t"
            " decl %0;                  \n\t"
            " sete %1;                  \n\t"
            : "=m" (*x), "=q" (result)
            : "m" (*x)
            : "memory");
        
        if (result != 0) return true; else return false;  // return true if --x == 0
    }

#elif defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
    #define ATOMIC_MEM_BARRIER()

    typedef volatile int ATOMIC_INT_t;

    #define ATOMIC_INT_INIT(val)    (val)
    #define ATOMIC_INT_GET(x)       (*(x))
    #define ATOMIC_INT_SET(x,val)   do { *(x) = (val); } while(0)
    #define ATOMIC_INT_TEST(x,val)  ((*(x)) == (val))
    #define ATOMIC_INT_INC(x)       do { (*(x))++; } while(0)
    #define ATOMIC_INT_DEC(x)       do { (*(x))--; } while(0)
    #define ATOMIC_INT_INC_TEST(x)  ((++(*(x))) != 0)
    #define ATOMIC_INT_DEC_TEST(x)  ((--(*(x))) != 0)

#elif defined(DMP_WINDOWS)
    #define ATOMIC_MEM_BARRIER() MemoryBarrier()

    typedef volatile int ATOMIC_INT_t;

    #define ATOMIC_INT_INIT(val)    (val)
    #define ATOMIC_INT_GET(x)       (*(x))
    #define ATOMIC_INT_SET(x,val)   do { *(x) = (val); } while(0)
    #define ATOMIC_INT_TEST(x,val)  ((*(x)) == (val))

    #define ATOMIC_INT_INC(x)       InterlockedIncrement((LONG*)(x))
    #define ATOMIC_INT_DEC(x)       InterlockedDecrement((LONG*)(x))
    #define ATOMIC_INT_INC_TEST(x)  (InterlockedIncrement((LONG*)(x)) != 0)
    #define ATOMIC_INT_DEC_TEST(x)  (InterlockedDecrement((LONG*)(x)) != 0)

#else
    #error unsupported platforms to define atomic operations...
#endif
/*--------------------  end of Atomic Variable System  -----------------------*/
