#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=zarafa
PKG_VERSION:=7.0.6
PKG_RELEASE:=1

PKG_SOURCE:=zcp-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://download.zarafa.com/community/final/7.0/7.0.6-32752/sourcecode

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/zarafa
  SECTION:=net
  CATEGORY:=Mail
  TITLE:=Zarafa Mail Server
  URL:=http://www.zarafa.com
endef

define Package/zarafa/description
 Zarafa Mail Server
endef

define Build/Configure
        $(call Build/Configure/Default, \
		--with-php-config=$(STAGING_DIR)/usr/bin/php-config \
		--with-krb5-config=$(STAGING_DIR)/usr/bin/krb5-config \
		--with-vmime-prefix=$(STAGING_DIR)/usr/include \
		--with-ical-prefix=$(STAGING_DIR)/usr/include \
	)
endef

define Package/zarafa/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/zarafa.so $(1)/lib
	ln -sf zarafa.so $(1)/lib/libzarafa.so.2
	$(INSTALL_DIR) $(1)/etc
	$(CP) ./files/nsswitch.conf $(1)/etc
	$(INSTALL_DIR) $(1)/lib/upgrade/keep.d
	echo /etc/nsswitch.conf >$(1)/lib/upgrade/keep.d/zarafa
endef

$(eval $(call BuildPackage,zarafa))
