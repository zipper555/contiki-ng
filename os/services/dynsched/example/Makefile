CONTIKI_PROJECT = dynamic-scheduling dyn_sched
all: $(CONTIKI_PROJECT)

CONTIKI = ../../../..

# MAKE_MAC = MAKE_MAC_NULLMAC
MAKE_MAC = MAKE_MAC_TSCH
MAKE_NET = MAKE_NET_NULLNET

MODULES += os/services/shell  
# MODULES += os/services/rpl-border-router
MODULES += os/services/dynsched/infra

include $(CONTIKI)/Makefile.include
