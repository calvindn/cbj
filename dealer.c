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

int main (){
    fd_set readfds, activefds;
    struct timerNode * timerL, * Lcopy;
    struct timeval tv;
    struct timerNode *current;
    int sum[11]; //sum of each players cards
    int deckBoolean[52];
    char buf[10];
    char deck[52][2], dealerStr1[3], dealerStr2[3], sendStr[20], dealerH[10];
    int i, j, card, dealerhand, playerhand, temp, value, len, rv;

    //initialize user card array
    memset(sum, 0, sizeof sum);    

    //create deck of cards
    create_deck(deck, &deckBoolean[0]);
    
    //initialize timer list
    timerL = (struct timerNode *)calloc(1, sizeof(struct timerNode));
    initialize_timerList(timerL);
    
    FD_ZERO (&activefds);
    FD_SET(4, &activefds); //4 is the master socket
    /*FD_SET(5, &activefds);
    FD_SET(6, &activefds);
    FD_SET(7, &activefds);
    FD_SET(8, &activefds);*/
    
    /*  --select dealer card 1--  */
    card = select_card(&deckBoolean[0]);
    if (card == -1)
        printf("deck empty\n");
    else {
        dealerStr1[0] = '\0';
        strncat(dealerStr1, &deck[card][0], 1);
        strncat(dealerStr1, &deck[card][1], 1);
        //printf("dealerStr1: %s\n", dealerStr1);
        value = hand_value(card);
        if (value == -1){
            temp = sum[0] + 11;
            if (temp <= 21)
                sum[0] = sum[0] + 11;
            else
                sum[0] = sum[0] + 1;
        }
        else {
            sum[0] = sum[0] + value;
        }
        //printf("dealer card 1 card: %d, value: %d\n", card, value);
    }
    
    /*  --select dealer card 2--  */
    card = select_card(&deckBoolean[0]);
    if (card == -1)
        printf("deck empty\n");
    else {
        dealerStr2[0] = '\0';
        strncat(dealerStr2, &deck[card][0], 1);
        strncat(dealerStr2, &deck[card][1], 1);
        //printf("dealerStr2: %s\n", dealerStr2);
        value = hand_value(card);
        if (value == -1){
            temp = sum[0] + 11;
            if (temp <= 21)
                sum[0] = sum[0] + 11;
            else
                sum[0] = sum[0] + 1;
        }
        else {
            sum[0] = sum[0] + value;
        }
    }
    
    //send out player cards
    for (i=5; i<15; i++){
        
        sendStr[0] = '\0';
        
        //card selection
        for (j=0; j<2; j++){
            card = select_card(&deckBoolean[0]);
            if (card == -1)
                printf("deck empty\n");
            else {
                strncat(sendStr, &deck[card][0], 1);
                strncat(sendStr, &deck[card][1], 1);
            }
            //printf("sendStr: %s\n", sendStr);
            //bookkeeping cards
            value = hand_value(card);
            if (value == -1){
                    sum[i-4] = sum[i-4] + 1;
            }
            else {
                sum[i-4] = sum[i-4] + value;
            }
        }    
        
        //add dealer card
        strncat(sendStr, dealerStr1, 2);
        
        //send cards
        len = strlen(sendStr);
        if(rv = send(i, sendStr, len+1, 0) == -1){
            //printf("send failed, will not add to list\n");
        }
        else {
            //add timer to list
            Lcopy = timerL;
			
			add_node(timerL);
			while(timerL->next != NULL){
                timerL = timerL->next;
            }
			timerL->timer = 10;
			timerL->connectTime = time(NULL);
			timerL->fd = i;
			
			timerL = Lcopy;
            FD_SET(i, &activefds);
        }
    }
    
    
    /* -- Create Dealers Hand -- */
    dealerH[0] = '\0';
    strncat(dealerH, dealerStr2, 2);
    dealerhand = sum[0];
    
    while (dealerhand < 17){ //continue to hit
        
        card = select_card(&deckBoolean[0]);
        if (card == -1)
            printf("deck empty\n");
        else {
            strncat(dealerH, &deck[card][0], 1);
            strncat(dealerH, &deck[card][1], 1);
            value = hand_value(card);
            if (value == -1){
                temp = sum[0] + 11;
                if (temp <= 21)
                    sum[0] = sum[0] + 11;
                else
                    sum[0] = sum[0] + 1;
            }
            else {
                sum[0] = sum[0] + value;
            }
        }
        dealerhand = sum[0]; //dealer hand value
    }
    
    //printf("dealer hand is %s\n", dealerH);
    
    //wait for response
    while(1)
	{
	    //display_timerList(timerL);
	    tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        /* Block until input arrives on one or more active sockets. */
        readfds = activefds;
        if (select (FD_SETSIZE, &readfds, NULL, NULL, &tv) < 0)
        {
			perror ("select");
            exit (EXIT_FAILURE);
        }
        
        /* Service all the sockets with input pending. */
		for (i = 0; i < FD_SETSIZE; ++i) //cycle through all fd
		{
			if (FD_ISSET (i, &readfds)) //check that fd is part of the set
			{
				if (i > 4)
				{
				    /* Data arriving on an already-connected socket. */
				    recv(i, buf, 10, 0);
				    
				    if (strcmp(buf, "STAND") == 0){
				        //send dealer cards
				        len = strlen(dealerH);
	                    send(i, dealerH, len+1, 0);
				        
				        //disconnect
				        printf("Disconnected %d: player stood\n", i);
				        delete_timerNode(timerL, i);
	                    close(i);
	                    FD_CLR(i, &activefds);
				    }
				    else if (strcmp(buf, "HIT") == 0){
				    
				        //reset timer
				        current = find_timerNode(timerL, i);
	                    current->connectTime = time(NULL);
				        
				        //deal another card
	                    sendStr[0] = '\0';
	                    card = select_card(&deckBoolean[0]);
                        if (card == -1)
                            printf("deck empty\n");
                        else {
                            strncat(sendStr, &deck[card][0], 1);
                            strncat(sendStr, &deck[card][1], 1);
                        } 
                        
                        //bookkeeping cards
                        value = hand_value(card);
                        if (value == -1){
                                sum[i-4] = sum[i-4] + 1;
                        }
                        else {
                            sum[i-4] = sum[i-4] + value;
                        }
                        
                        //send card
                        len = strlen(sendStr);
                        if(rv = send(i, sendStr, len+1, 0) == -1){
                            printf("send failed, removing from list\n");
                            delete_timerNode(timerL, i);
                            close(i);
                            FD_CLR(i, &activefds);
                        }
                        else {
                            playerhand = sum[i-4];
                            
                            //disconnect
                            if (playerhand > 21){
                                printf("Disconnected %d: player busts\n", i);
                                delete_timerNode(timerL, i);
	                            close(i);
	                            FD_CLR(i, &activefds);
                            }
                        }
                    }
                    else {
                        printf("Disconnected %d: improper data received\n", i);
                        delete_timerNode(timerL, i);
                        close(i);
                        FD_CLR(i, &activefds);
                    }
                }
            }
        }
        
    	check_dealertimers(timerL, &activefds);
		
		if (!(FD_ISSET(5, &activefds)) && !(FD_ISSET(6, &activefds)) && !(FD_ISSET(7, &activefds)) && !(FD_ISSET(8, &activefds)) && !(FD_ISSET(9, &activefds)) && !(FD_ISSET(10, &activefds)) && !(FD_ISSET(11, &activefds)) && !(FD_ISSET(12, &activefds)) && !(FD_ISSET(13, &activefds)) && !(FD_ISSET(14, &activefds)) && !(FD_ISSET(15, &activefds))){
		    for (i=5; i<9; i++)
		        printf("%d = %d\n", i, sum[i-4]);
	        return 0;
	    }
    
    }
return 0;
}
