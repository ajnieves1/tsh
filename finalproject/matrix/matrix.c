#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "matrix.h"

double C[N][N];
double A[N][N], B[N][N];

void main(int argc, char **argv) {
    char host[128];
    int i , j, k;
    time_t t0, t1;
    FILE *fd;
    int log = 0;

    if (argc>1) log = 1;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            C[i][j] = 0;
            A[i][j] = (double) i*j;
            B[i][j] = (double) i*j;
        }
    }
    t0 = time(NULL);
    for (k = 0; k < N; k++) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    t1 = (double) (time(NULL) - t0)/CLOCKS_PER_SEC;
    printf("%ld\n", t1);
    exit(1);
}