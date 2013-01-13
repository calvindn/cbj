#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ERROR(x) do { perror(x); exit(1); } while(0)

#define BUFSIZE 1024

int main(int argc, char *argv[])
{
	char *hostname;			/* Host on which to connect to server */
	unsigned short port;	/* Port on which server is listening  */
	int S;					/* fd for socket */
	char buf[BUFSIZE];		/* Input/output buffer */
	FILE *fp;				/* Stream (converted file des.) */
    int c, len;
	struct sockaddr_in sock;	/* INTERNET socket space */
	struct hostent *remote;		/* Remote host information */
    uint32_t passcode = 0xfacebeef;
	uint32_t nbo;
	if (argc != 3)
	{
		fprintf(stderr,"usage: client <hostname> <port>\n");
		exit(1);
	}

	hostname = argv[1];
	port = atoi(argv[2]);

	/*
	 * Obtain socket - INTERNET DOMAIN - TCP
	 */
	if ((S = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		ERROR("client: socket");

	/*
	 * Get network address of server
	 */
	if ((remote = gethostbyname(hostname)) == NULL)
	{
		fprintf(stderr,"%s: unknown host\n", hostname);
		exit(1);
	}

	/*
	 * Set address of server - converting to network byte order
	 */
	sock.sin_family = AF_INET;
	sock.sin_port = htons(port);
	memcpy(&sock.sin_addr,remote->h_addr,remote->h_length);

	/*
	 * Attempt connection to server - must be listening for success
	 */
	if (connect(S, (struct sockaddr *)&sock, sizeof(sock)) < 0)
		ERROR("client: connect");
    //else
        //send(facebeef)
	/*
	 * Using stdio library to write to socket
	 */
	if (!(fp = fdopen(S,"r")))
	{
		fprintf(stderr,">>> Error converting file des. to stream <<<\n");
		exit(1);
	}

    //int to send
    nbo = htonl(passcode);
	/*
	 * Read input from terminal, echo to server (terminate on newline)
	 */

	send(S, &nbo, sizeof(uint32_t), 0);




	while(1)
	{
	    //sleep(1);
		/*fgets(buf,BUFSIZE,stdin);
		if (*buf == '\n')
			break;
		else
		{
			fputs(buf,fp);
			fflush(fp);
		}*/
	    recv(S,buf, BUFSIZE, 0);
	    printf("recieved: %s\n", buf);

		printf("stand 1, hit 2\n");
		scanf("%d", &c);
		getchar(); //\n
		if (c == 1){
		    len = strlen("STAND");
            send(S, "STAND\0", len+1, 0);
	    }
	    else if (c == 2){
	        len = strlen("HIT");
            send(S, "HIT\0", len+1, 0);
	    }
	}

	/*
	 * DONE - simply close() connection
	 */
	fclose(fp);
	close(S);

	return(0);

}
