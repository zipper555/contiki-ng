/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Dynamic Scheduling infrastructure in TSCH contiki-ng
 *
 * \author
 *         Sharada Prasad Shantharam  <sp.shantharam@tum.de>
 */

/* Dynamic Scheduling APIs */
#include "contiki.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "net/link-stats.h"
#include "net/mac/framer/framer-802154.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/mac-sequence.h"
#include "net/routing/routing.h"
#include "sys/node-id.h"
#include "dyn_sched.h"
#include <string.h>
#include "sys/clock.h"
#include "dev/leds.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "DYNSCHED TSCH"
#define LOG_LEVEL LOG_LEVEL_INFO



struct network_schedules net_schedules_links;


/*---------------------------------------------------------------------------*/
void dynsched_led_debug()
{
	leds_toggle(LEDS_BLUE);
}
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Updates Network Schedules struct used for EB creation - At Coordinator*/

void dynsched_update_network_schedules(int num_links, uint16_t arr[FRAME802154E_IE_MAX_LINKS][4])
{
    int i;
    net_schedules_links.num_links = num_links;

	if (num_links > FRAME802154E_IE_MAX_LINKS) {
		LOG_ERR("Could not update Network schedule\n");
		return;
	}
	LOG_INFO ("Updating Network schedule\n");
    for (i=0; i<num_links; i++) {
    	net_schedules_links.links[i].timeslot = arr[i][0];
    	net_schedules_links.links[i].channel_offset = arr[i][1];
       	net_schedules_links.links[i].link_options = (uint8_t)arr[i][2];
      	net_schedules_links.links[i].nodeid = (uint8_t)arr[i][3];
 	}
	
}

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Returns network schedules object */
struct network_schedules
dynsched_get_network_schedules (void) 
{
	return net_schedules_links;
}

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Prints out the populated Network schedules - At Coordinator */
void dynsched_print_network_schedules(void)
{
	int i;
	for (i=0; i<net_schedules_links.num_links; i++) {
		LOG_INFO ("Link[%d]: ", i);
		LOG_INFO ("Node id %d Timeslot %d Channel offset %d Linkopt %02x\n", 
				net_schedules_links.links[i].nodeid,
				net_schedules_links.links[i].timeslot,
				net_schedules_links.links[i].channel_offset,
				net_schedules_links.links[i].link_options);
	}		

}

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Creates schedule from the IE obtained from received EB - At Node*/
void dynsched_create_schedule_from_ies(struct ieee802154_ies ies0)
{
	int num_links;
	struct tsch_slotframe *sf;

	if(ies0.ie_tsch_slotframe_and_link.num_slotframes > 0) {
		LOG_INFO ("Updating schedule from EB IES\n");
		num_links = ies0.ie_tsch_slotframe_and_link.num_links;
		
		if (num_links > FRAME802154E_IE_MAX_LINKS) {
			LOG_ERR("! parse_eb: too many links in schedule (%u)\n", num_links);
			return;
		}
		
  		/* Evaluation */
  		long unsigned start, end, diff;
  		start = (unsigned long)clock_time();
		//LOG_WARN("Start: %lu TICKS", start);

		tsch_schedule_remove_all_slotframes();
		
		sf = tsch_schedule_add_slotframe(
          ies0.ie_tsch_slotframe_and_link.slotframe_handle,
          ies0.ie_tsch_slotframe_and_link.slotframe_size);
		
		for(int i = 0; i < num_links; i++) {
        	if (ies0.ie_tsch_slotframe_and_link.links[i].nodeid == node_id) {
        	/* If the schedule is destined to the node, then follow IE */
        		LOG_INFO ("Link for node: %d timeslot: %d\n linkopt: %d", node_id,
				ies0.ie_tsch_slotframe_and_link.links[i].timeslot,
				ies0.ie_tsch_slotframe_and_link.links[i].link_options);
				dynsched_led_debug();
        		
				tsch_schedule_add_link(sf, 
					ies0.ie_tsch_slotframe_and_link.links[i].link_options,
            		LINK_TYPE_NORMAL, &tsch_broadcast_address,
            		ies0.ie_tsch_slotframe_and_link.links[i].timeslot, 
	    			ies0.ie_tsch_slotframe_and_link.links[i].channel_offset);
        	} else {
        		/* 
				 * If not, add a default RX Link in that timeslot
         		 * Here, timeslot and the loop integer i are the same
	 			 * because every time slot has only one channel offset 
				 */
				tsch_schedule_add_link(sf,
            		LINK_OPTION_RX | LINK_OPTION_TIME_KEEPING,
            		LINK_TYPE_ADVERTISING, &tsch_broadcast_address,
            		i, 0);
        	}
      	}
		/* Evaluation */		
  		end = (unsigned long)clock_time();
		//LOG_WARN("End: %lu TICKS", end);
  		diff = end - start;
  		LOG_WARN("Diff= %lu ticks\n", diff);
	} else {
	LOG_INFO ("No schedule found in the EB\n");
	}	
}

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Create schedule from the input 2D array - At Coordinator*/
void
dynsched_schedule_create_from_array(int num_links, uint16_t arr[FRAME802154E_IE_MAX_LINKS][4])
{
  struct tsch_slotframe *sfa;
  int t_slot, ch_off;
  uint8_t link_opt; 
  enum link_type link_typ=LINK_TYPE_ADVERTISING; 
  linkaddr_t addr=tsch_broadcast_address;

  LOG_INFO("Creating schedule from array input\n");
  /* Evaluation */
  long unsigned start, end, diff;
  start = (unsigned long)clock_time();
  //LOG_WARN("Start: %lu ticks\n", start); 
  
  /* First, empty current schedule */
  tsch_schedule_remove_all_slotframes();
  /* Build 6TiSCH custom schedule.
   * We pick a slotframe length equal to the param num_links
   * And add two links according to the table */
  sfa = tsch_schedule_add_slotframe(0, num_links);
  for (int i = 0; i < num_links; i++) {
		/* If the link is meant for self node id then add the link */
		if ((uint8_t)arr[i][3] == node_id) {
			dynsched_led_debug();
			t_slot = arr[i][0];
			ch_off = arr[i][1];
            link_opt = (uint8_t)arr[i][2];
         	tsch_schedule_add_link(sfa,
            	(link_opt | LINK_OPTION_TIME_KEEPING), 
            	link_typ, &addr, t_slot, ch_off);
		} else {
		/* If not, Add a default RX Link in that timeslot
         * Here, timeslot and the loop integer i are the same
	 	 * because every time slot has only one channel offset */
			tsch_schedule_add_link(sfa,
            	LINK_OPTION_RX | LINK_OPTION_TIME_KEEPING,
            	link_typ, &tsch_broadcast_address,
            	i, 0);
    	}
  } 
  end = (unsigned long)clock_time();
  //LOG_WARN("End: %lu ticks\n", end);
  diff = end - start;
  LOG_WARN("Diff= %lu ticks\n", diff);
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Embeds Network Schedule into the IE during EB creation - At Coordinator*/
void dynsched_create_ies_sf_and_link(struct ieee802154_ies *ies0)
{
	LOG_INFO("Creating schedule in EB Packet\n");
	struct tsch_slotframe *sf0 = tsch_schedule_get_slotframe_by_handle(0);
    struct network_schedules ns = dynsched_get_network_schedules();
    int i;

   	if(sf0) {
      	ies0->ie_tsch_slotframe_and_link.num_slotframes = 1;
      	ies0->ie_tsch_slotframe_and_link.slotframe_handle = sf0->handle;
     	ies0->ie_tsch_slotframe_and_link.slotframe_size = sf0->size.val;
      	ies0->ie_tsch_slotframe_and_link.num_links = ns.num_links;
      
    	for (i=0; i<ies0->ie_tsch_slotframe_and_link.num_links; i++) {	
      		/* Populate links from dynamic_schedules struct*/
      		ies0->ie_tsch_slotframe_and_link.links[i].timeslot = 
			ns.links[i].timeslot;
      		ies0->ie_tsch_slotframe_and_link.links[i].channel_offset = 
			ns.links[i].channel_offset;
      		ies0->ie_tsch_slotframe_and_link.links[i].link_options = 
			ns.links[i].link_options;
      		ies0->ie_tsch_slotframe_and_link.links[i].nodeid = 
			ns.links[i].nodeid;
      	}
    }
	
}

/*---------------------------------------------------------------------------*/

