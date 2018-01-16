#include        <sys/types.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <unistd.h>
#include        <errno.h>
#include        <string.h>
#include        <sys/socket.h>
#include        <netdb.h>
#include 		<sys/types.h>
#include 		<sys/time.h>

#define BUFLEN 512
#define NPACK 10
#define PORT 3543
#define ACK 1
#define NACK 0
#define WAIT 3
#define EMPTY "empty"


#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

void foundError(char* s)
{
	perror(s);
	exit(1);
}



char* sequencer(char* sequenceNumber)
{
	char *holder;
	if(strcmp(sequenceNumber,"0") == 0)
		holder = "1";
	else
		holder = "0";
	return holder;
}


int fileOffset= 0;
char* readFromFile()
{
	FILE *file;
	int i,size = 0;
	char* sendbuf;
	size_t nread;
	
	file = fopen("sendFile.txt", "r");
	sendbuf = (char*) malloc(sizeof(char)*4+1);
	fseek(file,fileOffset,SEEK_CUR);
	fread(sendbuf,4,1,file);
	fileOffset+=4;
	
	if (feof(file))
	{	
		if(strlen(sendbuf) == 0)	//Reached EOF BUT does buffer still has contents?
		{
			sprintf(sendbuf,"eof");
			fclose(file);
			return sendbuf;
		}
	}
	fclose(file);
	return sendbuf;
}


int main(int argc, char ** argv)
{
	int 				sd;
	struct sockaddr_in 	sockaddrOther;
	char				buffer[512];
	char 				c;
	int 				sockaddrOtherLen = sizeof(sockaddrOther);
	int 				bytes_read;
	char*				sequenceNumber = "0";
	int 				port;
	char* 				sendBuf;
	/* Timeout setup*/
	struct timeval timeout={3,0}; // n seconds 
	fd_set          	input_set;
	fd_set 				socks;
	/* Timeout setup*/

	if ( argc < 2 )
	{
		fprintf( stderr, "\x1b[1;31mEnter Port.  File %s line %d.\x1b[0m\n", __FILE__, __LINE__ );
		exit( 1 );
	}
	port = atoi(argv[1]);
	
	printf("\x1b[2;32;41mNote the host will auto default to 127.0.0.1 . Make sure to have another terminal up with the server!\x1b[0m\n");
	printf("Server: 127.0.0.1 , Timeout duration: 2 seconds, Port:%d\n",port);

	
	/* --------- Sockets, Servers, Oh my! ---------*/
	if ((sd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP )) == -1 ) 
		foundError("Socket()");
	
	memset((char *) &sockaddrOther, 0, sizeof(sockaddrOther));
	sockaddrOther.sin_family = AF_INET;
	sockaddrOther.sin_port = htons(port);
	sockaddrOther.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if( setsockopt(sd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval)) == -1 ) 
		foundError("Sockop\n");
	
	if (inet_aton("127.0.0.1", &sockaddrOther.sin_addr)==0) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	/* --------- Sockets, Servers, Oh my! ---------*/


	
	/* __________ Send / Recv packets! __________*/
	while(1)
	{	
//		printf("PACKET:");
//		if(fgets(buffer,60,stdin) != NULL)
//		{
			/* cleanup the \n at the end of array from fgets() */
//			char *pos;			
//			if ((pos = strchr(buffer, '\n')) != NULL) {
//				*pos = '\0';
//			}else{
//				while ((c = getchar()) != EOF && c != '\n');
//			}
//		}
		sendBuf = (char*)readFromFile(); 		// Got 4 bytes! (4chars)
		sequenceNumber = sequencer(sequenceNumber);
		
		printf("=====NEW PACKET | %s |=====\n",sendBuf);
		if(strcmp(sendBuf,"eof") != 0)			// Don;t add seqNum to sendBuf when EOF
			strcat(sendBuf,sequenceNumber);

			
		/* ---- Sending over Packet ---- */
		printf(CYAN"Sending: | %s |, SequenceNum:| %s | "RESET"\n",sendBuf,sequenceNumber);
		if( sendto(sd,sendBuf,sizeof(sendBuf),0,(struct sockaddr*)&sockaddrOther,sockaddrOtherLen) == -1) 
			foundError("sendto()");
		/* ---- Sending over Packet ---- */

	
		/* ==== Got something back? ==== */
		bytes_read = recvfrom(sd, buffer, sizeof(buffer),0, (struct sockaddr*)&sockaddrOther, &sockaddrOtherLen);
		if( bytes_read >= 0 && strcmp(buffer,"NACK") != 0) 			// Got something!
		{
			printf("Receving: %s\n",buffer);
		}
		else							// Nothing so far, keep resending
		{
			printf("\x1b[1;31mEntering Timeout Loop ====="RESET"\n");
			while(bytes_read <= 0 && strcmp(buffer,"NACK") != 0)		// Will stop when bytes_read picks up something (ACK)
			{
				if( sendto(sd,sendBuf,sizeof(sendBuf),0,(struct sockaddr*)&sockaddrOther,sockaddrOtherLen) == -1) 
					foundError("    sendto()");
				printf("\x1b[2;36m    Resending: %s | SequenceNum: %s  \x1b[0m\n",sendBuf,sequenceNumber);
				bytes_read = recvfrom(sd, buffer, sizeof(buffer),0, (struct sockaddr*)&sockaddrOther, &sockaddrOtherLen);
				printf("\x1b[2;31m    Timeout\x1b[0m\n");
			}
			printf("    Receving:  %s \n",buffer);
		}
		if(strcmp(sendBuf,"eof") == 0)
		{
			printf("END OF FILE\n");
			free(sendBuf);
			break;
		}
		free(sendBuf);
		/* ==== Got something back? ==== */
	}
	/* __________ Send / Recv packets! __________*/

	printf("function end\n");
	close(sd);
	return 0;
}