#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "net/mac/cc2420-pct.h"
#include "dev/cc2420.h"

#define MAX_NUM_OF_NEIGHBORS 3
#define LR 1
#define LN 10

/*
static uint8_t phase = 0;
static uint8_t received = 0;
static uint8_t lost = 0;
static uint8_t txpower = (CC2420_TXPOWER_MAX-1);
*/

typedef struct Neighbors{
   uint8_t phase;
   uint8_t received;
   uint8_t lost;
   uint8_t txpower;
   uint8_t u8_0;
   uint8_t u8_1;

} Neighbors;

static Neighbors nodes[MAX_NUM_OF_NEIGHBORS];
uint8_t cur_num_of_nodes = 0;

uint8_t search_table(uint8_t to){
	uint8_t i;	

	for(i=0;i<cur_num_of_nodes;i++){
		if(nodes[i].u8_0==to){
			return i;		
		}	
	}
    
    nodes[cur_num_of_nodes].phase = 1; 
    nodes[cur_num_of_nodes].received = 0;
    nodes[cur_num_of_nodes].lost = 0;
    nodes[cur_num_of_nodes].txpower = CC2420_TXPOWER_MAX-1;  
    nodes[cur_num_of_nodes].u8_0 = to;
    //nodes[cur_num_of_nodes].u8_1 = to[1];
    cur_num_of_nodes++;
    return (cur_num_of_nodes-1);
}


void adjust_tx_power(uint8_t result, uint8_t to){
	uint8_t n;
    n = search_table(to);
	//printf("the num of current nodes is: %u.\n",n);
    switch(result){
		case 1:
            if(1==nodes[n].phase || nodes[n].received==LR){
				if(nodes[n].txpower>(CC2420_TXPOWER_MIN+1)){
					//cc2420_set_txpower((uint8_t)(nodes[n].txpower--));
					nodes[n].txpower--;
				}  			
 				nodes[n].received = 0;
			}else{
				nodes[n].received++;
				nodes[n].lost = 0;	
				//cc2420_set_txpower((uint8_t)(nodes[n].txpower));							
			}
			break;
	    case 0:
			if(1==nodes[n].phase){
				nodes[n].phase++;
			}
            nodes[n].received = 0;
			nodes[n].lost++;
            if(nodes[n].lost==LN){
				if (nodes[n].txpower<(CC2420_TXPOWER_MAX-1)){
					//cc2420_set_txpower((uint8_t)(nodes[n].txpower++));
					nodes[n].txpower++;
				} 	
				nodes[n].lost = 0;
			}
			break;
	}
    printf("the observed value is: %u.\n",nodes[n].txpower);
    
}



uint8_t get_adjusted_tx_power(uint8_t to){
	uint8_t n;
	n = search_table(to);
	//printf("txpower for %u.%u is now adjusted to %u\n", to[0],to[1],nodes[n].txpower);
    return nodes[n].txpower;		
}

/*

void adjust_tx_power(uint8_t result, uint8_t *to){
    txpower = cc2420_get_txpower();
    if(0==phase){
		cc2420_set_txpower((uint8_t)(CC2420_TXPOWER_MAX-1));
        phase++;
    }
    switch(result){
		case 1:
    		if(1==phase){
				if(txpower>(CC2420_TXPOWER_MIN+1)){
					printf("Enter.\n");
					cc2420_set_txpower((uint8_t)(txpower-1));
				}    	
    		}
    		else if(received==LR){
				if (txpower>(CC2420_TXPOWER_MIN+1)){
					cc2420_set_txpower((uint8_t)(txpower-1));
				} 	
				received = 0;		
			}
			else{
				received++;
				lost  = 0;
			}
			break;
		case 0:
			if(1==phase){
				phase++;	
			}
			lost++;
			received = 0;
			if(lost==LN){
				if (txpower<(CC2420_TXPOWER_MAX-1)){
					cc2420_set_txpower((uint8_t)(txpower+1));
				} 	
				lost = 0;
			}
			break;
    }
    txpower = cc2420_get_txpower();
    printf("txpower is now adjusted to %u\n", txpower);
    //printf("Consecutive received num of pacs: %u\n",received);
}

*/



