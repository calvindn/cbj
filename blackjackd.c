#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "DeckInterface.h"
#include "TimerListInterface.h"

#define ERROR(x) do { perror(x); exit(1); } while(0)

#define NAME       257
#define BUFSIZE    257
#define MAXPENDING 1

static void s_handler(int sig){
    int status;
	while(waitpid(-1,&status,WNOHANG) > 0);
}

int success=0;

void handle_connection(int NS, fd_set * activefds);
void check_passcode(int NS, fd_set * activefds, struct timerNode *L);

int main(int argc, char *argv[])
{
    struct timerNode * timerL, * Lcopy;
	fd_set readfds, activefds;	/* the set of read descriptors */
	unsigned short port;		/* Port to which server will bind */
	char servhost[NAME];		/* Local host name */
	struct sockaddr_in sock;	/* INTERNET socket space */
	struct hostent *server;		/* Local host information */
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int S;						/* fd for socket */
	int NS;						/* fd for connected socket */
	int pid, status;					/* used to determine parent or child */
	int i;						/* counter to go through FDs in fdset */
    int len, bytes_sent;
    int j, flags, global_cd=1;
    struct timeval tv;

    signal(SIGCHLD, s_handler);

    /* Initialize timer list */
    timerL = (struct timerNode *)calloc(1, sizeof(struct timerNode));
    initialize_timerList(timerL);

	if (argc != 2)
	{
		fprintf(stderr,"usage: select-server <port>\n");
		exit(1);
	}

	port = atoi(argv[1]);
    //port = 2016;
	/*
	 * Get socket - INTERNET DOMAIN - TCP
	 */
	if ((S = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		ERROR("select-server: socket");

	/*
	 * Obtain host name & network address
	 */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		ERROR("Problem getting server address information");
		return 1;
	}


	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((S = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			ERROR("server: socket");
			continue;
		}

		/* Bind socket to port/addr */
		if (bind(S, p->ai_addr, p->ai_addrlen) == -1) {
			close(S);
			ERROR("server: bind");
			continue;
		}

		break;
	}

	/*
	 * Listen on this socket
	 */
	if (listen(S,MAXPENDING) < 0)
		ERROR("server: listen");

	/* Initialize the set of active sockets. */
    FD_ZERO (&activefds);
    FD_SET (S, &activefds);

	while(1)
	{
	    tv.tv_sec = 1;
        tv.tv_usec = 0;
	    //fprintf(stderr, "%d\n", time(NULL));
		/* Block until input arrives on one or more active sockets. */
        readfds = activefds;
        if (select (FD_SETSIZE, &readfds, NULL, NULL, &tv) < 0)
        {
			perror ("select");
            exit (EXIT_FAILURE);
        }
		//check_timers(timerL, &activefds);
		/* Service all the sockets with input pending. */
		for (i = 0; i < FD_SETSIZE; ++i) //cycle through all fd
		{
			if (FD_ISSET (i, &readfds)) //check that fd is part of the set
			{
				if (i == S)
				{

					 //wait for a few seconds for passcode, if it is not received skip this next part.
					if ((NS = accept(S,NULL,NULL)) < 0)
						ERROR("server: accept");
					FD_SET(NS, &activefds);	//add the new socket desc to our active connections set

					if (timerL->timer == 0){
					    timerL->timer = 30;
					    timerL->connectTime = time(NULL);
					    timerL->fd = -1;
					}

					Lcopy = timerL;

					add_node(timerL);
					while(timerL->next != NULL){
                        timerL = timerL->next;
                    }
					timerL->timer = 5;
					timerL->connectTime = time(NULL);
					timerL->fd = NS;

					timerL = Lcopy;

				}
				else
				{
					/* Data arriving on an already-connected socket. */
					check_passcode(NS, &activefds, timerL);
				}
			}
		} /* //end of for */
		//check time list here, still need to accept password
		global_cd = check_timers(timerL, &activefds);
		//printf("success: %d\n", success); //success as a local variable is being reset...
		//printf("global_cd: %d\n", global_cd);

		if (success == 4 || global_cd == 0){ //if we have 4 connections or reach the 30 second timer then call dealer
		    pid = fork();
            if (pid == 0){
                execl("./dealer", "./dealer", NULL);
            }
            else if (pid < 0){
                perror("fork failed");
                exit(0);
            }
            else {
                //parent
                wait(NULL);
                //free list
                free_list(timerL);
                timerL = NULL;
                success = 0;
		        global_cd = 1;
		        FD_ZERO (&activefds);
		        for (i = 5; i<15; i++)
		            close(i);
		        FD_SET (4, &activefds);
                /* Initialize timer list */
                timerL = (struct timerNode *)calloc(1, sizeof(struct timerNode));
                initialize_timerList(timerL);
                //display_timerList(timerL);
            }
		}
	} /* //end of while */

	return(0);
}

void check_passcode(int NS, fd_set * activefds, struct timerNode *L)
{
    struct timerNode *current;
    char recv_buf[sizeof(unsigned long)];
    unsigned long pass=0;

    recv(NS, recv_buf, sizeof(unsigned long), 0); //recieve number
    pass = ntohl(*((unsigned long*)recv_buf));    //convert back to host byte order
    //printf("passCode: %lu\n", pass);

    current = find_timerNode(L, NS);
    if (pass == 0xfacebeef && current->success == 0){
        //no more need for a timer for this node
        //delete_timerNode(L, NS);
        current->success = 1;
        success++;
    }
    else {
        if (current->success == 1)
            success--;

        delete_timerNode(current, NS);
        close(NS);
	    FD_CLR(NS, activefds);
	}
}
