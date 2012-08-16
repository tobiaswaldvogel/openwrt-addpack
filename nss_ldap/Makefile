#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nss_ldap
PKG_VERSION:=265
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME).tgz
PKG_SOURCE_URL:=http://www.padl.com/download

include $(INCLUDE_DIR)/package.mk

EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib
CONFIGURE_ARGS += --with-ldap-conf-file=/var/etc/ldap.conf

define Package/nss_ldap
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=NSS module for OpenLDAP
  URL:=http://www.padl.com
  DEPENDS:=+libopenldap +libcom_err
endef

define Package/nss_ldap/description
 NSS module for OpenLDAP
endef

define Build/Compile
	$(SED) 's,-o $$$$(INST_UID) -g $$$$(INST_GID),,g' $(PKG_BUILD_DIR)/Makefile
	$(MAKE) -C $(PKG_BUILD_DIR) DESTDIR="$(PKG_INSTALL_DIR)" all install
endef

define Package/nss_ldap/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib $(1)/
endef

$(eval $(call BuildPackage,nss_ldap))
