#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/uclibc++.mk

PKG_NAME:=luci-status-disc
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  TITLE:=Show disc status
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/admin_status
	$(INSTALL_DATA) files/disc_status.htm $(1)/usr/lib/lua/luci/view/admin_status/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/admin_status/index
	$(INSTALL_DATA) files/disc.htm $(1)/usr/lib/lua/luci/view/admin_status/index/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
