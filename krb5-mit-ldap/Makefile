include $(TOPDIR)/rules.mk

PKG_NAME:=krb5-mit-ldap
PKG_VERSION:=1.14.2
PKG_RELEASE:=2

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=NOTICE

PKG_SOURCE:=krb5-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://web.mit.edu/kerberos/dist/krb5/1.14/
PKG_MD5SUM:=2e35f0af0344d68aba99cef616d3a64f

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

MAKE_PATH:=src

define Package/krb5-mit/Default
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Kerberos
	TITLE:=MIT Kerberos 5
	URL:=http://web.mit.edu/kerberos/
endef

define Package/krb5-mit-libs
	$(call Package/krb5-mit/Default)
	DEPENDS:=+libncurses +libkeyutils +libreadline +libext2fs
	TITLE+= (Shared Libraries)
endef

define Package/krb5-mit-server
	$(call Package/krb5-mit/Default)
	DEPENDS:=+krb5-mit-libs +libpthread
	TITLE+= (Server)
endef

define Package/krb5-mit-ldap
	$(call Package/krb5-mit/Default)
	DEPENDS:=+krb5-mit-libs +libpthread +libopenldap-sasl
	TITLE+= (LDAP support)
endef

define Package/krb5-mit-client
	$(call Package/krb5-mit/Default)
	DEPENDS:=+krb5-mit-libs
	TITLE+= (Client)
endef

define Build/Prepare
	tar -xOf "$(DL_DIR)/$(PKG_SOURCE)" krb5-$(PKG_VERSION).tar.gz | \
		tar -xz -C "$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)"
	$(call Build/Patch/Default)
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/et

CONFIGURE_PATH = src

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
	--enable-dns-for-realm \
	--disable-pkinit \
	--with-system-et \
	--without-system-verto \
	--without-tcl \
	--without-libedit \
	--with-ldap \
	--with-readline \
	--with-size-optimizations \
	--localstatedir=/etc

#EXTRA_LDFLAGS += -llber -Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/krb5-mit-libs/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/krb5conf.init $(1)/etc/init.d/krb5conf
	ln -sf /var/etc/krb5.conf $(1)/etc/krb5.conf
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgss{api_krb5,rpc}.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libk5crypto.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libkadm5{clnt*,srv*}.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libkrad.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libkrb5.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libkrb5support.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libverto.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libkdb5.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/krb5/plugins/kdb
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/krb5/plugins/kdb/db2.so $(1)/usr/lib/krb5/plugins/kdb
endef

define Package/krb5-mit-ldap/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kdb5_ldap_util $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libkdb_ldap.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/krb5/plugins/kdb
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/krb5/plugins/kdb/kldap.so $(1)/usr/lib/krb5/plugins/kdb/
	$(INSTALL_DIR) $(1)/etc/openldap/schema
	$(CP) $(PKG_BUILD_DIR)/src/plugins/kdb/ldap/libkdb_ldap/kerberos.schema $(1)/etc/openldap/schema
endef

define Package/krb5-mit-server/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/krb5kdc.init $(1)/etc/init.d/krb5kdc
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kadmin.local $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kadmind $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kdb5_util $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/krb5kdc $(1)/usr/sbin
endef

define Package/krb5-mit-server/conffiles
/etc/krb5.keytab
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

$(eval $(call BuildPackage,krb5-mit-ldap))
$(eval $(call BuildPackage,krb5-mit-client))
$(eval $(call BuildPackage,krb5-mit-server))
$(eval $(call BuildPackage,krb5-mit-libs))
