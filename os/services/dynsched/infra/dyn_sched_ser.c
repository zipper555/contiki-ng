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
 *        Serial communication infrastructure for 
 *		  Dynamic Scheduling in TSCH contiki-ng
 *
 * \author
 *         Sharada Prasad Shantharam  <sp.shantharam@tum.de>
 */

#include "contiki.h"
#include "sys/node-id.h"
#include "net/nullnet/nullnet.h"
#include "net/netstack.h"
#include "net/mac/tsch/tsch.h"
#include "dev/serial-line.h"
#include "dev/leds.h" // Enables use of LEDs
#include "dyn_sched.h"


/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "DYNSCHED SER"
#define LOG_LEVEL LOG_LEVEL_INFO


/* Serial input string parsing */
#define SERIAL_STR_START_OF_LINKS 3
#define SERIAL_STR_NEXT_LINK 11

/* Number of links in the schedule */
int links_count;

// Initialize default values for serial schedules 
uint16_t ds_ser[DYNSCHED_TSCH_SCHEDULE_DEFAULT_LENGTH][4] = {
    {0, 0, 1, 1}, // t_slot:0 ch_off: 0 Link_opt:TX Nodeid: 1 (Coordinator usually)
    {1, 0, 1, 2}, // t_slot:1 ch_off: 0 Link_opt:TX Nodeid: 2
    {2, 0, 1, 3}, // t_slot:2 ch_off: 0 Link_opt:TX Nodeid: 3
	{3, 0, 1, 4}, // t_slot:3 ch_off: 0 Link_opt:TX Nodeid: 4
};

/*************************************************************/
/* Process for serial communication */
PROCESS(serial_process, "Serial process");

/*************************************************************/
/* Parse and populate schedule from serial input */
static int parse_serial_schedule_input(char* serial_ip)
{
	int ds[DYNSCHED_TSCH_SCHEDULE_DEFAULT_LENGTH][4];
	int match, link;
	int row, ts, choff, lo, nid;
	LOG_INFO("String length: %d\n", strlen(serial_ip));
	/*
	for (int k=0; k<6; k++) {
		LOG_INFO("Data1: %c\n", *serial_ip);
		serial_ip++;
	}
	*/
	if (*serial_ip == 'N') {
		LOG_DBG("Start of string identified\n");
		match = sscanf(serial_ip, "N%d", &links_count);
		if ((match < 1) || (strlen(serial_ip) < links_count * 11 + 3)) {
			LOG_ERR("Error in serial string format\n");
			return -1;
		}
		serial_ip += SERIAL_STR_START_OF_LINKS;//Bring ptr to start of linkinfo
		LOG_DBG("Current string:%s", serial_ip);
		for (link = 0; link<links_count; link++) {
			match = sscanf(serial_ip, "L%d %d,%d,%d,%d ",
							&row, &ts, &choff, &lo, &nid);
			LOG_DBG("Match:%d\n", match);
			LOG_DBG("row:%d ts:%d choff:%d lo:%d, nid:%d",
						row, ts, choff, lo, nid);
			if (!match) {
				LOG_ERR("Error in serial string: Link %d", link);
				return -1;
			}
			/* If matches are found as per format, populate local array */
			ds[row][0] = ts;
			ds[row][1] = choff;
			ds[row][2] = lo;
			ds[row][3] = nid;
			
			/* Set ptr to next link */
			serial_ip += SERIAL_STR_NEXT_LINK;
		}	
	}
	
	/* 
 	 * If parsing goes fine, then 
     * Assign ds back to the global variable ds_ser 
	 */
	for (int i=0; i<DYNSCHED_TSCH_SCHEDULE_DEFAULT_LENGTH; i++) {
		for (int j=0; j<4; j++) {
			ds_ser[i][j] = (uint16_t)ds[i][j];
		}
	}
	return 0;
}

/*************************************************************/
/* Print schedule populated from serial input */
static void dynsched_print_serial_schedules()
{
	int i;
	LOG_INFO("\nSchedule from serial input\n");
	for (i=0; i<links_count; i++) {
		LOG_INFO("Timeslot %d Channel offset %d Linkopt %02x Nodeid %d\n",
				ds_ser[i][0], ds_ser[i][1], ds_ser[i][2], ds_ser[i][3]);
		/* Fix me: Stop printing default values */
	}
}

/*************************************************************/
/* Function to start Serial process */
void dynsched_start_serial_process()
{
	process_start(&serial_process, NULL);
}


/*************************************************************/
PROCESS_THREAD(serial_process, ev, data)
{
	int status;
	PROCESS_BEGIN();
		
	while(1) {
    	PROCESS_YIELD();
		if (ev == serial_line_event_message) {
			LOG_INFO("\n\nData is %s\n\n", (char*)data);
			/* Populate ds_ser array from serial input*/
			status = parse_serial_schedule_input((char*)data);
			if (status) {
			LOG_ERR("!! Serial schedule error. Try sending the schedule again\n");
			continue;
			}
			dynsched_print_serial_schedules(); 
			// For updating schedule locally (Coordinator)
			dynsched_schedule_create_from_array(links_count, ds_ser);
			// For sending schedules over EBs
			dynsched_update_network_schedules(links_count, ds_ser);
		}
	}

	PROCESS_END();
}

/*************************************************************/
