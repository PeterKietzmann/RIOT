include ../Makefile.tests_common

USEMODULE += xtimer

# this test uses 1000 timers by default. for boards that boards don't have
# enough memory, reduce that to 100 or 20, unless NUMOF_TIMERS has been overridden.
LOW_MEMORY_BOARDS += \
  airfy-beacon \
  arduino-mega2560 \
  atmega1284p \
  b-l072z-lrwan1 \
  blackpill \
  blackpill-128kib \
  bluepill \
  bluepill-128kib \
  calliope-mini \
  cc1352-launchpad \
  cc2650-launchpad \
  cc2650stk \
  chronos \
  derfmega128 \
  hifive1 \
  hifive1b \
  i-nucleo-lrwan1 \
  lsn50 \
  maple-mini \
  mega-xplained \
  microbit \
  microduino-corerf \
  msb-430 \
  msb-430h \
  nrf51dongle \
  nrf6310 \
  nucleo-f030r8 \
  nucleo-f042k6 \
  nucleo-f070rb \
  nucleo-f072rb \
  nucleo-f103rb \
  nucleo-f303k8 \
  nucleo-f334r8 \
  nucleo-l031k6 \
  nucleo-l053r8 \
  nucleo-l073rz \
  opencm904 \
  saml10-xpro \
  saml11-xpro \
  spark-core \
  stm32f0discovery \
  stm32l0538-disco \
  telosb \
  waspmote-pro \
  wsn430-v1_3b \
  wsn430-v1_4 \
  yunjia-nrf51822 \
  z1 \
  #

SUPER_LOW_MEMORY_BOARDS += \
  arduino-duemilanove \
  arduino-leonardo \
  arduino-nano \
  arduino-uno \
  atmega328p \
  nucleo-f031k6 \
  stm32f030f4-demo \
  #

ifneq (, $(filter $(BOARD), $(LOW_MEMORY_BOARDS)))
  NUMOF_TIMERS ?= 100
endif

ifneq (, $(filter $(BOARD), $(SUPER_LOW_MEMORY_BOARDS)))
  NUMOF_TIMERS ?= 16
endif

NUMOF_TIMERS ?= 1000

CFLAGS += -DNUMOF_TIMERS=$(NUMOF_TIMERS)

include $(RIOTBASE)/Makefile.include
