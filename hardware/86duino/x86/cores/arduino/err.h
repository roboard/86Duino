/**************************  Error Logging Functions  *************************/
#if !defined(ERR_COMMON_LIB)
    static void err_print(char* fmt, ...) {
        #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC) || defined(DMP_WIN_MSVC) || defined(DMP_LINUX)
            va_list args;

            va_start(args, fmt);
            vfprintf(stderr, fmt, args);
            va_end(args);
	
            fflush(stderr);
        #endif
    }
#endif
/*----------------------  end. Error Logging Functions  ----------------------*/
