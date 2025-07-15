/*.........................................................................*/
/*                  TSHTEST.H ------> TSH test program                     */
/*                  February '13, Oct '18 updated by Andrew Nieves        */
/*.........................................................................*/

#include "synergy.h"

char login[NAME_LEN];

void OpPut(/*void*/) ;
void OpGet(/*void*/) ;
void OpExit(/*void*/) ;
void OpShell(/*void*/);
void OpRetrieve(/*void*/) ;

int tshsock ;
int connectTsh(u_short) ;
u_short drawMenu(/*void*/) ;
char* read_input(size_t *length);


// Unit test function prototypes
void run_unit_tests();
static void open_connection();

/* Function prototypes for the TSH test (client) program */

/* 
 * Reads a full command line from standard input.
 * Returns a pointer to a dynamically allocated string (which the caller must free).
 */
void OpShell(void);


/* 
 * Draws the interactive menu and returns the user's choice as a u_short.
 */
u_short drawMenu(void);

