#include "tshPingPong.h"

int main(int argc, char ** argv) {
    char tpName[] = "Test";
    char tpName2[] = "Again";
    double *tuple = malloc(5*sizeof(double));
    tuple[0] = 1;
    tuple[1] = 2;
    tuple[2] = 3;
    tuple[3] = 4;
    tuple[4] = 5;
    printf("Tuple: %f %f %f %f %f\n", tuple[0], tuple[1], tuple[2], tuple[3], tuple[4]);
    double *rtTuple;
    
    u_long status;
    tshInit();
    status = OpPut(tpName, (char *) tuple, 5 * sizeof(double));
    printf("Status: %lu\n", status);
    tshInit();

    status = OpRead(tpName2, (char *) &rtTuple);
    printf("Opread status [%ld] name [%s] tuple [%f %f %f %f %f]\n", status, tpName2, rtTuple[0], rtTuple[1], rtTuple[2], rtTuple[3], rtTuple[4]);
    exit(0);
}