/*.........................................................................*/
/*                  TSHTEST.C ------> TSH test program                     */
/*                                                                         */
/*                  February '13, updated by Andrew Nieves                 */
/*.........................................................................*/
// Client side

#define BUFFER_SIZE 1024

#include "tshtest.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/wait.h> 
#include <assert.h>
int status;
static u_short test_port; // Global variable for server port

/*
Unit Test for OP_SHELL: This test connects to TSH, sends an OP_SHELL command with a simple command ("true"), 
and verifies that the acknowledgment indicates a successful execution (exit status 0)
*/
void test_op_shell(void) { 
   printf("Running OP_SHELL unit test...\n"); 
   int sock = connectTsh(atoi("4966")); // change port as needed 
   if (sock < 0) { 
      printf("OP_SHELL test failed: cannot connect to TSH\n"); 
      return; 
   } // Send OP_SHELL op-code (TSH_OP_SHELL is 405) 
   u_short op_code = htons(TSH_OP_SHELL); 
   if (!writen(sock, (char *)&op_code, sizeof(op_code))) { 
      printf("OP_SHELL test failed: cannot send op-code\n"); 
      close(sock); 
      return; 
   } // Prepare the shell input with a simple command ("true") 
   tsh_shell_it shell_in; 
   char *command = "true"; 
   size_t cmd_len = strlen(command); 
   shell_in.length = htonl(cmd_len); 
   if (!writen(sock, (char *)&shell_in, sizeof(shell_in))) { 
      printf("OP_SHELL test failed: cannot send shell_in header\n"); 
      close(sock); 
      return; 
   } 
   if (!writen(sock, command, cmd_len)) { 
      printf("OP_SHELL test failed: cannot send command string\n"); 
      close(sock); 
      return; 
   } // Read the acknowledgment from TSH 
   tsh_shell_ot shell_out; 
   if (!readn(sock, (char *)&shell_out, sizeof(shell_out))) { 
      printf("OP_SHELL test failed: cannot read response\n"); 
      close(sock); 
      return; 
   } 
   close(sock); 
   if (ntohs(shell_out.status) == SUCCESS) { 
      printf("OP_SHELL test passed: %s\n", shell_out.out_buffer); 
   } else { 
      printf("OP_SHELL test failed: status %d, error %d\n", ntohs(shell_out.status), ntohs(shell_out.error)); 
   } 
}

void test_put_single_read(void) {
   printf("Running Put-Single Read unit test...\n");
   /* --- Perform PUT operation --- */
   int sock_put = connectTsh(atoi("4966")); // adjust port as needed
   if (sock_put < 0) {
      printf("Put test failed: cannot connect for put\n");
      return;
   }
   tsh_put_it put_in;
   memset(&put_in, 0, sizeof(put_in));
   strcpy(put_in.name, "unit_test_tuple_read");
   put_in.priority = htons(1);
   const char *tuple_data = "read_test_content";
   put_in.length = htonl(strlen(tuple_data) + 1);
   put_in.proc_id = htonl(0);
   u_short op_code_put = htons(TSH_OP_PUT); // e.g. 401
   if (!writen(sock_put, (char *)&op_code_put, sizeof(op_code_put))) {
      printf("Put test failed: cannot send op-code for put\n");
      close(sock_put);
      return;
   }
   if (!writen(sock_put, (char *)&put_in, sizeof(put_in))) {
      printf("Put test failed: cannot send PUT structure\n");
      close(sock_put);
      return;
   }
   if (!writen(sock_put, tuple_data, ntohl(put_in.length))) {
      printf("Put test failed: cannot send tuple data\n");
      close(sock_put);
      return;
   }
   tsh_put_ot put_ot;
   if (!readn(sock_put, (char *)&put_ot, sizeof(put_ot))) {
      printf("Put test failed: cannot read PUT response\n");
      close(sock_put);
      return;
   }
   close(sock_put);

   /* --- Perform first READ operation --- */
   int sock_read1 = connectTsh(atoi("4966"));
   if (sock_read1 < 0) {
      printf("Read test failed: cannot connect for first read\n");
      return;
   }
   u_short op_code_read = htons(TSH_OP_READ); // e.g. 403
   if (!writen(sock_read1, (char *)&op_code_read, sizeof(op_code_read))) {
      printf("Read test failed: cannot send op-code for first read\n");
      close(sock_read1);
      return;
   }
   tsh_get_it get_in;
   memset(&get_in, 0, sizeof(get_in));
   strcpy(get_in.expr, "unit_test_tuple_read");
   get_in.host = inet_addr("127.0.0.1");
   int sd1 = get_socket();
   if (sd1 == -1) {
      printf("Read test failed: cannot get socket for first read\n");
      close(sock_read1);
      return;
   }
   get_in.port = bind_socket(sd1, 0);
   if (!get_in.port) {
      printf("Read test failed: cannot bind socket for first read\n");
      close(sock_read1);
      close(sd1);
      return;
   }
   get_in.len = htonl(1024);  /* maximum bytes requested */
   get_in.proc_id = htonl(0);
   if (!writen(sock_read1, (char *)&get_in, sizeof(get_in))) {
      printf("Read test failed: cannot send get_in for first read\n");
      close(sock_read1);
      close(sd1);
      return;
   }
   tsh_get_ot1 get_ot1;
   if (!readn(sock_read1, (char *)&get_ot1, sizeof(get_ot1))) {
      printf("Read test failed: cannot read get_ot1 for first read\n");
      close(sock_read1);
      close(sd1);
      return;
   }
   if (ntohs(get_ot1.status) != SUCCESS) {
      printf("Read test failed: tuple not found on first read, status %d\n", ntohs(get_ot1.status));
      close(sock_read1);
      close(sd1);
      return;
   }
   tsh_get_ot2 get_ot2;
   if (!readn(sock_read1, (char *)&get_ot2, sizeof(get_ot2))) {
      printf("Read test failed: cannot read get_ot2 for first read\n");
      close(sock_read1);
      close(sd1);
      return;
   }
   char *recv_tuple1 = malloc(ntohl(get_ot2.length));
   if (!recv_tuple1) {
      printf("Read test failed: malloc error on first read\n");
      close(sock_read1);
      close(sd1);
      return;
   }
   if (!readn(sock_read1, recv_tuple1, ntohl(get_ot2.length))) {
      printf("Read test failed: cannot read tuple data on first read\n");
      free(recv_tuple1);
      close(sock_read1);
      close(sd1);
      return;
   }
   if (strcmp(recv_tuple1, tuple_data) == 0) {
      printf("Put-Single Read (first read) test passed.\n");
   } else {
      printf("Put-Single Read (first read) test failed: expected '%s', got '%s'\n", tuple_data, recv_tuple1);
   }
   free(recv_tuple1);
   close(sock_read1);
   close(sd1);

   /* --- Perform second READ operation (multiple read) --- */
   int sock_read2 = connectTsh(atoi("4966"));
   if (sock_read2 < 0) {
      printf("Read test failed: cannot connect for second read\n");
      return;
   }
   op_code_read = htons(TSH_OP_READ);
   if (!writen(sock_read2, (char *)&op_code_read, sizeof(op_code_read))) {
      printf("Read test failed: cannot send op-code for second read\n");
      close(sock_read2);
      return;
   }
   memset(&get_in, 0, sizeof(get_in));
   strcpy(get_in.expr, "unit_test_tuple_read");
   get_in.host = inet_addr("127.0.0.1");
   int sd2 = get_socket();
   if (sd2 == -1) {
      printf("Read test failed: cannot get socket for second read\n");
      close(sock_read2);
      return;
   }
   get_in.port = bind_socket(sd2, 0);
   if (!get_in.port) {
      printf("Read test failed: cannot bind socket for second read\n");
      close(sock_read2);
      close(sd2);
      return;
   }
   get_in.len = htonl(1024);
   get_in.proc_id = htonl(0);
   if (!writen(sock_read2, (char *)&get_in, sizeof(get_in))) {
      printf("Read test failed: cannot send get_in for second read\n");
      close(sock_read2);
      close(sd2);
      return;
   }
   if (!readn(sock_read2, (char *)&get_ot1, sizeof(get_ot1))) {
      printf("Read test failed: cannot read get_ot1 for second read\n");
      close(sock_read2);
      close(sd2);
      return;
   }
   if (ntohs(get_ot1.status) != SUCCESS) {
      printf("Read test failed: tuple not found on second read, status %d\n", ntohs(get_ot1.status));
      close(sock_read2);
      close(sd2);
      return;
   }
   if (!readn(sock_read2, (char *)&get_ot2, sizeof(get_ot2))) {
      printf("Read test failed: cannot read get_ot2 for second read\n");
      close(sock_read2);
      close(sd2);
      return;
   }
   char *recv_tuple2 = malloc(ntohl(get_ot2.length));
   if (!recv_tuple2) {
      printf("Read test failed: malloc error on second read\n");
      close(sock_read2);
      close(sd2);
      return;
   }
   if (!readn(sock_read2, recv_tuple2, ntohl(get_ot2.length))) {
      printf("Read test failed: cannot read tuple data on second read\n");
      free(recv_tuple2);
      close(sock_read2);
      close(sd2);
      return;
   }
   if (strcmp(recv_tuple2, tuple_data) == 0) {
      printf("Put-Single Read (second read) test passed.\n");
   } else {
      printf("Put-Single Read (second read) test failed: expected '%s', got '%s'\n", tuple_data, recv_tuple2);
   }
   free(recv_tuple2);
   close(sock_read2);
   close(sd2);


}

int main(int argc, char **argv)
{
	static void (*op_func[])() = 
	{OpPut, OpGet, OpGet, OpExit, OpShell} ;
	u_short this_op ;
   
	if (argc < 2)
    {
       printf("Usage : %s port\n", argv[0]) ;
       exit(1) ;
    }

   // If a second argument "unit" is provided, run the unit tests.
   if (argc >= 3 && strcmp(argv[2], "unit") == 0) {
      test_op_shell();  // 
      test_put_single_read();
      exit(0);
   }
   // Otherwise, run the interactive menu as usual:
   while (TRUE) {
      this_op = drawMenu() + TSH_OP_MIN - 1;
      if (this_op >= TSH_OP_MIN && this_op <= TSH_OP_MAX) {
         this_op = htons(this_op);
         tshsock = connectTsh(atoi(argv[1]));
         if (!writen(tshsock, (char *)&this_op, sizeof(this_op))) {
            perror("main::writen");
            exit(1);
         }
         printf("sent tsh op\n");
         (*op_func[ntohs(this_op) - TSH_OP_MIN])();
         close(tshsock);
      } else {
         return 0;
      }
   }
}


char *read_input(size_t *length)
{
    char *line_buf = NULL;
    size_t line_buf_size;
    size_t line_size = 0;
 
	while ((getchar()) != '\n');
    line_size = getline(&line_buf, &line_buf_size, stdin);
    *length = line_size;
	
    if(line_size == -1) {
        if (feof(stdin)) {
        exit(EXIT_SUCCESS);
        } else {
            perror("myShell");
            exit(EXIT_FAILURE);
        }
    }
    return line_buf;
}


// Refere to shell input part before parsing: Get total length + stream
// Prepare tsh_shell_it
// Send length
// Send stream
// Read ack
void OpShell() { // Added
   tsh_shell_it out ; // sending command line to tsh.c
   tsh_shell_ot in ; // for ack from tsh
   char *buff, *st;
   char *stdout;

   status=system("clear") ;
   printf("TSH_OP_Shell") ;
   printf("\n----------\n") ;
   
    // buff comamnd 
    printf("\n>>>>>: ") ; /* shell command */
   buff = read_input(&out.length);
   out.length = htonl(out.length) ;
   
    /* print entered commands */
   printf("\nEntered Shell Command: %s\n", buff) ;

	/* send length */
   if (!writen(tshsock, (char *)&out, sizeof(out)))
    {
       perror("\nOpShell sending commandLine lengtherror::writen\n") ;
       getchar() ;
       free(buff);
       return ;
    }

	/* send stream */
   if (!writen(tshsock, buff, ntohl(out.length)))
    {
       perror("\nOpShell sending commandLine error::writen\n") ;
       getchar() ;
       free(buff);
       return ;
    }

	/* read ack */
   if (!readn(tshsock, (char *)&in, sizeof(in)))
    {
       perror("\nOpShell::readn\n") ;
       getchar() ;
       return ;
    }

    in.stdoutLength = ntohl(in.stdoutLength); // Convert from network to host long int
    if (!readn(tshsock, (char *)&stdout, in.stdoutLength))
    {
       perror("\nOpShell:: read stdoutlength\n") ;
    }
	
    /* print result from TSH */
    printf("\n\nFrom TSH :\n") ;
    printf("status : %d \n", ntohs(in.status)) ;
    printf("error : %d\n", ntohs(in.error)) ;
    printf("Stdout: [%s]\n", stdout);
    getchar();	
}

void OpPut()
{
   tsh_put_it out ;
   tsh_put_ot in ;
   int tmp ;
   char *buff,*st ;

   status=system("clear") ;
   printf("TSH_OP_PUT") ;
   printf("\n----------\n") ;
				/* obtain tuple name, priority, length, */
   printf("\nEnter tuple name : ") ; /* and the tuple */
   status=scanf("%s", out.name) ;
   printf("Enter priority : ") ;
   status=scanf("%d", &tmp) ;
   out.priority = (u_short)tmp ;
   printf("Enter length : ") ;
   status=scanf("%d", &out.length) ;
   getchar() ;
   printf("Enter tuple : ") ;
   buff = (char *)malloc(out.length) ;
   st=fgets(buff, out.length, stdin) ;
				/* print data sent to TSH */
   printf("\n\nTo TSH :\n") ;
   printf("\nname : %s", out.name) ;
   printf("\npriority : %d", out.priority) ;
   printf("\nlength : %d", out.length) ;
   printf("\ntuple : %s\n", buff) ;

   out.priority = htons(out.priority) ;
   out.length = htonl(out.length) ;
				/* send data to TSH */
   if (!writen(tshsock, (char *)&out, sizeof(out)))
    {
       perror("\nOpPut::writen\n") ;
       getchar() ;
       free(buff) ;
       return ;
    }
				/* send tuple to TSH */
   if (!writen(tshsock, buff, ntohl(out.length)))
    {
       perror("\nOpPut::writen\n") ;
       getchar() ;
       free(buff) ;
       return ;
    }
				/* read result */
   if (!readn(tshsock, (char *)&in, sizeof(in)))
    {
       perror("\nOpPut::readn\n") ;
       getchar() ;
       return ;
    }
				/* print result from TSH */
   printf("\n\nFrom TSH :\n") ;
   printf("\nstatus : %d", ntohs(in.status)) ;
   printf("\nerror : %d\n", ntohs(in.error)) ;
   getchar() ;
}


void OpGet()
{
   tsh_get_it out ;
   tsh_get_ot1 in1 ;
   tsh_get_ot2 in2 ;
   struct in_addr addr ;
   int sd, sock ;
   char *buff ;

   status=system("clear") ;
   printf("TSH_OP_GET") ;
   printf("\n----------\n") ;
				/* obtain tuple name/wild card */
   printf("\nEnter tuple name [wild cards ?, * allowed] : ") ;
   status=scanf("%s", out.expr) ;
				/* obtain port for return data if tuple not available */
   // This line has to revise for clusters. out.host = gethostid() ;	
   out.host = inet_addr("127.0.0.1");
   if ((sd = get_socket()) == -1)
    {
       perror("\nOpGet::get_socket\n") ;
       getchar() ; getchar() ;
       return ;
    }
   if (!(out.port = bind_socket(sd, 0)))
    {
       perror("\nOpGet::bind_socket\n") ;
       getchar() ; getchar() ;
       return ;
    }
   addr.s_addr = out.host ;
				/* print data  sent to TSH */
   printf("\n\nTo TSH :\n") ;
   printf("\nexpr : %s", out.expr) ;
   printf("\nhost : %s", inet_ntoa(addr)) ;
   printf("\nport : %d\n", (out.port)) ;
				/* send data to TSH */
   if (!writen(tshsock, (char *)&out, sizeof(out)))
    {
       perror("\nOpGet::writen\n") ;
       getchar() ; getchar() ;
       close(sd) ;
       return ;
    }
				/* find out if tuple available */
   if (!readn(tshsock, (char *)&in1, sizeof(in1)))
    {
       perror("\nOpGet::readn\n") ;
       getchar() ; getchar() ;
       close(sd) ;
       return ;
    }
				/* print whether tuple available in TSH */
   printf("\n\nFrom TSH :\n") ;
   printf("\nstatus : %d", ntohs(in1.status)) ;
   printf("\nerror : %d\n", ntohs(in1.error)) ;
				/* if tuple is available read from the same */
   if (ntohs(in1.status) == SUCCESS) /* socket */
      sock = tshsock ;
   else				/* get connection in the return port */
      sock = get_connection(sd, NULL) ;
				/* read tuple details from TSH */
   if (!readn(sock, (char *)&in2, sizeof(in2)))
    {
       perror("\nOpGet::readn\n") ;
       getchar() ; getchar() ;
       close(sd) ;
       return ;
    }				/* print tuple details from TSH */
   printf("\nname : %s", in2.name) ;
   printf("\npriority : %d", ntohs(in2.priority)) ;
   printf("\nlength : %d", ntohl(in2.length)) ;
   buff = (char *)malloc(ntohl(in2.length)) ;
				/* read, print  tuple from TSH */
   if (!readn(sock, buff, ntohl(in2.length)))
      perror("\nOpGet::readn\n") ;
   else
      printf("\ntuple : %s\n", buff) ;

   close(sd) ;
   close(sock) ;
   free(buff) ;
   getchar() ; getchar() ;
}


void OpExit()
{
   tsh_exit_ot in ;
   
   status=system("clear") ;
   printf("TSH_OP_EXIT") ;
   printf("\n-----------\n") ;
				/* read TSH response */
   if (!readn(tshsock, (char *)&in, sizeof(in)))
    {
       perror("\nOpExit::readn\n") ;
       getchar() ;  getchar() ;
       return ;
    }
				/* print TSH response */
   printf("\n\nFrom TSH :\n") ;
   printf("\nstatus : %d", ntohs(in.status)) ;
   printf("\nerror : %d\n", ntohs(in.error)) ;
   getchar() ;  getchar() ;
}


int connectTsh(u_short port)
{
   struct hostent *host ;
   short tsh_port ;
   u_long tsh_host ;
   int sock ;

   // use local host 
   tsh_host = inet_addr("127.0.0.1");
   /*
   if ((host = gethostbyname("localhost")) == NULL)
	{
	   perror("connectTsh::gethostbyname\n") ;
	   exit(1) ;
	}
   tsh_host = *((long *)host->h_addr_list[0]) ;
   */
   tsh_port = htons(port);
				/* get socket and connect to TSH */
   if ((sock = get_socket()) == -1)
    {
       perror("connectTsh::get_socket\n") ;
       exit(1) ;
    }
   if (!do_connect(sock, tsh_host, tsh_port))
    {
       perror("connectTsh::do_connect\n") ;
       exit(1) ;
    }      
   return sock ;
}

u_short drawMenu() {
   int choice ;
	/* draw menu of user options */
   status=system("clear") ;
   printf("\n\n\n\t\t\t---------") ;
   printf("\n\t\t\tMAIN MENU") ;
   printf("\n\t\t\t---------") ;
   printf("\n\n\t\t\t 1. Put") ;
   printf("\n\t\t\t 2. Get") ;
   printf("\n\t\t\t 3. Read") ;
   printf("\n\t\t\t 4. Exit (TSH)") ;
   printf("\n\t\t\t 5. OP_SHELL (TSH)") ;
   printf("\n\t\t\t 6. Quit from this program") ;
   printf("\n\n\n\t\t\tEnter Choice : ") ;

   status=scanf("%d", &choice) ;	/* return user choice */
   return choice ;
}

