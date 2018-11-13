BOARD ?= nucleo-f411re

include ../Makefile.tests_common

USEMODULE += puf_sram
FEATURES_OPTIONAL += periph_rtc

CFLAGS += -DCLOCK_LSE=1

include $(RIOTBASE)/Makefile.include
