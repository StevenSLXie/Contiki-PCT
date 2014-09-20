#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "net/mac/cc2420-pct.h"
#include "dev/cc2420.h"

static uint8_t phase = 1;
static uint8_t received = 0;
static uint8_t lost = 0;
static int txpower;

#define LR 10
#define LN 4

void adjust_tx_power(uint8_t result){
    txpower = cc2420_get_txpower();
    if(0==phase){
		cc2420_set_txpower((uint8_t)(CC2420_TXPOWER_MAX-1));
        phase++;
    }
    switch(result){
		case 1:
    		if(1==phase){
				if(txpower>(CC2420_TXPOWER_MIN+1)){
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
    printf("Consecutive received num of pacs: %u\n",received);
}




