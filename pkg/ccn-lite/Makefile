PKG_NAME=ccn-lite
PKG_URL=
PKG_VERSION=0474080335aac08d3afa44a439ac9def92b9fcf3
PKG_LICENSE=ISC

.PHONY: all

export RIOT_CFLAGS = ${CFLAGS} ${INCLUDES}

CCNL_PATH := ../../../ccn-lite

all:
	rm -rf $(PKG_BUILDDIR) && mkdir -p $(PKG_BUILDDIR)
	rsync -a $(CCNL_PATH) $(dir $(PKG_BUILDDIR)) --exclude .git && \
	cd $(PKG_BUILDDIR)/src && cmake -DCCNL_RIOT=1 -DRIOT_CFLAGS="${RIOT_CFLAGS}" . && make
	cp $(PKG_BUILDDIR)/src/lib/libccnl-riot.a ${BINDIR}/ccn-lite.a

include $(RIOTBASE)/pkg/pkg.mk
