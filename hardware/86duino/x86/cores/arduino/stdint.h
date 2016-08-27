#ifndef __stdint__h_
#define __stdint__h_

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char int8_t;

typedef unsigned char uint8_t;

typedef signed short int16_t;

typedef unsigned short uint16_t;

typedef signed long int int32_t;

typedef unsigned long int uint32_t;

typedef signed long long int int64_t;

typedef unsigned long long int uint64_t;


typedef long int intptr_t;

typedef unsigned long int uintptr_t;


typedef int8_t   int_least8_t;

typedef uint8_t  uint_least8_t;

typedef int16_t  int_least16_t;

typedef uint16_t uint_least16_t;

typedef int32_t  int_least32_t;

typedef uint32_t uint_least32_t;

typedef int64_t  int_least64_t;

typedef uint64_t uint_least64_t;


typedef int8_t int_fast8_t;

typedef uint8_t uint_fast8_t;

typedef int16_t int_fast16_t;

typedef uint16_t uint_fast16_t;

typedef int32_t int_fast32_t;

typedef uint32_t uint_fast32_t;

typedef int64_t int_fast64_t;

typedef uint64_t uint_fast64_t;


typedef int32_t intmax_t;

typedef uint32_t uintmax_t;

/*
#if !defined(__cplusplus) || defined(__STDC_LIMIT_MACROS)

#define INT_LEAST8_MAX	 127
#define UINT_LEAST8_MAX	 255
#define INT_FAST8_MAX	 127
#define UINT_FAST8_MAX	 255
#define INT8_MAX	 127
#define UINT8_MAX	 255 
#define INT_LEAST8_MIN	 (-128)
#define INT_FAST8_MIN	 (-128)
#define INT8_MIN	 (-128)

#define INT_LEAST16_MAX	 32767
#define UINT_LEAST16_MAX 65535
#define INT_FAST16_MAX	 2147483647
#define UINT_FAST16_MAX	 4294967295U
#define INT16_MAX	 32767
#define UINT16_MAX	 65535
#define INT_LEAST16_MIN	 (-32768)
#define INT_FAST16_MIN	 (-2147483647-1)
#define INT16_MIN	 (-32768)

#define INT_LEAST32_MAX	 2147483647
#define UINT_LEAST32_MAX 4294967295U
#define INT_FAST32_MAX	 2147483647
#define UINT_FAST32_MAX	 4294967295U
#define INT32_MAX	 2147483647L
#define UINT32_MAX	 4294967295UL
#define INT_LEAST32_MIN	 (-2147483647-1)
#define INT_FAST32_MIN	 (-2147483647-1)
#define INT32_MIN	 (-2147483647L-1)

#define INT_LEAST64_MAX	 9223372036854775807LL
#define UINT_LEAST64_MAX 18446744073709551615ULL
#define INT_FAST64_MAX	 9223372036854775807LL
#define UINT_FAST64_MAX	 18446744073709551615ULL
#define INT64_MAX	 9223372036854775807LL
#define UINT64_MAX	 18446744073709551615ULL
#define INT_LEAST64_MIN	 (-9223372036854775807LL-1LL)
#define INT_FAST64_MIN	 (-9223372036854775807LL-1LL)
#define INT64_MIN	 (-9223372036854775807LL-1LL)

#define INTPTR_MAX	2147483647L
#define INTPTR_MIN	(-2147483647L-1L)
#define UINTPTR_MAX	0xffffffffUL

#define INTMAX_MAX	9223372036854775807LL
#define UINTMAX_MAX	18446744073709551615ULL
#define INTMAX_MIN	(-9223372036854775807LL-1LL)

#define PTRDIFF_MAX	2147483647
#define PTRDIFF_MIN	(-2147483647-1)
#define SIG_ATOMIC_MAX	2147483647
#define SIG_ATOMIC_MIN	(-2147483647-1)
#define SIZE_MAX	4294967295U


#ifndef WCHAR_MAX
#define WCHAR_MAX	65535
#endif
#ifndef WCHAR_MIN
#define WCHAR_MIN	0
#endif
#ifndef WINT_MAX
#define WINT_MAX	2147483647
#endif
#ifndef WINT_MIN
#define WINT_MIN	(-2147483647-1)
#endif

#endif
*/

#ifdef __cplusplus
}
#endif

#endif /* !__stdint__h_ */
