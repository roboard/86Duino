#include <stdio.h>
typedef struct _is {
  float current;
  float rawdata;
} I_t;

void detectIsIp_Init(void);
void detectIsIp(uint16_t* Ip, uint16_t* Is);
double _getIs(void);
double _getIp(void);
void parseI(FILE* fp, I_t* i);