#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pam-krb5
PKG_VERSION:=4.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://archives.eyrie.org/software/kerberos

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

EXTRA_CFLAGS += -I$(STAGING_DIR)/usr/include/krb5

define Package/pam-krb5
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=PAM module for Kerbers 5
  DEPENDS:= +krb5-mit-libs +libpam
  URL:=http://http://www.eyrie.org/~eagle/software/pam-krb5/
endef

define Package/pam-krb5/description
 PAM module for Kerbers 5
endef

EXTRA_CFLAGS=-I$(STAGING_DIR)/usr/include/krb5/

define Package/pam-krb5/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/lib/security
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/lib32/security/pam_krb5.so $(1)/lib/security
	$(INSTALL_DIR) $(1)/etc/pam.d
	$(CP) files/* $(1)/etc/pam.d/
endef

$(eval $(call BuildPackage,pam-krb5))
