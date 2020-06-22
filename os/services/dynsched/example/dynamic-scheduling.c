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
 *         A very simple Contiki application which performs operations
 *         periodically
 * \author
 *         Samuele Zoppi <samuele.zoppi@tum.de>
 */

#include "contiki.h"
#include "sys/node-id.h"
#include "net/nullnet/nullnet.h"
#include "net/netstack.h"
#include "net/mac/tsch/tsch.h"
#include "dev/serial-line.h"
#include "dev/leds.h" // Enables use of LEDs
#include "dyn_sched.h"
#include "dyn_sched_ser.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Dynamic-scheduling App"
#define LOG_LEVEL LOG_LEVEL_INFO

//LOG_LEVEL_NONE: Logs disabled,
//LOG_LEVEL_ERR: Errors,
//LOG_LEVEL_WARN: Errors and warnings,
//LOG_LEVEL_INFO: Errors, warnings, and information logs,
//LOG_LEVEL_DBG: All the above and debug messages,


void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
    uint8_t count;
    leds_on(LEDS_GREEN);
    memcpy(&count, data, sizeof(count));
    LOG_INFO("Received %u from ", count);
    LOG_INFO_LLADDR(src);
    LOG_INFO_("\n");
    LOG_INFO("Src Node ID: u8[6]:0x%x u8[7]:0x%x\n", 
	(src->u8[6] & 0xFF), (src->u8[7] & 0xFF));

    leds_off(LEDS_GREEN);
}

/* DEBUG: Used to debug local dynamic scheduling (without serial I/F)

// Callback function for ctimer
static struct ctimer dynsched_update_timer;
static uint16_t callback_cnt;


// Nodeid : Slot = 1:0, 2:1, 3:2 4:3  
uint16_t ds_arr1[DYNSCHED_TSCH_SCHEDULE_DEFAULT_LENGTH][4] = {
    {0, 0, 1, 1}, // t_slot:0 ch_off: 0 Link_opt:TX Nodeid: 1 (Coordinator usually)
    {1, 0, 1, 2}, // t_slot:1 ch_off: 0 Link_opt:TX Nodeid: 2
    {2, 0, 1, 3}, // t_slot:2 ch_off: 0 Link_opt:TX Nodeid: 3
	{3, 0, 1, 4},
};

// Nodeid : Slot = 1:0, 3:1, 2:2 4:3 
uint16_t ds_arr2[DYNSCHED_TSCH_SCHEDULE_DEFAULT_LENGTH][4] = {
    {0, 0, 1, 1}, // t_slot:0 ch_off: 0 Link_opt:TX Nodeid: 1 (Coordinator usually)
    {1, 0, 1, 3}, // t_slot:1 ch_off: 0 Link_opt:TX Nodeid: 3
    {2, 0, 1, 2}, // t_slot:2 ch_off: 0 Link_opt:TX Nodeid: 2
	{3, 0, 1, 4},
 };


static void dynsched_update_callback (void *ptr)
{
	LOG_INFO (" ***** Dynsched update callback ***** \n");
	// Use the two predefined schedules alternatively 
	if (callback_cnt % 2 == 0) {
	// tsch_schedule_create_custom(0); // To dynamically change Coordinators Schedule
		dynsched_update_network_schedules(DYNSCHED_TSCH_SCHEDULE_DEFAULT_LENGTH, ds_arr1);
	} else {
	// tsch_schedule_create_custom(1); // To dynamically change Coordinators Schedule
		dynsched_update_network_schedules(DYNSCHED_TSCH_SCHEDULE_DEFAULT_LENGTH, ds_arr2);
	}
	dynsched_print_network_schedules();
	callback_cnt++;
        ctimer_reset(&dynsched_update_timer);
}
*/

/*---------------------------------------------------------------------------*/
PROCESS(application_process, "Application process");
AUTOSTART_PROCESSES(&application_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(application_process, ev, data)
{
	static struct etimer et;
	static uint8_t cnt = 0;
	
	PROCESS_BEGIN();
	
	nullnet_buf = &cnt; /* Point NullNet buffer to payload i.e., cnt */
	nullnet_len = 1; /* Tell NullNet that the payload length is one byte */
	
	nullnet_set_input_callback(input_callback);

	if (node_id == 1) {
		LOG_INFO("Starting as Coordinator\n");
		tsch_set_coordinator(1);
		/* To start serial communication with NM */
		dynsched_start_serial_process();
	} else {	
		LOG_INFO("Starting as Node\n");
		tsch_set_coordinator(0);
	}


	/* Performs a periodic operation using a timer*/
	etimer_set(&et, CLOCK_SECOND * 5);
	
	/* DEBUG: Used to debug local dynamic scheduling
	if (tsch_is_coordinator) {
		// Callback timer to update schedules 
		//ctimer_set(&dynsched_update_timer, CLOCK_SECOND * 30, 
				//dynsched_update_callback, NULL);
	}
	*/

	while(1) {
		PROCESS_YIELD_UNTIL(etimer_expired(&et));
		if(tsch_is_associated){
			leds_on(LEDS_RED);

			NETSTACK_NETWORK.output(NULL); /* Send as broadcast */
			LOG_INFO("sent message %d\n", cnt);

			cnt++;
			leds_off(LEDS_RED);
			
			if (cnt % 5 == 0) {
				LOG_INFO("\n ***** Current schedule ***** \n");
				tsch_schedule_print();
 			}
			
			

		}
		etimer_restart(&et);
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/

