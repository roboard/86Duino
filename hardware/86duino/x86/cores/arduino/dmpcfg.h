/*
################################################################################
    
  configuration file to set OS, platform, common marcos
    
################################################################################
*/
#ifndef __DMPCFG_H
#define __DMPCFG_H


/*************************  Platform Information  *****************************/
#if (defined(__BORLANDC__) || defined(__TURBOC__)) && !defined(__WIN32__)
    #define DMP_DOS_BC

    #if defined(__BORLANDC__) && ((__BORLANDC__ == 0x0400) || (__BORLANDC__ == 0x0410) || (__BORLANDC__ == 0x0452))
        #define DMP_DOS_BC30
    #endif

    #if defined(DMP_DOS_BC30) || (defined(__TURBOC__) && !defined(__BORLANDC__))
        typedef int bool;
        #define true  (1==1)
        #define false (1==0)
    
        #define __FUNCB__(x)    #x              // for stringizing __LINE__
        #define __FUNCA__(x)    __FUNCB__(x)    // for stringizing __LINE__
        #define __FUNCTION__    (__FILE__ "[line " __FUNCA__(__LINE__) "]")
    #endif
#endif

#if defined (DJGPP)
    #define DMP_DOS_DJGPP
    
    #define DMP_DOS_DJGPP_NO_VIRMEM  // set this if using a DPMI server that doesn't enable virtual memory
    #define DMP_DOS_DJGPP_NO_EMM386  // set this if using a DOS environment without EMM386 (needed by the DMA buffer allocation codes)
#endif

#if (defined (__WATCOMC__) || defined(__WATCOM_CPLUSPLUS__)) && defined(__386__) //&& defined(__i386__)
    #define DMP_DOS_WATCOM
#endif

#if defined(linux) && defined(__GNUC__)
    #define DMP_LINUX
#endif

#if defined(_MSC_VER) //&& defined(_M_IX86)
    #if defined   (WINCE)
        #define DMP_WINCE_MSVC
    #elif defined (WIN32)
        #define DMP_WIN32_MSVC

        #define USE_WINIO3       // use Yariv Kaplan's WinIO library 3.0 (allowing MMIO, http://www.internals.com/)
        //#define USE_WINIO2     // use Yariv Kaplan's WinIO library 2.0 (has bug on MMIO, http://www.internals.com/)
        //#define USE_PCIDEBUG   // use Kasiwano's PciDebug library      (http://www.otto.to/~kasiwano/)
        //#define USE_PHYMEM     // use cyb70289's PhyMem library        (http://www.codeproject.com/KB/system/phymem.aspx)
    #endif

    #define DMP_WINDOWS
#endif


/*
// not used yet
#if defined(__BORLANDC__) && defined(__WIN32__)
    #if __BORLANDC__ < 0x0520
        #define DMP_WIN32_BC
    #else
        #define DMP_WIN32_BCB
    #endif
#endif

#if defined(__MINGW32__)
    #define DMP_WIN32_MINGW
#endif
*/
/*---------------------  end of Platform Information  ------------------------*/



/******************************  API Prefix  **********************************/
#if defined(__cplusplus)
    #define __dmp_inline(rtype)     static inline rtype
#elif defined(DMP_DOS_DJGPP)
    //#define __dmp_inline(rtype)     static __INLINE__ rtype
    #define __dmp_inline(rtype)     static __inline__ rtype
#elif defined(DMP_LINUX_GCC)
    // note: GCC would always do inline-expansion for "static inline"
    #define __dmp_inline(rtype)     static __inline__ rtype
#elif defined(DMP_WINCE_MSVC) || defined(DMP_WIN32_MSVC)
    #define __dmp_inline(rtype)     static __inline rtype
#else
    #define __dmp_inline(rtype)     static rtype
#endif


#if defined(DMP_LIB_DLL)

    #if (defined(DMP_WINCE_MSVC) || defined(DMP_WIN32_MSVC))
        #ifdef DMP_DLL_EXPORTING
            #define DMPAPI(rtype)       __declspec(dllexport) rtype __stdcall
            #define DMPAPI_C(rtype)     __declspec(dllexport) rtype __cdecl
        #else
            #define DMPAPI(rtype)       __declspec(dllimport) rtype __stdcall
            #define DMPAPI_C(rtype)     __declspec(dllimport) rtype __cdecl
        #endif
    #else  // Linux shared lib 
        #define DMPAPI(rtype)           rtype
        #define DMPAPI_C(rtype)         rtype
    #endif
    #define DMP_INLINE(rtype)       DMPAPI(rtype)

#else  // static C/C++ lib

    #define DMPAPI(rtype)           rtype
    #define DMPAPI_C(rtype)         rtype
    #define DMP_INLINE(rtype)       __dmp_inline(rtype)

#endif
/*--------------------------  end of API Prefix  -----------------------------*/


#endif

