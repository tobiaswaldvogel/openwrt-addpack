#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nss-mdns
PKG_VERSION:=0.10
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://0pointer.de/lennart/projects/nss-mdns

include $(INCLUDE_DIR)/package.mk

EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/nss-mdns
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=NSS module for mDNS
  URL:=http://0pointer.de/lennart/projects/nss-mdns
endef

define Package/nss-mdns/description
 NSS module for mDNS
endef

CONFIGURE_ARGS += \
	--enable-legacy

define Package/nss-mdns/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/.libs/libnss_mdns4_minimal.so.2 $(1)/lib
endef

$(eval $(call BuildPackage,nss-mdns))
