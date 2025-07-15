/* TSLIB.C -> TSLIB API Program*/

#include "api.h"
int status;
u_short PORT = 4966;
int tshsock;

void tshInit() {
    tshsock = connectTsh(PORT);
}


u_long OpPut(char *TpName, char *tuple, u_long TpLength)
{
   tsh_put_it out ;
   tsh_put_ot in ;
   int tmp ;
   char *buff;
   u_short this_op;

   // send tsh put to server
   this_op = htons(TSH_OP_PUT);
   if (!writen(tshsock, (char *)&this_op, sizeof(this_op)))
    {
       perror("\nOpPut:: op code sending error\n") ;
       exit(1);
    }
            /* obtain tuple name, priority, length, */

   strcpy(out.name, TpName);
   out.priority = (u_short)0;
   out.length = TpLength;
   buff = tuple;
				/* print data sent to TSH */
   printf("\n\nAPI PUT To TSH :\n") ;
   printf("\nname : %s", out.name) ;
   printf("\npriority : %d", out.priority) ;
   printf("\nlength : %d", out.length) ;
   printf("\ntuple : %s\n", buff) ;

   out.priority = htons(out.priority) ;
   out.length = htonl(out.length) ;
				/* send data to TSH */
   if (!writen(tshsock, (char *)&out, sizeof(out)))
    {
       perror("\nOpPut::payload ssending error\n") ;
       return -1;
    }
				/* send tuple to TSH */
   if (!writen(tshsock, buff, ntohl(out.length)))
    {
       perror("\nOpPut::tuple sending error\n") ;
       return -2;
    }
				/* read ack */
   if (!readn(tshsock, (char *)&in, sizeof(in)))
    {
       perror("\nOpPut::reading ack error\n") ;
       return -3;
    }
				/* print result from TSH */
   printf("\n\nFrom TSH :\n") ;
   printf("\nstatus : %d", ntohs(in.status)) ;
   printf("\nerror : %d\n", ntohs(in.error)) ;
   return ntohl(out.length);
}


u_long OpGet(char *TpExpr, char *Tuple)
{
   tsh_get_it out ;
   tsh_get_ot1 in1 ;
   tsh_get_ot2 in2 ;
   struct in_addr addr ;
   int sd, sock ;
   char *buff ;

   u_short this_op;

   // Sent tsh to server
   this_op = htons(TSH_OP_GET);
   if (!writen(tshsock, (char *)&this_op, sizeof(this_op)))
    {
       perror("\nOpGet:: op code sending error\n") ;
       return -1;
    }

   strcpy(out.expr, TpExpr);
				/* obtain port for return data if tuple not available */
   // This line has to revise for clusters. out.host = gethostid() ;	
   out.host = inet_addr("127.0.0.1");
   if ((sd = get_socket()) == -1) // Get aysynch return socket ready
    {
       perror("\nOpGet::return sockeet error\n") ;
       return -1;
    }
   if (!(out.port = bind_socket(sd, 0))) // Bind to random port
    {
       perror("\nOpGet::bind return socket error\n") ;
       return -2;
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
       close(sd) ; // Free return socket
       return -3;
    }
				/* find out if tuple available */
   if (!readn(tshsock, (char *)&in1, sizeof(in1)))
    {
       perror("\nOpGet::read ack error\n") ;
       close(sd) ;
       return -4;
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
       perror("\nOpGet::read tuple error\n") ;
       close(sd) ;
       return -5;
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
   Tuple = buff;
   strcpy(TpExpr, in2.name);
    return (ntohl(in2.length));
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

u_long OpRead(char *TpExpr, char *Tuple) {
   tsh_get_it out;
   tsh_get_ot1 in1;
   tsh_get_ot2 in2;
   struct in_addr addr;
   int sd, sock;
   char *buff;
   u_short this_op;

   // Sent tsh to server
   this_op = htons(TSH_OP_READ);
   if (!writen(tshsock, (char *)&this_op, sizeof(this_op)))
    {
       perror("\nOpGet:: op code sending error\n") ;
       return -1;
    }

    strcpy(out.expr, TpExpr);
				/* obtain port for return data if tuple not available */
   // This line has to revise for clusters. out.host = gethostid() ;	
   out.host = inet_addr("127.0.0.1");
   if ((sd = get_socket()) == -1) // Get aysynch return socket ready
    {
       perror("\nOpGet::return sockeet error\n") ;
       return -1;
    }
   if (!(out.port = bind_socket(sd, 0))) // Bind to random port
    {
       perror("\nOpGet::bind return socket error\n") ;
       return -2;
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
       close(sd) ; // Free return socket
       return -3;
    }
				/* find out if tuple available */
   if (!readn(tshsock, (char *)&in1, sizeof(in1)))
    {
       perror("\nOpGet::read ack error\n") ;
       close(sd) ;
       return -4;
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
       perror("\nOpGet::read tuple error\n") ;
       close(sd) ;
       return -5;
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
   Tuple = buff;
   strcpy(TpExpr, in2.name);
    return (ntohl(in2.length));
}
