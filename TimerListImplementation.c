#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "TimerListInterface.h"

// reference http://www.c.happycodings.com/Data_Structures/code5.html

void initialize_timerList(struct timerNode *L){
    L->timer = 0;
    L->connectTime = -1;
    L->fd = -1;
    L->success = 0;
    
    L->next = NULL;
}

void add_node(struct timerNode *L){
    while(L->next != NULL){
        L = L->next;
    }
    L->next = (struct timerNode *) calloc(1, sizeof(struct timerNode));


    L->next->timer = 0;
    L->next->connectTime = -1;
    L->next->fd = -1;
    L->next->success = 0;
    
    L->next->next = NULL;

}

void delete_timerNode(struct timerNode *L, int num) {
    struct timerNode *temp;
    temp = (struct timerNode *)malloc(sizeof(struct timerNode));

    if(L->fd == num) {
        /* remove the node */
        temp = L->next;
        free(L);
        L = temp;
    } 
    else {
        while(L->next->fd != num)
            L = L->next;

            temp = L->next->next;
            free(L->next);
            L->next = temp;
    }   
}

struct timerNode * find_timerNode(struct timerNode *L, int num){

    if(L->fd == num) {
        return L;
    } 
    else {
        while(L->next->fd != num)
            L = L->next;

            return L->next;
    }
    
    printf("node not found\n");   
    return NULL;
}

int check_dealertimers(struct timerNode *L, fd_set *activefds){
    double delta;
    struct timerNode *C = L;
    
    while (L->next != NULL){
        if (L->timer != -1 && L->fd != -1){
            delta = difftime(time(NULL), L->connectTime);
            //printf("%lf\n", delta);
            
            if (delta > L->timer && L->timer != 0){
                printf("Disconnected %d: time limit exceeded: %lf\n", L->fd, delta);
                //printf("fd %d closed\n", L->fd);
                delete_timerNode(C, L->fd);
                close(L->fd);
		        FD_CLR(L->fd, activefds);
            }
        }
        L = L->next;
    }
    
    if (L->timer != -1 && L->fd != -1){
        delta = difftime(time(NULL), L->connectTime);
        //printf("%lf\n", delta);
        
        if (delta > L->timer && L->timer != 0){
            printf("Disconnected %d: time limit exceeded: %lf\n", L->fd, delta);
            //printf("fd %d closed\n", L->fd);
            delete_timerNode(C, L->fd);
            close(L->fd);
		    FD_CLR(L->fd, activefds);
        }
    }
    //display_timerList(C);
return 1;
}

int check_timers(struct timerNode *L, fd_set *activefds){
    double delta;
    struct timerNode *C = L;
    
    while (L->next != NULL){
        if (L->connectTime != -1 && L->fd != -1 && L->success == 0){
            delta = difftime(time(NULL), L->connectTime);
            //printf("%lf\n", delta);
            
            if (delta > L->timer && L->fd != -1){
                delete_timerNode(C, L->fd);
                close(L->fd);
		        FD_CLR(L->fd, activefds);
            }
        }
        else if (L->timer == 30){ //handle global timer
            delta = difftime(time(NULL), L->connectTime);
            //printf("global timer: %lf\n", delta);
            if (delta > L->timer){
                //printf("global timer complete\n");
                return 0;
            }
        }
        L = L->next;
    }
    
    if (L->connectTime != -1 && L->fd != -1 && L->success == 0){
        delta = difftime(time(NULL), L->connectTime);
        //printf("%lf\n", delta);
        
        if (delta > L->timer && L->fd != -1){
            delete_timerNode(C, L->fd);
            close(L->fd);
		    FD_CLR(L->fd, activefds);
        }
    }
    else if (L->timer == 30){ //handle global timer
        delta = difftime(time(NULL), L->connectTime);
        //printf("global timer: %lf\n", delta);
        if (delta > L->timer){
            printf("global timer complete\n");
            return 0;
        }
    }
    //display_timerList(C);
return 1;
}

void display_timerList(struct timerNode *L) {
    //L=L->next;
    while(L->next != NULL) {
        printf("timer:    %d\n", L->timer);
        printf("fd:       %d\n", L->fd);
        printf("success:  %d\n", L->success);
        L = L->next;
    }
    printf("timer:    %d\n", L->timer);
    printf("fd:       %d\n", L->fd);
    printf("success:  %d\n", L->success);
}

void free_list(struct timerNode *L){
    struct timerNode *temp=NULL;

   while( L != NULL ) {   /* whilst there are still nodes to delete     */
      temp = L->next;     /* record address of next node                */
      free(L);          /* free this node                             */
      L = temp;           /* point to next node to be deleted           */
   }

}

