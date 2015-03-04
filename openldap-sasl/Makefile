#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openldap-sasl
PKG_VERSION:=2.4.40
PKG_RELEASE:=1

PKG_SOURCE:=openldap-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=ftp://ftp.openldap.org/pub/OpenLDAP/openldap-release/ \
	ftp://sunsite.cnlab-switch.ch/mirror/OpenLDAP/openldap-release/ \
	ftp://ftp.nl.uu.net/pub/unix/db/openldap/openldap-release/ \
	ftp://ftp.plig.org/pub/OpenLDAP/openldap-release/
PKG_MD5SUM:=

PKG_FIXUP:=autoreconf

BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/openldap-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/openldap-sasl/Default
  TITLE:=LDAP implementation (with SASL)
  URL:=http://www.openldap.org/
  MAINTAINER:=W. Michael Petullo <mike@flyn.org>
endef

define Package/libopenldap-sasl
  $(call Package/openldap-sasl/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libsasl2-krb5 +libpthread
  TITLE+= (libraries)
endef

define Package/libopenldap-sasl/conffiles
/etc/openldap/ldap.conf
/etc/openldap
endef

define Package/openldap-sasl-utils
  $(call Package/openldap-sasl/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libopenldap-sasl
  TITLE+= (utilities)
endef

define Package/openldap-sasl-server
  $(call Package/openldap-sasl/Default)
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libopenldap-sasl +libuuid
  TITLE+= (server)
endef

define Package/openldap-sasl-server/conffiles
/etc/openldap/slapd.conf
/etc/openldap/slapd.conf.template
/etc/openldap/data
endef

TARGET_CFLAGS += $(FPIC) -lpthread

CONFIGURE_ARGS += \
	--enable-shared \
	--disable-static \
	--disable-debug \
	--enable-dynamic \
	--enable-syslog \
	--enable-local \
	--disable-slurpd \
	--enable-spasswd \
	--without-fetch \
	--with-tls \
	--without-threads \
	--with-yielding_select="yes" \
	--with-cyrus-sasl \
	--disable-bdb \
	--disable-hdb \
	--disable-monitor \
	--disable-relay \
	--enable-overlays \
	--enable-syncprov

MAKE_VARS += HOSTCC="$(HOSTCC)" DESTDIR="$(PKG_INSTALL_DIR)"

define Build/Compile
	$(call Build/Compile/Default,depend all install)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/{lber,ldap}*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{lber,ldap}*.so* $(1)/usr/lib/
endef

define Package/libopenldap-sasl/install
	$(INSTALL_DIR) $(1)/etc/openldap
	$(LN) /var/etc/ldap.conf $(1)/etc/openldap/ldap.conf
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{lber,ldap}*.so.* $(1)/usr/lib/
endef

define Package/openldap-sasl-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ldap* $(1)/usr/bin/
endef

define Package/openldap-sasl-server/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ldap.init $(1)/etc/init.d/ldap
	$(INSTALL_DIR) $(1)/etc/openldap/schema
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/openldap/schema/* $(1)/etc/openldap/schema/
	$(INSTALL_CONF) ./files/slapd.conf.template  $(1)/etc/openldap/
	$(INSTALL_DIR) $(1)/usr/sbin
	# XXX: OpenLDAP installs slapd into libexecdir, not sbindir:
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/slapd $(1)/usr/sbin/
	( cd $(1)/usr/sbin; \
		find $(PKG_INSTALL_DIR)/usr/sbin -type l -name "slap*" | \
		xargs -n 1 basename | \
		xargs -I {} $(LN) slapd {} )
endef

$(eval $(call BuildPackage,libopenldap-sasl))
$(eval $(call BuildPackage,openldap-sasl-utils))
$(eval $(call BuildPackage,openldap-sasl-server))
