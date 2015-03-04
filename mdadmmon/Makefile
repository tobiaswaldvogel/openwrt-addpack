#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mdadmmon
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_BUILD_DEPENDS:=luci
PO2LMO:=$(STAGING_DIR_HOST)/bin/po2lmo

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-mdadm
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  DEPENDS:=+mdadm +luci-app-ssmtp +luci-status-disc
  TITLE:=RAID array monitoring
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-mdadm/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) files/mdadm.controller.lua $(1)/usr/lib/lua/luci/controller/mdadm.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DATA) files/mdadm.cbi.lua $(1)/usr/lib/lua/luci/model/cbi/mdadm.lua
#	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
#	for transl in files/*.po; do \
#		transl_dest=`basename $$$$transl .po`.lmo; \
#		$(PO2LMO) $$$$transl $(1)/usr/lib/lua/luci/i18n/$$$$transl_dest; \
#	done
endef

$(eval $(call BuildPackage,luci-app-mdadm))
