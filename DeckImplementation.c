#include "DeckInterface.h"
#include <stdio.h>
#include <stdlib.h>

//int available[52];

int initialize_list(int *available){
    int i;
    for (i=0; i<52; i++)
        available[i]=1;
return 1;
}

int select_card(int *available){
    int num;
    int count=0;
    
    srand((unsigned int)time(NULL));
    num = rand() % 51;
    
    while (available[num] != 1){
        if (count > 52){
            return -1;
        }
        
        if (num < 52){
            num++;
        }
        else
            num = 0;
            
        count++;
    }
    
    available[num] = 0;
    
    return num;
}

void populate_deck(char deck[][2]){
    int count = 0, i, j;
    char suit = '\0';
    char value = '\0';
    
    for (i=0; i<4; i++){
        if (i == 0)
            suit = 'S';
        else if (i == 1)
            suit = 'H';
        else if (i == 2)
            suit = 'D';
        else
            suit = 'C';
          
        for (j=0; j<13; j++){
            
            if (j < 8){
                value = '2' + j;
            }
            else { 
                if (j == 8)
                    value = 'T';
                else if (j == 9)
                    value = 'J';
                else if (j == 10)
                    value = 'Q';
                else if (j == 11)
                    value = 'K';
                else if (j == 12)
                    value = 'A';
            }
            
            deck[count][0] = value;
            deck[count][1] = suit;
            count++;   
        }
    }
}

void create_deck(char deck[][2], int *list){
    initialize_list(list);
    populate_deck(deck);
}

int hand_value(int n){

    //find value of the card
    if (n >= 0 && n <= 8)
        n = n+2;
    else if (n >= 13 && n <= 21)
        n = n+2-13;
    else if (n >= 26 && n <= 34)
        n = n+2-26;
    else if (n >= 39 && n <= 47)
        n = n+2-39;
    else if (n == 12 || n == 25 || n == 38 || n == 51)
        n = -1;    
    else
        n = 10;
    //printf("value of the card is %d\n", n);
return n;
}
