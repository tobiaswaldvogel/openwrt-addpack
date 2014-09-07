#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hiawatha
PKG_VERSION:=9.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.hiawatha-webserver.org/files/

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/hiawatha
  SUBMENU:=Web Servers/Proxies
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Hiawatha Web Server
  URL:=https://www.hiawatha-webserver.org
  DEPENDS:=+libpolarssl +libxml2 +libxslt
endef

define Package/hiawatha/description
 Hiawatha Web Server
endef

define Package/hiawatha/conffiles
/etc/hiawatha
endef


CMAKE_OPTIONS += \
	-DUSE_SYSTEM_POLARSSL=on \
	-DENABLE_CACHE=off \
	-DCMAKE_INSTALL_SYSCONFDIR=/etc \
	-DPID_DIR=/var/run \
	-DLOG_DIR=/var/log/hiawatha \
	-DWEBROOT_DIR=/www \
	-DWORK_DIR=/tmp/hiawatha

define Package/hiawatha/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ssi-cgi $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/hiawatha $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/wigwam $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/cgi-wrapper $(1)/usr/sbin
endef

$(eval $(call BuildPackage,hiawatha))
