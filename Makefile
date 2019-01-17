obj-m += aioserial.o
CC              := gcc
KVERSION        := $(shell uname -r)
KDIR            := /lib/modules/$(KVERSION)/build

aioserial-objs := \
	8250_acces.o


all:
	$(MAKE) CC=$(CC) -C $(KDIR) M=$(PWD) modules

clean: 
	$(MAKE) CC=$(CC) -C $(KDIR) M=$(PWD) clean

install:
	$(MAKE) CC=$(CC) -C $(KDIR) M=$(PWD) modules_install
