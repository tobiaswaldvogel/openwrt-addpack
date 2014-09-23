#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vsftpd-aio
PKG_VERSION:=3.0.2
PKG_RELEASE:=4

PKG_SOURCE:=vsftpd-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://security.appspot.com/downloads/
PKG_MD5SUM:=8b00c749719089401315bd3c44dddbb2
PKG_LICENSE:=GPLv2

BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/vsftpd-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/vsftpd-aio
  SUBMENU:=File Transfer
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A fast and secure FTP server (with aio support)
  URL:=http://vsftpd.beasts.org/
  MAINTAINER:=Cezary Jackiewicz <cezary@eko.one.pl>
  DEPENDS=+PACKAGE_libpam:libpam +libaio +libopenssl
endef

define Package/vsftpd-aio/conffiles
/etc/vsftpd.conf
endef

EXTRA_LDFLAGS:= -lcrypt -lcrypto -lssl -laio

ifdef CONFIG_PACKAGE_libpam
  EXTRA_LDFLAGS += -lpam
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(EXTRA_LDFLAGS)" \
		vsftpd
endef

define Package/vsftpd-aio/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vsftpd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) ./files/vsftpd.conf $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/vsftpd.init $(1)/etc/init.d/vsftpd
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/pam.d
	$(LN) other $(1)/etc/pam.d/ftp
endef

$(eval $(call BuildPackage,vsftpd-aio))
