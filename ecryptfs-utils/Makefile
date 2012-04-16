# 
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ecryptfs-utils
PKG_VERSION:=96
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_SOURCE_URL:=https://launchpad.net/ecryptfs/trunk/$(PKG_VERSION)/+download
PKG_MD5SUM:=

PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ecryptfs-utils
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=eCryptfs utilities
  URL:=http://ecryptfs-utils.sourceforge.net/
endef

define Package/ecryptfs-utils/description
  eCryptfs utilities
endef


define Build/Configure
	$(call Build/Configure/Default, \
		--disable-pywrap \
		--disable-pam \
		--disable-gui \
		--disable-nss \
		--disable-pkcs11-helper \
		--disable-tspi \
	)
endef


#TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/tirpc

define Package/ecryptfs-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
#	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
#	$(INSTALL_DIR) $(1)/etc
#	$(CP) ./files/netconfig $(1)/etc/
#	$(INSTALL_DIR) $(1)/etc/init.d
#	$(INSTALL_BIN) ./files/ecryptfs-utils.init $(1)/etc/init.d/ecryptfs-utils
endef

$(eval $(call BuildPackage,ecryptfs-utils))
