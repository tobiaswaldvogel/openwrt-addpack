#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gptfdisk
PKG_VERSION:=0.8.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/gptfdisk
#PKG_MD5SUM:=c7ac6afdf7730ac8387a8e87198d4491

include $(INCLUDE_DIR)/package.mk

define Package/gptfdisk
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=disc
  TITLE:=Partition table manipulation utility with GPT support
  URL:=http://www.rodsbooks.com/gdisk
endef

define Package/gptfdisk/description
 Partition table manipulation utility with GPT support 
endef

define Package/gptfdisk/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gdisk $(1)/sbin/
endef

$(eval $(call BuildPackage,gptfdisk))
