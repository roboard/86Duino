#ifndef __WZ_PROTOTHREAD_H
#define __WZ_PROTOTHREAD_H

struct pt {
    int lc;
};

#define PT_BUSY(pt)     ((pt)->lc > 0)
#define PT_INIT(pt)     (pt)->lc = 0;
#define PT_BEGIN(pt)    { char PT_YIELD_FLAG = 1; switch((pt)->lc) { case 0:
#define PT_END(pt)      }; if (PT_YIELD_FLAG) PT_YIELD_FLAG = 0; PT_INIT(pt); return 1; }
#define PT_WAIT_UNTIL(pt, cond)                 \
    do {                                        \
        (pt)->lc = __LINE__; case __LINE__:;    \
        if(!(cond)) {                           \
            return 0;                           \
        }                                       \
    } while(0)
#define PT_YIELD_UNTIL(pt, cond)                \
    do {                                        \
        PT_YIELD_FLAG = 0;                      \
        (pt)->lc = __LINE__; case __LINE__:;    \
        if((PT_YIELD_FLAG == 0) || !(cond)) {   \
            return 0;                           \
        }                                       \
    } while(0)
#define PT_RETURN(pt, return_value)             \
    do {                                        \
        (pt)->lc = __LINE__; case __LINE__:;    \
        return (return_value);                  \
    } while(0)

#endif
