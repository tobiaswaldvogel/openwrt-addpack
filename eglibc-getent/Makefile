#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=eglibc-locale
PKG_VERSION:=$(call qstrip,$(CONFIG_GLIBC_VERSION))
PKG_REVISION:=$(call qstrip,$(CONFIG_GLIBC_REVISION))
PKG_RELEASE:=1

EGLIBC_BUILD_DIR:=$(BUILD_DIR_TOOLCHAIN)/glibc-$(PKG_VERSION)-r$(PKG_REVISION)-final

include $(INCLUDE_DIR)/package.mk

define Package/eglibc-getent
  SECTION:=util
  CATEGORY:=Utilities
  URL:=http://www.eglibc.org
  TITLE:=getent utility for eglibc
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/eglibc-getent/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(EGLIBC_BUILD_DIR)/nss/getent $(1)/usr/bin
endef

$(eval $(call BuildPackage,eglibc-getent))
