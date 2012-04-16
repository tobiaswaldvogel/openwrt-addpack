#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pam_ldap
PKG_VERSION:=186
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME).tgz
PKG_SOURCE_URL:=http://www.padl.com/download

include $(INCLUDE_DIR)/package.mk

EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/pam_ldap
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=PAM module for OpenLDAP
  DEPENDS:= +libpam
  URL:=http://www.padl.com
endef

define Package/pam_ldap/description
 PAM module for OpenLDAP
endef

define Package/pam_ldap/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/lib/security
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pam_ldap.so $(1)/lib/security
endef

$(eval $(call BuildPackage,pam_ldap))
