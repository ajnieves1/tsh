/*.........................................................................*/
/*                     LAUNCH.c ------> LAUNCH Handler                   */
/*                     NOV '22, updated by Andrew Nieves           */
/*.........................................................................*/
#include "launch.h"
int status;

// from tshtest.c
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


int main(int argc, char **argv){ // Input format: ./launch port# commandStream [&]

   u_short this_op;
   char *stdout;
   char *cmdStream;

     if(argc < 3){
        printf("Usage : %s port commandStream [&]\n", argv[0]) ;
        return 0;
    }
	
   printf("Commandline = [%s]\n", argv[2]) ;
	this_op = htons(TSH_OP_SHELL);
   tshsock = connectTsh(atoi(argv[1])) ;

   // Send this_op to tsh
    if (!writen(tshsock, (char *)&this_op, sizeof(this_op)))
    {
       perror("\nOpShell::writen\n") ;
       exit(1);
    }


   printf("sent tsh op\n");
   ssize_t nread;

   nread = strlen(argv[2]);

   tsh_shell_it out;
   tsh_shell_ot in;


   out.length = htonl(nread); // Convert from host to network long int
   out.length = htonl(out.length); // Convert length to network byte order

   // Send header
   if (!writen(tshsock, (char *)&out, sizeof(out)))
   {
      perror("\nOpShell errorr sending tsh shell it\n") ;
      exit(2);
   }

   // Send command stream
   if (!writen(tshsock, argv[2], nread))
   {
      perror("\nOpShell error sending command stream\n") ;
      exit(3);
   }

   //Read ack
   if (!readn(tshsock, (char *)&in, sizeof(in)))
   {
      perror("\nOpShell::readn error\n") ;
      exit(4);
   }
   
   // read stdoutlength
   in.stdoutLength = ntohl(in.stdoutLength); // Convert from network to host long int
   if (!readn(tshsock, (char *)&stdout, in.stdoutLength))
   {
      perror("\nOpShell:: read stdoutlength\n") ;
      exit(5);
   }
            /* print result from TSH */
            
    printf("\n\nFrom TSH :\n") ;
    printf("Stdout: [%s]\n", stdout);
    printf("\nStatus: %d", ntohs(in.status)) ;
   printf("\nError: %d\n", ntohs(in.error)) ;
	
}





