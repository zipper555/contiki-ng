#ifndef __PROJECT_CONF_H__
#define __PROJECT_CONF_H__


/******************* Free some space from the network stack ********************/
/* USB serial takes space, free more space elsewhere */
// #define SICSLOWPAN_CONF_FRAG 0
// #define UIP_CONF_BUFFER_SIZE 160


/******************* Configure TSCH ********************/
#define TSCH_PACKET_CONF_EB_WITH_SLOTFRAME_AND_LINK 0
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 3
#define TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL 0
#define TSCH_CONF_EB_PERIOD (5 * CLOCK_SECOND)

#define DYNSCHED_TSCH_SCHEDULE_DEFAULT_LENGTH 4
#define DYNSCHED_CONF_CUSTOM_SCHEDULE 1
#define DYNSCHED_TSCH_PACKET_EB_WITH_SLOTFRAME_AND_LINK 1


/* Logging */
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_NULLNET		   	   		   LOG_LEVEL_NONE
#define TSCH_LOG_CONF_PER_SLOT                     1

#endif /* __PROJECT_CONF_H__ */
