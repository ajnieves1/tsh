#include "synergy.h"
void OpShell(char *line);


void tsInit(short PORT);
void OpPut(char *TpName, char *Tuple, long TpLength);
void OpGet(char *pattern, char *buffer);
void OpExit();

int connectTsh(u_short port);