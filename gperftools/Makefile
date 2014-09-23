# 
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gperftools
PKG_VERSION:=2.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://gperftools.googlecode.com/files
PKG_MD5SUM:=
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libtcmalloc
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Google tcmalloc
  URL:=https://code.google.com/p/gperftools/downloads
  DEPENDS:=+libstdcpp
endef

CONFIGURE_ARGS += --enable-minimal

define Package/libtcmalloc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtcmalloc_minimal.so* $(1)/usr/lib
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtcmalloc_minimal.{a,so*} $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
endef

$(eval $(call BuildPackage,libtcmalloc))
