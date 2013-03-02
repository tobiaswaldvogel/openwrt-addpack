# $Id$

include $(TOPDIR)/rules.mk

PKG_REV:=382
PKG_NAME:=xupnpd
PKG_RELEASE:=1
PKG_VERSION:=$(PKG_REV)

PKG_SOURCE_PROTO:=svn
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://tsdemuxer.googlecode.com/svn/trunk/xupnpd
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz

PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/xupnpd
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=xupnpd - eXtensible UPnP agent
  URL:=http://xupnpd.org
  DEPENDS:=
endef

define Package/xupnpd/conffiles
/etc/xupnpd.conf
endef

MAKE_PATH:=src
CONFIGURE_PATH:=src

#TARGET_CFLAGS += $(FPIC) -I$(STAGING_DIR)/usr/include/libevent
#TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib/libevent -Wl,-rpath-link,$(STAGING_DIR)/usr/lib

#define Build/Configure
#	$(call Build/Configure/Default, \
#		--enable-shared \
#		--enable-static)
#endef

define Package/xupnpd/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/xupnpd.conf $(1)/etc/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/xupnpd
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/xupnpd/* $(1)//usr/lib/xupnpd/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/xupnpd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/xupnpd.init $(1)/etc/init.d/xupnpd
endef

$(eval $(call BuildPackage,xupnpd))
