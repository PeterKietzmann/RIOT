include ../Makefile.tests_common

# some boards have not enough rom and/or ram
BOARD_BLACKLIST += nucleo-f031k6 nucleo-f042k6 nucleo-l031k6 pic32-clicker

USEMODULE += random
USEMODULE += prng_xorshift
# override PRNG if desired (see sys/random for alternatives)
# USEMODULE += prng_minstd

include $(RIOTBASE)/Makefile.include
