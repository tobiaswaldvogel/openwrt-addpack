#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=shellinabox
PKG_VERSION:=2.14
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://shellinabox.googlecode.com/files
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

#EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/shellinabox
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Shellinabox - Web Terminal
  URL:=https://code.google.com/p/shellinabox/
  DEPENDS:=+libopenssl +libpam +PACKAGE_libpam:libpam
endef

define Package/lhellinabox/description
 Shellinabox - Web Terminal
endef

CONFIGURE_ARGS += \
	--disable-runtime-loading \
	--disable-ssl

define Package/shellinabox/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/shellinaboxd $(1)/usr/bin
endef

$(eval $(call BuildPackage,shellinabox))
