CONTIKI_PROJECT = client ifttt-client
all: $(CONTIKI_PROJECT)

BOARD = orion

MODULES += os/net/app-layer/http-socket
MODULES += arch/dev/enc28j60
WITH_IP64 = 1

PLATFORMS_ONLY = zoul
BOARDS_ONLY = orion

CONTIKI = ../../../../..
include $(CONTIKI)/Makefile.include
