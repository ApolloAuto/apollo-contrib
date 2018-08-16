SRCDIR = .
CC = gcc
LIB_PATH = ../../lib/adv_trigger
ZYNQ_INC = ../../../include/uapi/
CFLAGS = -I$(LIB_PATH) -I$(ZYNQ_INC) -static -O2

all: adv_trigger

adv_trigger: $(SRCDIR)/adv_trigger.c
	$(CC) $(CFLAGS) -o adv_trigger -L$(LIB_PATH) $(SRCDIR)/adv_trigger.c -ladv_trigger_ctl

clean:
	rm -f adv_trigger
