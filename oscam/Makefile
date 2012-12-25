#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id$

include $(TOPDIR)/rules.mk

PKG_NAME:=oscam
PKG_REV:=7946
PKG_VER:=2012-12-03
PKG_VERSION:=$($PKG_VER)-$(PKG_REV)
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=svn
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://www.streamboard.tv/svn/oscam/trunk
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.bz2
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/oscam
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Open Source Conditional Access Modul
  URL:=http://www.streamboard.tv/oscam/wiki/BuildingOscam
endef

define Package/oscam/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/Distribution/oscam-$(TARGET) $(1)/usr/sbin/oscam
endef

$(eval $(call BuildPackage,oscam))
