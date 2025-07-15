/*.........................................................................*/
/*                     TSH.H ------> Tuple Space Handler                   */
/*                     February '13, updated by Justin Y. Shi              */
/*.........................................................................*/

#include "synergy.h"

/*  Tuples data structure.  */

struct t_space1 {
   char name[TUPLENAME_LEN] ;	/* tuple name */
   char *tuple ;		/* pointer to tuple */
   u_short priority ;		/* priority of the tuple */
   u_long length ;		/* length of tuple */
   struct t_space1 *next ;
   struct t_space1 *prev ;
} ;
typedef struct t_space1 space1_t ;

/*  Backup tuple list. FSUN 09/94 */
/*  host1(tp) -> host2(tp) -> ... */
struct t_space2 {
        char name[TUPLENAME_LEN];
        char *tuple;
        u_short priority;
        u_long length;
        u_long host;
        u_short port;
	u_short cidport;  /* for dspace. ys'96 */
	int proc_id;
        int fault;
        struct t_space2 *next;
};
typedef struct t_space2 space2_t;

/*  Pending requests data structure.  */

struct t_queue {
   char expr[TUPLENAME_LEN] ;	/* tuple name */
   u_long host ;		/* host from where the request came */
   u_short port ;		/* corresponding port # */
   u_short cidport ;		/* for dspace. ys'96 */
   int proc_id;			/* FSUN 10/94. For FDD */
   u_short request ;		/* read/get */
   struct t_queue *next ;
   struct t_queue *prev ;
} ;
typedef struct t_queue queue1_t ;


/*  Tuple space data structure.  */

struct {
   char appid[NAME_LEN] ;	/* application id */
   char name[NAME_LEN] ;	/* name of the tuple space */
   u_short port ;		/* port where it receives commands */
   
   space1_t *space ;		/* list of tuples */
   space2_t *retrieve ;         /* list of tuples propobly retrieved. FSUN 09/94 */
   queue1_t *queue_hd ;		/* queue of waiting requests */
   queue1_t *queue_tl ;		/* new requests added at the end */
} tsh ;

queue1_t *tid_q;
int oldsock ;			/* socket on which requests are accepted */
int newsock ;			/* new socket identifying a connection */
u_short this_op ;		/* the current operation that is serviced */
char mapid[MAP_LEN];
int EOT = 0;  			/* End of task tuples mark */
int TIDS = 0;
int total_fetched = 0;

/*  Prototypes.  */

void OpPut(/*void*/) ;
void OpGet(/*void*/) ;
void OpExit(/*void*/) ;
void OpShell(/*void*/); // Added


int initCommon(u_short);
void start(/*void*/);
space1_t *createTuple(char *, char *, u_long, u_short);
int consumeTuple(space1_t *);
short int storeTuple(space1_t *, int);
space1_t *findTuple(char *);
void deleteTuple(space1_t *, tsh_get_it *);
int storeRequest(tsh_get_it);
int sendTuple(queue1_t*, space1_t*);
void deleteSpace(/*void*/);
void deleteQueue(/*void*/);
queue1_t *findRequest(char *);
void deleteRequest(queue1_t *);
void sigtermHandler(/*void*/);
int getTshport(u_short);
int match(char *, char *);
int guardf(u_long, int);

/* Function prototypes */

/* Clears the screen and displays a welcome message */
void init_shell(void);

/* Reads a line of input from the user using getline().
   Returns 1 if no input was provided; otherwise returns 0.
   The input line is allocated dynamically and must be freed by the caller. */
int takeInput(char **str);

/* Prints the current working directory. */
void printDir(void);

/* Checks the parsed argument list for output redirection.
   If found, sets *filename to the redirection target and returns 1;
   otherwise, returns 0. */
int checkRedirection(char **parsed, char **filename);

/* Executes a command (without piping) by forking and using execvp().
   'parsed' is an array of tokenized arguments. */
void execArgs(char **parsed);

/* Executes a piped command where two sets of tokenized arguments are provided.
   The function forks twice, connects the standard output of the left command to
   the standard input of the right command using a pipe, then executes both commands. */
void execArgsPiped(char **parsed, char **parsedpipe);

/* Displays a help message listing built-in commands. */
void openHelp(void);

/* Handles built-in commands such as "exit", "cd", and "help".
   Returns 1 if a built-in command was executed; otherwise returns 0. */
int ownCmdHandler(char **parsed);

/* Checks for a pipe symbol "|" in the input string, splitting the string into
   two parts (before and after the pipe). Returns 1 if a pipe is found, 0 otherwise.
   The results are stored in the array 'strpiped'. */
int parsePipe(char *str, char **strpiped);

/* Tokenizes a string (splitting by spaces) into an array of tokens. */
void parseSpace(char *str, char **parsed);

/* Processes the input command:
   - Checks for a pipe,
   - Tokenizes the input accordingly,
   - And calls the built-in command handler.
   Returns 0 if a built-in command was handled; returns 1 if a single command should be executed;
   returns 2 if there is a piped command to execute.
*/
int processString(char *str, char **parsed, char **parsedpipe);
