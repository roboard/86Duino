#ifndef _TP_H_
#define _TP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "src/lv_hal_indev.h"

void touchpanel_init(void);
bool get_touchXY(int* dataX, int* dataY);
void resetTPAll(void);

#ifdef __cplusplus
}
#endif

#endif