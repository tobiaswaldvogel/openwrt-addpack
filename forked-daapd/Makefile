# $Id$

include $(TOPDIR)/rules.mk

PKG_NAME:=forked-daapd
PKG_VERSION:=0.19
PKG_RELEASE:=4
PKG_MD5SUM:=53e6f1d6683e3275e7c7698ec923a97e

PKG_SOURCE_URL:=http://alioth.debian.org/~jblache/forked-daapd \
		http://www.gyfgafguf.dk/openwrt/mirror
PKG_SOURCE:=forked-daapd-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_FIXUP:=libtool
PKG_INSTALL:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/forked-daapd-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=libgpg-error libgcrypt libgdbm zlib libexpat libunistring \
	libevent libdaemon libantlr3c confuse glib2 alsa-lib libffmpeg-full \
	mxml libavl avahi-daemon libavahi-client sqlite3-cli

include $(INCLUDE_DIR)/package.mk

define Package/forked-daapd
SECTION:=sound
CATEGORY:=Sound
TITLE:=Improved iTunes server. Support for Apple Remote. Includes patch for iTunes 10.5 timeouts.
MAINTAINER:=Espen Jürgensen <espenjurgensen+openwrt@gmail.com>
URL:=http://www.jblache.org/projects/daapd/
DEPENDS:=+libgpg-error +libgcrypt +libgdbm +zlib +libexpat +libunistring \
	+libevent +libdaemon +libantlr3c +confuse +glib2 +alsa-lib +libffmpeg-full \
	+mxml +libavl +avahi-daemon +libavahi-client +sqlite3-cli
endef

define Package/forked-daapd/conffiles
/etc/forked-daapd.conf
endef

TARGET_CFLAGS += $(FPIC)
TARGET_LDFLAGS += -Wl,-rpath-link,$(STAGING_DIR)/usr/lib

define Build/Configure
	$(call Build/Configure/Default, \
		--enable-shared \
		--enable-static)
endef

define Package/forked-daapd/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/forked-daapd.conf $(1)/etc/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/forked-daapd
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/forked-daapd/* $(1)//usr/lib/forked-daapd/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/forked-daapd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/forked-daapd.init $(1)/etc/init.d/forked-daapd
endef

$(eval $(call BuildPackage,forked-daapd))
