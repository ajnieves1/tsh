#include "synergy.h"
#define N 500

char tpname[64];

double ituple_B[N][N];
double *ituple_A;

double *otuple;

void tshInit();
u_long OpPut();
u_long OpGet();
u_long OpRead();
