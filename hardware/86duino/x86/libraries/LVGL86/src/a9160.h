#ifndef _A9160_H_
#define _A9160_H_

#ifdef __cplusplus
extern "C" {
#endif

void a9160_Init(unsigned int hor, unsigned int ver);
int get_a9160_fb_selector(void);
void a9160_Close(void);

#ifdef __cplusplus
}
#endif

#endif