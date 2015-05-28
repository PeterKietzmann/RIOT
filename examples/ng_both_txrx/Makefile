# name of your application
APPLICATION = ng_both_txrx

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DTESTSUITE
#CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

export ATRF_SPI ?= SPI_0
export ATRF_CS ?= GPIO_11
export ATRF_INT ?= GPIO_12
export ATRF_RESET ?= GPIO_13
export ATRF_SLEEP ?= GPIO_14

# added for measure 3 and 3.1
USEMODULE += vtimer
USEMODULE += ps

USEMODULE += ng_netif
USEMODULE += auto_init_ng_netif
USEMODULE += ng_ipv6_default
USEMODULE += ng_udp
USEMODULE += ng_pktdump

ifneq (,$(ATRF_SPI))
  CFLAGS += -DATRF_SPI=$(ATRF_SPI)
else
  CFLAGS += -DATRF_SPI=SPI_0        # set default
endif
ifneq (,$(ATRF_CS))
  CFLAGS += -DATRF_CS=$(ATRF_CS)
else
  CFLAGS += -DATRF_CS=GPIO_0        # set default
endif
ifneq (,$(ATRF_INT))
  CFLAGS += -DATRF_INT=$(ATRF_INT)
else
  CFLAGS += -DATRF_INT=GPIO_1        # set default
endif
ifneq (,$(ATRF_SLEEP))
  CFLAGS += -DATRF_SLEEP=$(ATRF_SLEEP)
else
  CFLAGS += -DATRF_SLEEP=GPIO_2        # set default
endif
ifneq (,$(ATRF_RESET))
  CFLAGS += -DATRF_RESET=$(ATRF_RESET)
else
  CFLAGS += -DATRF_RESET=GPIO_3        # set default
endif
ifneq (,$(ATRF_SPI_SPEED))
  CFLAGS += -DATRF_SPI_SPEED=$(ATRF_SPI_SPEED)
endif

include $(RIOTBASE)/Makefile.include
