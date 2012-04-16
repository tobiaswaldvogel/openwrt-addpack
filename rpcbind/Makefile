# 
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rpcbind
PKG_VERSION:=0.2.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/rpcbind
PKG_MD5SUM:=

PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/rpcbind
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Converts RPC program numbers into universal addresses
  URL:=http://rpcbind.sourceforge.net/
endef

define Package/rpcbind/description
  Converts RPC program numbers into universal addresses
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/tirpc

define Package/rpcbind/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc
	$(CP) ./files/netconfig $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/rpcbind.init $(1)/etc/init.d/rpcbind
endef

$(eval $(call BuildPackage,rpcbind))
