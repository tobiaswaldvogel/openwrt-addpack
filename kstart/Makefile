#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=kstart
PKG_VERSION:=4.1
PKG_RELEASE:=1
PKG_INSTALL:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://archives.eyrie.org/software/kerberos

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/krb5

define Package/kstart
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Wrapper for aquiring and maintaing Kerberos TGT
  URL:=http://www.eyrie.org/~eagle/software/kstart/
  DEPENDS:=+krb5-libs
endef

define Package/kstart/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/k5start $(1)/usr/bin
endef

$(eval $(call BuildPackage,kstart))
