#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=p7zip
PKG_VERSION:=4.65
PKG_RELEASE:=1

http://sourceforge.net/projects/p7zip/files/p7zip/9.20.1/p7zip_9.20.1_src_all.tar.bz2/download

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION)_src_all.tar.bz2
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)_$(PKG_VERSION)
PKG_SOURCE_URL:=@SF/$(PKG_NAME)/$(PKG_NAME)/$(PKG_VERSION)
PKG_MD5SUM:=

include $(INCLUDE_DIR)/package.mk

#define Build/Configure
#	$(CP) $(PKG_BUILD_DIR)/makefile.linux_cross_arm $(PKG_BUILD_DIR)/makefile.machine
#endef

define Package/p7zip
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=p7zip archiver
  URL:=http://http://www.7-zip.org
endef

define Package/p7zip/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/zip $(1)/usr/bin/
endef

$(eval $(call BuildPackage,p7zip))
