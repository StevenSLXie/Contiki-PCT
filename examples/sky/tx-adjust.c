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
 * Author  : Adam Dunkels, Joakim Eriksson, Niclas Finne
 * Created : 2006-03-07
 * Updated : $Date: 2010/01/15 10:32:36 $
 *           $Revision: 1.6 $
 *
 * Simple application to indicate connectivity between two nodes:
 *
 * - Red led indicates a packet sent via radio (one packet sent each second)
 * - Green led indicates that this node can hear the other node but not
 *   necessary vice versa (unidirectional communication).
 * - Blue led indicates that both nodes can communicate with each
 *   other (bidirectional communication)
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

PROCESS(radio_test_process, "Radio test");
AUTOSTART_PROCESSES(&radio_test_process);


#define HEADER "RTST"
#define PACKET_SIZE 20
#define PORT 9345

#define MAX_NEIGHBORS 3
#define CYCLE 3

static uint8_t u8_0[MAX_NEIGHBORS*CYCLE] = {0};
static uint8_t node_ptr = 0;
static uint8_t ID = 73;

struct pct_list{
	uint8_t list[MAX_NEIGHBORS*CYCLE];
    uint8_t sender_ID;
};

static struct etimer send_timer, delete_timer;


/*---------------------------------------------------------------------*/
static uint8_t
point_next(){
	if(node_ptr < MAX_NEIGHBORS*CYCLE-1){
		node_ptr++;
		return node_ptr;
	}
    else
		return 0;
} 

static uint8_t
get_point(){
	if (node_ptr == MAX_NEIGHBORS*CYCLE-1){
		return 0;	
	}
	else
		return (node_ptr+1);
}



/*---------------------------------------------------------------------------*/
static void
abc_recv(struct abc_conn *c)
{
  /* packet received */
  //if(packetbuf_datalen() < sizeof(uint8_t)*MAX_NEIGHBORS*CYCLE
  //   || strncmp((char *)packetbuf_dataptr(), HEADER, sizeof(HEADER))) {
    /* invalid message */
  if(0){
  } else {
    PROCESS_CONTEXT_BEGIN(&radio_test_process);


	// Upon receiving a packet, do 2 things: 
	// 1. check if my ID is on the message list.
	// if yes: decrease the power for that node;
    // if no:  increase the power for that node;
    // 2. include the ID of the sender to my next packet

    // first do the 1:
    struct pct_list *rec_pac;
	uint8_t i,flag=0;
    rec_pac = packetbuf_dataptr();

    printf("received incoming packet.\n The msg is:");
    for(i=0;i<MAX_NEIGHBORS*CYCLE;i++)
		printf("%u ",rec_pac->list[i]);
	printf("\n");

    for(i=0;i<MAX_NEIGHBORS*CYCLE;i++){
		if(rec_pac->list[i]==ID){
			adjust_tx_power(1,rec_pac->sender_ID);
			flag = 1;
			break;		
		}	
	}

	if(0==flag){
		adjust_tx_power(0,rec_pac->sender_ID);			
	}

    flag = 0;

	// then do the 2:
    // first change the local u8_0 list, then append it to the packet
    u8_0[point_next()] = rec_pac->sender_ID;
    printf("the sender ID is:%u.\n",rec_pac->sender_ID);

    packetbuf_clear();
    struct pct_list *send_pac;
    send_pac = (struct pct_list *)packetbuf_dataptr();
    packetbuf_set_datalen(sizeof(struct pct_list));

    printf("The data prepared for sending is:");
	for(i=0;i<MAX_NEIGHBORS*CYCLE;i++){
		send_pac->list[i] = u8_0[i];    
		printf("%u ",send_pac->list[i]);
	}
	printf("\n");

	send_pac->sender_ID = ID;
    printf("the sender ID prepared is:%u.\n",send_pac->sender_ID);
	
	/* synchronize the sending to keep the nodes from sending
       simultaneously */
    etimer_set(&send_timer, CLOCK_SECOND);
    etimer_adjust(&send_timer, - (int) (CLOCK_SECOND >> 1));
    PROCESS_CONTEXT_END(&radio_test_process);
  }
}
static const struct abc_callbacks abc_call = {abc_recv};
static struct abc_conn abc;
/*---------------------------------------------------------------------*/
PROCESS_THREAD(radio_test_process, ev, data)
{
  PROCESS_BEGIN();

  abc_open(&abc, PORT, &abc_call);
  etimer_set(&send_timer, CLOCK_SECOND);
  etimer_set(&delete_timer,1.1*CLOCK_SECOND);


  while(1) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_TIMER) {
      if(data == &send_timer) {
		etimer_reset(&send_timer);
		//packetbuf_set_datalen(sizeof(uint8_t)*MAX_NEIGHBORS*CYCLE);

		abc_send(&abc);
		printf("Sending a packet.\n");

      } 
	  else if(data == &delete_timer){
	  	  u8_0[get_point()] = 0;
		  etimer_reset(&delete_timer);
	  }
	}
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------*/
