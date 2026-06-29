#ifndef _ARDUINO_DELAY_H
#define _ARDUINO_DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dmpcfg.h"
#include "io.h"

#include <inttypes.h>
#include <math.h>

static __inline__ void _delay_us(double __us) __attribute__((__always_inline__));
static __inline__ void _delay_ms(double __ms) __attribute__((__always_inline__));

void _delay_ms(double __ms) {
    timer_Delay(__ms);
}

void _delay_us(double __us) {
    timer_DelayMicroseconds(__us);
}

#ifdef __cplusplus
}
#endif

#endif