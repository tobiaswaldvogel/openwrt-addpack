include $(TOPDIR)/rules.mk

PKG_NAME:=krb5-mit
PKG_VERSION_MAJOR=1.12.1
PKG_VERSION_MINOR=
PKG_VERSION:=$(PKG_VERSION_MAJOR)$(PKG_VERSION_MINOR)
PKG_RELEASE:=2

PKG_SOURCE:=krb5-$(PKG_VERSION)-signed.tar
PKG_SOURCE_URL:=http://web.mit.edu/kerberos/dist/krb5/$(PKG_VERSION_MAJOR)/
PKG_MD5SUM:=

PKG_BUILD_DEPENDS:=+libopenldap-sasl
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/krb5-$(PKG_VERSION)
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

MAKE_PATH:=src
EXTRA_LDFLAGS += -llber -Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/krb5-mit/Default
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Kerberos
	TITLE:=MIT Kerberos 5
	URL:=http://web.mit.edu/kerberos/
	MAINTAINER:=W. Michael Petullo <mike@flyn.org>
endef

define Package/krb5-mit-libs
	$(call Package/krb5-mit/Default)
	DEPENDS:=+libncurses +libopenldap-sasl +libkeyutils +libreadline
	TITLE+= (Shared Libraries)
endef

define Package/krb5-mit-server
	$(call Package/krb5-mit/Default)
	DEPENDS:=+krb5-mit-libs +libpthread
	TITLE+= (Server)
endef

define Package/krb5-mit-ldap
	$(call Package/krb5-mit/Default)
	DEPENDS:=+krb5-mit-libs +libpthread
	TITLE+= (LDAP support)
endef

define Package/krb5-mit-client
	$(call Package/krb5-mit/Default)
	DEPENDS:=+krb5-mit-libs
	TITLE+= (Client)
endef

define Package/krb5-mit/description
	MIT Kerberos
endef

define Build/Prepare
	# Krb5 tarball contains signature and a second tarball
	# containing source code.
	tar -xOf "$(DL_DIR)/$(PKG_SOURCE)" krb5-$(PKG_VERSION).tar.gz | \
		tar -xz -C "$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)"
	$(call Build/Patch/Default)
endef

CONFIGURE_PATH = ./src

CONFIGURE_VARS += \
	cross_compiling=yes \
	krb5_cv_attr_constructor_destructor=yes,yes \
	krb5_cv_sys_rcdir=/tmp \
	ac_cv_func_regcomp=yes \
	ac_cv_printf_positional=yes \
	ac_cv_file__etc_environment=no \
	ac_cv_file__etc_TIMEZONE=no

CONFIGURE_ARGS += \
	--enable-thread-support \
	--without-tcl \
	--without-libedit \
	--with-readline \
	--with-ldap \
	--with-size-optimizations \
	--enable-dns-for-realm \
	--with-system-verto=no \
	--localstatedir=/etc

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include \
		$(1)/usr/include/krb5
	$(INSTALL_DIR) $(1)/usr
	$(CP) $(PKG_INSTALL_DIR)/usr/lib \
		$(1)/usr
	rm $(1)/usr/lib/libcom_err.so
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/krb5-config \
		$(1)/usr/bin
	$(SED) 's,^includedir=$$$${prefix}/include,includedir=$(STAGING_DIR)/usr/include/krb5,g' $(1)/usr/bin/krb5-config
	$(SED) 's,^libdir=$$$${exec_prefix}/lib,libdir=$(STAGING_DIR)/usr/lib,g' $(1)/usr/bin/krb5-config
	$(SED) 's,-lcom_err,-l:libcom_err.so.3,g' $(1)/usr/bin/krb5-config
endef

define Package/krb5-mit-libs/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/krb5conf $(1)/etc/init.d/krb5conf
	ln -sf /var/etc/krb5.conf $(1)/etc/krb5.conf
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/krb5
	$(INSTALL_DIR) $(1)/usr/lib/krb5/plugins
	$(INSTALL_DIR) $(1)/usr/lib/krb5/plugins/kdb
	$(INSTALL_DIR) $(1)/usr/lib/krb5/plugins/libkrb5
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/krb5/plugins/kdb/db2.so $(1)/usr/lib/krb5/plugins/kdb
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib
	rm $(1)/usr/lib/libcom_err.so
endef

define Package/krb5-mit-client/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kdestroy $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kinit $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/klist $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kpasswd $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ksu $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kvno $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kadmin $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/krb5-send-pr $(1)/usr/sbin
endef

# Removed some server-side software to reduce package size. This should be
# put in a separate package if needed.
define Package/krb5-mit-server/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/krb5kdc $(1)/etc/init.d/krb5kdc
#	$(INSTALL_DIR) $(1)/usr/bin
#	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/sclient $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kadmin.local $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kadmind $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kdb5_util $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/krb5kdc $(1)/usr/sbin
endef

define Package/krb5-server/conffiles
/etc/krb5.keytab
endef

define Package/krb5-mit-ldap/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kdb5_ldap_util $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib/krb5/plugins/kdb
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/krb5/plugins/kdb/kldap.so $(1)/usr/lib/krb5/plugins/kdb
	$(INSTALL_DIR) $(1)/etc/openldap/schema
	$(CP) $(PKG_BUILD_DIR)/src/plugins/kdb/ldap/libkdb_ldap/kerberos.schema $(1)/etc/openldap/schema
endef

$(eval $(call BuildPackage,krb5-mit-libs))
$(eval $(call BuildPackage,krb5-mit-server))
$(eval $(call BuildPackage,krb5-mit-client))
$(eval $(call BuildPackage,krb5-mit-ldap))
