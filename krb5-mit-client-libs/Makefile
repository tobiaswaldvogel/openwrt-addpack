include $(TOPDIR)/rules.mk

PKG_NAME:=krb5-mit-client-libs
PKG_VERSION_MAJOR=1.12.1
PKG_VERSION_MINOR=
PKG_VERSION:=$(PKG_VERSION_MAJOR)$(PKG_VERSION_MINOR)
PKG_RELEASE:=1

PKG_SOURCE:=krb5-$(PKG_VERSION)-signed.tar
PKG_SOURCE_URL:=http://web.mit.edu/kerberos/dist/krb5/$(PKG_VERSION_MAJOR)/
PKG_MD5SUM:=

PKG_BUILD_PARALLEL:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/krb5-$(PKG_VERSION)
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

MAKE_PATH:=src

define Package/krb5-mit/Default
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Kerberos
	TITLE:=MIT Kerberos 5
	URL:=http://web.mit.edu/kerberos/
	MAINTAINER:=W. Michael Petullo <mike@flyn.org>
endef

define Package/krb5-mit-client-libs
	$(call Package/krb5-mit/Default)
	DEPENDS:=+libncurses +libkeyutils +libreadline
	TITLE+= (Initial shared Libraries)
endef

# Krb5 tarball contains signature and a second tarball
# containing source code.
define Build/Prepare
	tar -xOf "$(DL_DIR)/$(PKG_SOURCE)" krb5-$(PKG_VERSION).tar.gz | \
		tar -xz -C "$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)"
	$(call Build/Patch/Default)
endef

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
	--without-tcl \
	--without-libedit \
	--without-ldap \
	--with-readline \
	--with-size-optimizations \
	--with-system-verto=no \
	--localstatedir=/etc

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include $(1)/usr/include/krb5
	$(INSTALL_DIR) $(1)/usr
	$(CP) $(PKG_INSTALL_DIR)/usr/lib $(1)/usr
	rm $(1)/usr/lib/libcom_err.so
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/krb5-config $(1)/usr/bin
	$(SED) 's,^includedir=$$$${prefix}/include,includedir=$(STAGING_DIR)/usr/include/krb5,g' $(1)/usr/bin/krb5-config
	$(SED) 's,^libdir=$$$${exec_prefix}/lib,libdir=$(STAGING_DIR)/usr/lib,g' $(1)/usr/bin/krb5-config
	$(SED) 's,-lcom_err,-l:libcom_err.so.3,g' $(1)/usr/bin/krb5-config
endef

define Package/krb5-mit-client-libs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcom_err.so.* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgssapi_krb5.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libk5crypto.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libkrb5.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libkrb5support.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,krb5-mit-client-libs))