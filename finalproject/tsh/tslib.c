/* TSLIB.C -> TSLIB API Program*/

#include "tslib.h"
int status;
u_short PORT = 4966;
int tshsock;
char *TSHIPAddress = "127.0.0.1";

void tsInit(short PORT) {
    tshsock = connectTsh(PORT);
}

void OpShell(char* line) {
   ssize_t nread;

   u_short this_op = htons(TSH_OP_SHELL);
   if (writen(tshsock, (char *)&this_op, sizeof(this_op))) {
       perror("OpShell::writen\n");
       exit (1);
   }

   tsh_shell_it out;
   tsh_shell_ot in;
   out.length = htonl(strlen(line));

   if (!writen(tshsock, (char *)&out, sizeof(out))) {
         perror("OpShell::Error sending header\n");
         exit (2);
   }

   if (!writen(tshsock, line, strlen(line))) {
         perror("OpShell::Error sending data\n");
         exit (3);
   }
   if (!readn(tshsock, (char *)&in, sizeof(in))) {
         perror("OpShell::Error reading header\n");
         exit (4);
   }

   in.stdoutLength = ntohl(in.stdoutLength);
   if (!readn(tshsock, (char *)stdout, in.stdoutLength)) {
         perror("OpShell::Error reading data\n");
         exit (5);
   }

   printf("\n\n FRom tsh: \n");
   printf("status: %d\n", ntohs(in.status));
   printf("error: %d\n", ntohs(in.error));

}

void OpPut(char *TpName, char *Tuple, long TpLength)
{
   tsh_put_it out ;
   tsh_put_ot in ;
   int tmp ;
   char *buff,*st ;

   strcpy(out.name, TpName);
   out.priority = 0;
   out.length = htonl(TpLength);
   buff = Tuple;

				/* send data to TSH */
   if (!writen(tshsock, (char *)&out, sizeof(out)))
    {
       perror("\nOpPut::writen\n") ;
       return -1;
    }
				/* send tuple to TSH */
   if (!writen(tshsock, buff, TpLength))
    {
       perror("\nOpPut::writen\n") ;
      exit(1);
    }

    if (!readn(tshsock, (char *)&in, sizeof(in)))
    {
       perror("\nOpPut::readn\n") ;

         exit(2);
    }

				/* print result from TSH */
   printf("\n\nFrom TSH :\n") ;
   printf("\nstatus : %d", ntohs(in.status)) ;
   printf("\nerror : %d\n", ntohs(in.error)) ;
   getchar() ;
}


void OpGet(char *pattern, char *buffer)
{
   tsh_get_it out ;
   tsh_get_ot1 in1 ;
   tsh_get_ot2 in2 ;
   struct in_addr addr ;
   int sd, sock ;
   char *buff ;

   strcpy(out.expr, pattern);
   out.host = inet_addr(TSHIPAddress);
   if ((sd = get_socket()) == -1)
    {
       perror("\nOpGet::get_socket\n") ;
       return ;
    }

    if (!(out.port = bind_socket(sd, 0)))
    {
       perror("\nOpGet::bind_socket\n") ;
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
         exit(1);
   }
				/* find out if tuple available */
   if (!readn(tshsock, (char *)&in1, sizeof(in1)))
    {
       perror("\nOpGet::readn\n") ;
       getchar() ; getchar() ;
         close(sd) ;
         exit(2);
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

