/********************************************
* File Name : DeckInterface.h
* Purpose : Contains function declarations.
* Creation Date : 05-04-2012
* Last Modified : Thur 05 Apr 2012 09:43:18 PM EST
* Created By :  Calvin Nichols  
********************************************/

struct timerNode {
    int timer;               
    time_t connectTime;
    int fd;
    int success;
    
    struct timerNode *next;       
};

struct timerNode * find_timerNode(struct timerNode *L, int num);

void initialize_timerList(struct timerNode *L);

int check_timers(struct timerNode *L, fd_set *activefds);

int check_dealertimers(struct timerNode *L, fd_set *activefds);

void display_timerList(struct timerNode *L);

void add_node(struct timerNode *L);

void free_list(struct timerNode *L);
