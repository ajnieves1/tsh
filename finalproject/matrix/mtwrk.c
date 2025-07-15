#include <stdio.h>
#include "matrix.h"
#include <time.h>

double ita[N/2][N];
double ott[N/2][N];

void main () {
    int G, tsd, res, i, j, k, status;
    int ix, ia, ib, tplength;
    time_t t0, t1;

    tshInit();

    strcpy(tpname, "B");
    puts(tpname);
    puts("To opread\n");
    status = OpRead(tpname, (char *)&ituple_B);
    tplength = (1+N*N)*sizeof(double);
    if ((ituple_A = (double *)malloc(tplength)) == NULL) {
        perror("Master malloc A error\n");
        exit(1);
    }
    while (1) {
        printf("Worker waiting for tuple\n");
        strcpy(tpname, "A");
        tshInit();
        tplength = OpGet(tpname, (char *) &ituple_A);
        t0 = time(NULL);

        ix = atoi(&tpname[1]);
        if (tplength > 0) {
            G = (int) ituple_A[0];
            printf("mtwork gotr ix (%d) G(%d)\n", ix, G);
            printf("Tuple first item [%f]", *ituple_A);

            if (G == -1) {
                tshInit();
                status = OpPut(tpname, (char *)ituple_A, tplength);
                exit(1);
                return;
            }
            for (i = 0; i < G; i++) {
                for (j = 0; j < N; j++) {
                    ita[i][j] = ituple_A[i*N+j+1];
                    ott[i][j] = 0;
                }
            }
            if ((otuple = (double *)malloc(tplength)) == NULL) {
                perror("Master malloc A error\n");
                exit(-1);
            }
            otuple[0] = ituple_A[0];

            for (i = 0; i < G; i++) {
                for (k = 0; k < N; k++) {
                    for (j = 0; j < N; j++) {
                        ott[i][j] = ott[i][j] + ita[i][k] * ituple_B[k][j];
                    }
                }
            }
            for (i = 0; i < G; i++) {
                for (j = 0; j < N; j++) {
                    otuple[i*N+j+1] = ott[i][j];
                }
            }
            sprintf(tpname, "%d\0", ix);
            tshInit();
            status = OpPut(tpname, (char *)otuple, tplength);
            double newtime = wall_clock();
            t1 = (double)(time(NULL) - t0)/CLOCKS_PER_SEC;
            free(otuple);
        } else {
            printf("Worker terminated\n");
            exit(0);
            return;
        }
    }
}