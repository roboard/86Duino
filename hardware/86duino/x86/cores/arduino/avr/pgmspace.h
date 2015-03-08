#ifndef __PGMSPACE_H_
#define __PGMSPACE_H_ 1

#include <stdio.h>
#include <string.h>
#include "dmpcfg.h"

#ifndef __ATTR_CONST__
#define __ATTR_CONST__
#endif

#ifndef __ATTR_PROGMEM__
#define __ATTR_PROGMEM__
#endif

#ifndef __ATTR_PURE__
#define __ATTR_PURE__
#endif

#define PROGMEM __ATTR_PROGMEM__

#ifdef __cplusplus
extern "C" {
#endif

typedef void               prog_void     PROGMEM;
typedef char               prog_char     PROGMEM;
typedef unsigned char      prog_uchar    PROGMEM;
typedef char               prog_int8_t   PROGMEM;
typedef unsigned char      prog_uint8_t  PROGMEM;
typedef short              prog_int16_t  PROGMEM;
typedef unsigned short     prog_uint16_t PROGMEM;
typedef long               prog_int32_t  PROGMEM;
typedef unsigned long      prog_uint32_t PROGMEM;
typedef long long          prog_int64_t  PROGMEM;
typedef unsigned long long prog_uint64_t PROGMEM;

#define _SFR_BYTE(n) (n)
#define PSTR(s)      (s)

#define pgm_read_byte_near(address_short)  (*(const unsigned char *)(address_short))
#define pgm_read_word_near(address_short)  (*(const unsigned short *)(address_short))
#define pgm_read_dword_near(address_short) (*(const unsigned long *)(address_short))
#define pgm_read_float_near(address_short) (*(const float *)(address_short))
#define pgm_read_ptr_near(address_short)   (void*)(*(const unsigned short *)(address_short))

#define pgm_read_byte_far(address_long)  (*(const unsigned char *)(address_long))
#define pgm_read_word_far(address_long)  (*(const unsigned short *)(address_long))
#define pgm_read_dword_far(address_long) (*(const unsigned long *)(address_long))
#define pgm_read_float_far(address_long) (*(const float *)(address_long))
#define pgm_read_ptr_far(address_long)   (void*)(*(const unsigned long *)(address_long)) 

#define pgm_read_byte(address_short)    pgm_read_byte_near(address_short)
#define pgm_read_word(address_short)    pgm_read_word_near(address_short)
#define pgm_read_dword(address_short)   pgm_read_dword_near(address_short)
#define pgm_read_float(address_short)   pgm_read_float_near(address_short)
#define pgm_read_ptr(address_short)     pgm_read_ptr_near(address_short)

#ifndef PGM_P
#define PGM_P const prog_char *
#endif

#ifndef PGM_VOID_P
#define PGM_VOID_P const prog_void *
#endif

#define memcmp_P(a, b, c)        memcmp((a), (b), (c))
#define memchr_P(a, b, c)        memchr((a), (b), (c))
#define memccpy_P(a, b, c, d)    memccpy_P((a), (b), (c), (d)) // new for 1.5.8
#define memcpy_P(a, b, c)        memcpy((a), (b), (c))
//extern void *memmem_P(const void *, size_t, PGM_VOID_P, size_t) __ATTR_PURE__;
#define memrchr_P(a, b, c)       memchr((a), (b), (c))
#define strcat_P(a, b)           strcat((a), (b))
#define strchr_P(a, b)           strchr((a), (b))
//extern PGM_P strchrnul_P(PGM_P, int __val) __ATTR_CONST__;
#define strcmp_P(a, b)           strcmp((a), (b))
#define strcpy_P(a, b)           strcpy((a), (b))
#define strcasecmp_P(a, b)       strcasecmp((a), (b))
//extern char *strcasestr_P(const char *, PGM_P) __ATTR_PURE__;
#define strcspn_P(a, b)          strcspn((a), (b))
#define strlcat_P(a, b)          strlcat((a), (b))
#define strlcpy_P(a, b)          strlcpy((a), (b))
#define strlen_P(a)              strlen((a))
//extern size_t strnlen_P(PGM_P, size_t) __ATTR_CONST__; /* program memory can't change */
#define strncmp_P(a, b, c)       strncmp((a), (b), (c))
#define strncasecmp_P(a, b, c)   strncasecmp((a), (b), (c))
#define strncat_P(a, b, c)       strncat((a), (b), (c))
#define strncpy_P(a, b, c)       strncpy((a), (b), (c))
#define strpbrk_P(a, b)          strpbrk((a), (b))
#define strrchr_P(a, b)          strrchr((a), (b))
#define strsep_P(a, b)           strsep((a), (b))
#define strspn_P(a, b)           strspn((a), (b))
#define strstr_P(a, b)           strstr((a), (b))
#define strtok_P(a, b)           strtok((a), (b)) // new for 1.5.8
#define strtok_rP(a, b, c)       strtok_r((a), (b), (c)) // new for 1.5.8

#define printf_P                 printf
#define sprintf_P                sprintf

#ifdef __cplusplus
}
#endif
DMP_INLINE(void*) memmem_P(const void * s1, size_t len1, PGM_VOID_P s2, size_t len2) {
	if(len2 == 0) return (void*)s1;
	return (void*)strstr((const char*)s1, (const char*)s2);
}

DMP_INLINE(char*) strcasestr_P(const char *s1, PGM_P s2) {
    return strstr(s1, s2);
}

DMP_INLINE(char*) strchrnul_P(PGM_P s1, int __val) {
    char* tmp = strchr(s1, __val);
	if(tmp == NULL) return (char*)(s1+strlen(s1));
	return tmp;
}

DMP_INLINE(size_t) strnlen_P(PGM_P s1, size_t len) {
    int i;
    for(i=0; i<len; i++)
    	if(*(s1 + i) == '\0') return strlen(s1);
    return len;
}

#define _BV(bit) (1 << (bit))

#endif /* __PGMSPACE_H_ */
