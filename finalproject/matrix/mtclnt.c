// refer to github mtclnt.c

#include <stdio.h>
#include "matrix.h"
#include <time.h>

double C[N][N];
double A[N][N];

void main() {
    char host[128];
    int i, j, k, received;
    int ix, iy, tplength, status;
    int G, R, P, res, tsd, x;
    time_t t0, t1;
    float F;
    FILE *fd;

    t0 = time(NULL);
    ix = 0;

    printf("Before tshinit:\n");
    tshInit();

    G = 4;
    P = 2;
    printf("mtclnt. chink size (%d) \n", G);
    R = N;

    tplength = (1+N*N)*sizeof(double);

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            ituple_B[i][j] = (double) i*j;
            A[i][j] = (double) i*j;
        }
    }
    sprintf(tpname,"B%d\0", 0);
    printf("First put:\n");
    status = OpPut(tpname, (char *) (double* ) ituple_B, tplength);
    printf("Master sent (%d)\n", status);
    tplength = (1+G*N) * sizeof(double);

    if ((ituple_A = (double *) malloc(tplength)) == NULL) {
        perror("Master malloc A error\n");
        exit(1);
    }

    while (R > 0) {
        if (R<G) G = R;
        R = R-G;
        printf("mtclnt. G(%d) R(%d) \n", G,R);
        ituple_A[0] = G;
        for (x = 0; x < G; x++) {
            for (j = 0; j < N; j++) {
                ituple_A[x*N+j+1] = A[ix+x][j];
            }
        }
        sprintf(tpname, "A%d\0",ix);
        tshInit();
        status = OpPut(tpname, (char *) ituple_A, tplength);
        printf("Master sent (%s) bytes (%d)\n", tpname, status);
        ix += G;
    }
    free(ituple_A);

    // now receive
    received = i = 0;
    tplength = (1+N*N)*sizeof(double);
    if ((otuple = (double *)malloc(tplength)) == NULL) {
        perror("Master malloc B error\n");
        exit(2);
    }
    while (received < N) {
        strcpy(tpname, "C");
        printf("master. waiting for result \n");
        tshInit();
        tplength = OpGet(tpname, (char *)&otuple);

        G = (int) otuple[0];
        ix = atoi(tpname);
        iy = 1;
        printf("mtclnt. tuple %d received %d", ix, received);
        for (i = 0; i < G; i++) {
            received++;
            for (j = 0; j < N; j++) {
                C[ix+i][j] = otuple[iy];
                iy++;
            }
            ix++;
        }
    }
    free(otuple);
    printf("Master received everything\n");

    tplength = sizeof(double);
    if ((ituple_A = (double *)malloc(tplength)) == NULL) {
        exit(3);
    }
    ituple_A[0] = -1;
    sprintf(tpname, "A%d\0", N*N);
    tshInit();
    status = OpPut(tpname, (char *)ituple_A, tplength);
    free(ituple_A);
    t1 = (double)(time(NULL) - t0);

    printf("nXDR chunked: (%s) (%ld) sec. P(%d) f(%d) n(%d)", host, t1/1000000, P, G, N*1);
    if (t1>0) fprintf(fd, "(%f) MFLOPS.\n", (float) N*N*N/t1);
    else fprintf(fd, "MFLOPS: not measured.\n");
    fclose(fd);
    exit(0);
}


