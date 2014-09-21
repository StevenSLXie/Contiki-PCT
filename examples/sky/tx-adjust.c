/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 * -----------------------------------------------------------------
 *
 * Author  : Xie Shuanglong (stevenslxie@gmail.com)
 * Created : 2014-09-26
 * 
 *
 * An application to automatically find the optimum TX power with respect to each receiver 
 *
 *  
 *  Based on the whether the other node can hear the message, increase or decrease the tx power.
 */

#include "contiki.h"
#include "net/rime.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "dev/cc2420.h"
#include <stdio.h>
#include <string.h>

#include "net/mac/cc2420-pct.h"

PROCESS(power_adjust_process, "TX Power Adjustment");
AUTOSTART_PROCESSES(&power_adjust_process);

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif


#define HEADER "RTST"
#define PACKET_SIZE 20
#define PORT 9345

#define MAX_NEIGHBORS 3
#define CYCLE 3

static uint8_t u8_0[MAX_NEIGHBORS*CYCLE] = {0};
static uint8_t head_ptr = 0;
static uint8_t ID = 1;
static uint8_t dest[MAX_NEIGHBORS-1] = {2,3};
static uint8_t node_ptr = 0;

struct pct_list{
	uint8_t list[MAX_NEIGHBORS*CYCLE];
};

static struct etimer send_timer, delete_timer;
static struct pct_list *rec_pac;
static struct pct_list *send_pac;

/*---------------------------------------------------------------------*/
static uint8_t
get_next(uint8_t *pos){
	if(*pos < MAX_NEIGHBORS*CYCLE-1){
		(*pos)++;
	}
    else
	{
		*pos = 0;
    }
	return *pos;
} 


/*---------------------------------------------------------------------------*/
static void
recv(struct unicast_conn *c, const rimeaddr_t *from)
{

  if(0){
	// invalid messgae.
  } else {

	// Upon receiving a packet, do 2 things: 
	// 1. check if my ID is on the message list.
	// if yes: decrease the power for that node;
    // if no:  increase the power for that node;
    // 2. include the ID of the sender to my next packet

    // first do the 1:
   
	uint8_t i,flag=0;
    rec_pac = packetbuf_dataptr();

	if(1){

    	PRINTF("received incoming packet.\n The msg is:");
    	for(i=0;i<MAX_NEIGHBORS*CYCLE;i++)
			PRINTF("%u ",rec_pac->list[i]);
		PRINTF("\n");


    	for(i=0;i<MAX_NEIGHBORS*CYCLE;i++){
			if(rec_pac->list[i]==rimeaddr_node_addr.u8[0]){
				flag = 1;
				break;		
			}	
		}

		adjust_tx_power(flag,from->u8[0]);	

    	flag = 0;

		// then do the 2:
    	// first change the local u8_0 list, then append it to the packet

    	u8_0[get_next(&head_ptr)] = from->u8[0];
    	PRINTF("the sender ID is:%u.\n",from->u8[0]);

    
	}		
	/* synchronize the sending to keep the nodes from sending
       simultaneously */
    //etimer_set(&send_timer, CLOCK_SECOND);
    //etimer_adjust(&send_timer, - (int) (CLOCK_SECOND >> 3));
    //PROCESS_CONTEXT_END(&radio_test_process);
	
  }
}
static const struct unicast_callbacks recv_call = {recv};
static struct unicast_conn pct;
/*---------------------------------------------------------------------*/
PROCESS_THREAD(power_adjust_process, ev, data)
{

  PROCESS_EXITHANDLER(unicast_close(&pct);)
    
  PROCESS_BEGIN();

  unicast_open(&pct, 146, &recv_call);  

  etimer_set(&send_timer, CLOCK_SECOND);
  etimer_set(&delete_timer, CLOCK_SECOND);
  //etimer_set(&no_res_timer,50*CLOCK_SECOND);

  rimeaddr_t my_addr;
 
  my_addr.u8[0] = ID;
  my_addr.u8[1] = 0;

  rimeaddr_set_node_addr(&my_addr);

  rimeaddr_t to_addr;

  uint8_t i;
  while(1) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_TIMER) {
      if(data == &send_timer) {
		etimer_set(&send_timer,5*CLOCK_SECOND);

		packetbuf_clear();
    	send_pac = (struct pct_list *)packetbuf_dataptr();
    	packetbuf_set_datalen(sizeof(struct pct_list));

    	PRINTF("The data prepared for sending is:");
		for(i=0;i<MAX_NEIGHBORS*CYCLE;i++){
			send_pac->list[i] = u8_0[i];    
			PRINTF("%u ",send_pac->list[i]);
		}
		PRINTF("\n");

		node_ptr++;

		if(node_ptr>(MAX_NEIGHBORS-2))
			node_ptr = 0;


		to_addr.u8[0] = dest[node_ptr];
        to_addr.u8[1] = 0;

    	//printf("the sender ID prepared is:%u.\n",send_pac->sender_ID);

		cc2420_set_txpower((uint8_t)(get_adjusted_tx_power(to_addr.u8[0])));

		if(!rimeaddr_cmp(&to_addr, &rimeaddr_node_addr)) {
      		unicast_send(&pct, &to_addr);
    	}
		printf("Sending a packet.\n");

      } 
	  else if(data == &delete_timer){
		  PRINTF("delete an old entry.\n");
          u8_0[get_next(&head_ptr)] = 0;
		  uint8_t sum = 0;
		  for(i=0;i<MAX_NEIGHBORS*CYCLE;i++){
			sum += u8_0[i];    
		  }
		  if(0 == sum){
			  printf("NO RESPONSE.");
		  	  for(i=0;i<MAX_NEIGHBORS-1;i++)
		      	adjust_tx_power(0,dest[i]);
          }
	  	  
		  etimer_reset(&delete_timer);
	  }
	
	}
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------*/
