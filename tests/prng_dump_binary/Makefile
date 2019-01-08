include ../Makefile.tests_common

# some boards have not enough rom and/or ram
BOARD_BLACKLIST += nucleo-f031k6 nucleo-f042k6 nucleo-l031k6 pic32-clicker

USEMODULE += random
USEMODULE += prng_xorshift
# override PRNG if desired (see sys/random for alternatives)
# USEMODULE += prng_minstd

# fortuna requires more stack
ifneq (,$(filter prng_fortuna,$(USEMODULE)))
  CFLAGS += -DTHREAD_STACKSIZE_MAIN=2048
endif

# only include xtimer in toggel pin mode
# build with TOGGLE_PIN_MODE=1
ifeq (1, $(TOGGLE_PIN_MODE))
  USEMODULE += xtimer
  CFLAGS += -DTOGGLE_PIN_MODE
endif

include $(RIOTBASE)/Makefile.include
