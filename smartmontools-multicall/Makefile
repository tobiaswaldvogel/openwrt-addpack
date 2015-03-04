#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/uclibc++.mk

PKG_NAME:=smartmontools-multicall
PKG_VERSION:=6.3
PKG_RELEASE:=1
PKG_INSTALL:=1

PKG_SOURCE:=smartmontools-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/smartmontools
PKG_MD5SUM:=2ea0c62206e110192a97b59291b17f54
PKG_MAINTAINER:=Maxim Storchak <m.storchak@gmail.com>
PKG_LICENSE:=GPL-2.0+

PKG_BUILD_DEPENDS:=luci
PO2LMO:=$(STAGING_DIR_HOST)/bin/po2lmo

BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/smartmontools-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/smartmonmulti
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libstdcpp
  TITLE:=S.M.A.R.T Monitoring (Multicall Daemon + Tool)
  URL:=http://smartmontools.sourceforge.net/
endef

define Package/luci-app-smartd
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  DEPENDS:=+smartmonmulti +luci-app-ssmtp +luci-status-disc
  TITLE:=S.M.A.R.T. monitoring
endef

define Build/Configure
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		./autogen.sh \
	);
	$(call Build/Configure/Default)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		BUILD_INFO='"(localbuild)"' \
		LD="$(TARGET_CXX)"
endef

#Adapt for sendmail
define Package/smartmonmulti/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/smartmontools $(1)/usr/sbin/
	$(LN) smartmontools $(1)/usr/sbin/smartctl
	$(LN) smartmontools $(1)/usr/sbin/smartd
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/etc/smartd_warning.sh $(1)/etc/
	$(SED) 's,os_mailer="mail",os_mailer="sendmail",g' $(1)/etc/smartd_warning.sh
	$(SED) 's,fullmessage=`,fullmessage=`echo Subject: $$$${SMARTD_SUBJECT},g' $(1)/etc/smartd_warning.sh
	$(SED) 's,-s .$$$$SMARTD_SUBJECT..,,g' $(1)/etc/smartd_warning.sh
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/smartd.init $(1)/etc/init.d/smartd
endef

define Package/luci-app-smartd/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) files/smartd.controller.lua $(1)/usr/lib/lua/luci/controller/smartd.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DATA) files/smartd.cbi.lua $(1)/usr/lib/lua/luci/model/cbi/smartd.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	for transl in files/*.po; do \
		transl_dest=`basename $$$$transl .po`.lmo; \
		$(PO2LMO) $$$$transl $(1)/usr/lib/lua/luci/i18n/$$$$transl_dest; \
	done
endef

$(eval $(call BuildPackage,smartmonmulti))
$(eval $(call BuildPackage,luci-app-smartd))
