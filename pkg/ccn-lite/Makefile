PKG_NAME=ccn-lite
PKG_URL=https://github.com/cn-uofbasel/ccn-lite/
PKG_VERSION=dbf3e1a8935cfc4b6bcfa1a5baa477553ffdafec
PKG_LICENSE=ISC

.PHONY: all

export RIOT_CFLAGS = ${CFLAGS} ${INCLUDES}

CCNLITE_PATH := "../../../ccn-lite"

all: git-download
	rm -rf $(PKG_BUILDDIR) && mkdir -p $(PKG_BUILDDIR)
	rsync -a $(CCNLITE_PATH) $(dir $(PKG_BUILDDIR)) --exclude .git

	cd $(PKG_BUILDDIR)/src && cmake -DCCNL_RIOT=1 -DRIOT_CFLAGS="${RIOT_CFLAGS}" -DBUILD_TESTING=OFF . && make
	cp $(PKG_BUILDDIR)/src/lib/libccnl-riot.a ${BINDIR}/ccn-lite.a

include $(RIOTBASE)/pkg/pkg.mk
