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

/* Dynamic Scheduling header file */

/********** Includes **********/

#include "contiki.h"
#include "net/mac/mac.h"
#include "net/mac/tsch/tsch-conf.h"
#include "net/mac/tsch/tsch-const.h"
#include "net/mac/tsch/tsch-types.h"
#include "net/mac/tsch/tsch-queue.h"
#include "net/mac/tsch/tsch-log.h"
#include "net/mac/tsch/tsch-packet.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/framer/framer-802154.h"


#ifndef __DYN_SCHED_H__
#define __DYN_SCHED_H__


struct network_schedules {
  uint8_t num_links;
  struct tsch_slotframe_and_links_link links[FRAME802154E_IE_MAX_LINKS];
};

struct network_schedules net_schedules_links;

/* A default network schedule where all the nodes are put in
 * RX state and only Coordinator remains in tx state */ 
extern uint16_t default_ns_arr[DYNSCHED_TSCH_SCHEDULE_DEFAULT_LENGTH][4];

void  dynsched_update_network_schedules(int num_links, uint16_t arr[FRAME802154E_IE_MAX_LINKS][4]);

void dynsched_print_network_schedules(void);

struct network_schedules dynsched_get_network_schedules (void);

void dynsched_create_ies_sf_and_link(struct ieee802154_ies *ies0);

/**
 * \brief Create a schedule based on the array input
 */
void dynsched_schedule_create_from_array
(int num_links, uint16_t arr[FRAME802154E_IE_MAX_LINKS][4]);

/**
 * \brief Create a schedule based on the IE recived from EB
 */

void dynsched_create_schedule_from_ies(struct ieee802154_ies ies0);


#endif /* __DYN_SCHED_H__ */
