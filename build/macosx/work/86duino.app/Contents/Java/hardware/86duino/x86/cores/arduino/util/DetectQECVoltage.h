
#include <stdio.h>

typedef struct _vs {
  int voltage;
  int rawdata;
} V_t;

void readcb(int flag);
double _getVs(void);
double _getVp(void);
void parseV(FILE* fp, V_t* v);
