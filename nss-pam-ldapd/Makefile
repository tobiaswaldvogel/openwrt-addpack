#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nss-pam-ldapd
PKG_VERSION:=0.8.10
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://arthurdejong.org/nss-pam-ldapd

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

#EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib
#CONFIGURE_ARGS += --with-ldap-conf-file=/var/etc/ldap.conf
EXTRA_CFLAGS += -I$(STAGING_DIR)/usr/include/krb5/

define Package/nss-pam-ldapd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Daemon for nns-ldap and pam-ldap module
  URL:=//http://arthurdejong.org/nss-pam-ldapd/
  DEPENDS:=+libopenldap +krb5-libs +libpam
endef

define Package/libnss_ldap
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=NSS module for LDAP
  URL:=//http://arthurdejong.org/nss-pam-ldapd/
  DEPENDS:=+nss-pam-ldapd
endef

define Package/libpam_ldap
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=PAM modules for LDAD
  URL:=//http://arthurdejong.org/nss-pam-ldapd/
  DEPENDS:=+nss-pam-ldapd
endef

define Package/nss-pam-ldapd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/nslcd $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/nslcd.init $(1)/etc/init.d/nslcd
endef

define Package/libnss_ldap/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnss_ldap.so.2 $(1)/lib
endef

define Package/libpam_ldap/install
	$(INSTALL_DIR) $(1)/lib/security
	$(CP) $(PKG_INSTALL_DIR)/lib/security/pam_ldap.so $(1)/lib/security
endef

$(eval $(call BuildPackage,nss-pam-ldapd))
$(eval $(call BuildPackage,libnss_ldap))
$(eval $(call BuildPackage,libpam_ldap))
