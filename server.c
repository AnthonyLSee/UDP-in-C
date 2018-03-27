#include        <sys/types.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <unistd.h>
#include        <errno.h>
#include        <string.h>
#include        <sys/socket.h>
#include        <netdb.h>
#include 		<sys/wait.h>
#include 		<limits.h>


#define BUFLEN 512
#define NPACK 10
#define PORT 3543
#define ACK 1
#define NACK 0


#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"


void foundError(char* s)
{
	/* foundError()
	 *	- External debuggig funciton
	 */
	perror(s);
	exit(1);
}


void rebuildFile(char* packet)
{
	/* rebuildFile()
	 *	- Grabs a string / packet
	 *	- Uses File IO and saves packet.
	 */
	FILE *fp;
	fp = fopen("recvFile.txt", "a");
	if ( strlen(packet) < 4) fprintf(fp, "%s\n",packet);
	else 	fprintf(fp, "%s",packet);
	fclose(fp);
}
	
	
char sequencer(char sequenceNumber)
{
	/* sequencer()
	 *	- provides either 1 or 0 
	 *	- depends on previous value (if 1,then 0. If 0, then 1)
	 */
	char holder;
	if(sequenceNumber == '0')
		holder = '1';
	else
		holder = '0';
	return holder;
}
	

int main(int argc, char * argv[] )
{
	/* VARS! */
	struct sockaddr_in sockaddr,sockaddrOther;
	int 			sd;
	char 			buffer[512];
	int 			addrlen;
	int 			on = 1;
	char* 			ack = "ack";
	int 			bytes_read,send_read;
	char			response[2048];
	char 			*p;
	float 			dropRate;
	int 			port;
	char			expectedSeq = '1';
	char			recievedSeq;
	/* VARS! */
	
	if ( argc < 3 )
	{
		printf("USAGE: ./server <PORT> <DROP RATE>\nDROP RATE must be between 0 and 100\n");
		exit( 1 );
	}
	port = atoi(argv[1]);			// char->int
	dropRate = strtof(argv[2],&p);	// char->float
	
	if (dropRate > 100) 
		foundError("Not between 0-100\n");
	else
	{
		printf("DROP RATE : %f%% , PORT: %d\n", dropRate,port);
		dropRate = dropRate *.01;
	}
	
	
	/* ======== Sockets, binds, oh my! ======== */
	if ( (sd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) foundError("socket()\n");
	printf("Socket() successful\n");
	
	memset(&sockaddr, 0, sizeof(sockaddr)); 
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); //from any sender
	 
	if ( setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) == -1 ) foundError("setsockopt()\n");
	printf("setsockopt() successful\n");

	if( bind( sd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) == -1) foundError("bind()");
	printf("Bind() successful\n");
	/* ======== Sockets, binds, oh my! ======== */
	
	
	/* =========== Receving Packets! ========== */
	int temp = 0;
	while(bytes_read = recvfrom(sd, buffer, sizeof(buffer),0, (struct sockaddr*)&sockaddrOther, &addrlen) != -1)
	{	

		if( temp == 0 )
		{
			printf("Loading...\n");
			temp++;
		}
		
		else if (drand48() <= dropRate) printf("\x1b[1;31mDrop packet from %d\x1b[0m" RESET"\n",ntohs( sockaddrOther.sin_port ));
		
		else
		{
			/*
			Does packet sequnum == next expected seqnum?
				If so, "next expected seq num" += 1, process packet
			Is packet seqnum < next expected sequnum?
				Packet is a duplicate, ok to drop
			Is packet seqnum > next expected sequnum?
				Packet is out of seq, receiver is not ready for this packet
			*/
			
			printf(CYAN"=== RECEIVED: | %s | from : | %d | ===" RESET "\n", buffer,ntohs( sockaddrOther.sin_port ));
			if( strcmp(buffer,"eof") != 0)								// is it EOF?
			{															// Nope not EOF
				
				recievedSeq = buffer[strlen(buffer)-1];
				printf("    recievedSeq: | %c |, expectedSeq: | %c |\n", recievedSeq,expectedSeq);
		
				if( expectedSeq == recievedSeq)							// The seq number is the same!
				{	
					expectedSeq = sequencer(expectedSeq);				// Swap seq num
					printf("    Same seq number\n");
					buffer[strlen(buffer)-1] = '\0'; 					// REMOVE SEQ NUMBER!
					rebuildFile(buffer);								// Add buffer to file
					printf("    Added |%s| recvFile, sending ACK\n",buffer);
					sprintf(buffer,"ACK\n");							// Sendover ACK
				}
				else													// Seq num are not the same!
				{
					printf("    Wrong seq number, drop packet\n");
					sprintf(buffer,"NACK\n");							// Sendover ACK
				}
			}
			else 														// Yes it is EOF
			{
				printf("\x1b[2;32;47mEnd of File of | %d |!\x1b[0m\n",ntohs( sockaddrOther.sin_port ));					
				expectedSeq = '1';										// Reset sequence for next file
			}
			if(send_read = sendto(sd,buffer,sizeof(buffer),0,(struct sockaddr*)&sockaddrOther,addrlen) == -1) 
				foundError("sendto()");
		}
		printf("\n");
	}
	/* =========== Receving Packets! ========== */

	close(sd);
	printf("end\n");
    return 0;
}
	
	
