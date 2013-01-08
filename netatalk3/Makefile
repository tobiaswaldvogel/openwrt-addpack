#
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=netatalk3
PKG_VERSION:=3.0.1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/netatalk-$(PKG_VERSION)
PKG_SOURCE:=netatalk-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/netatalk
PKG_MD5SUM:=b4f5c932b2ca99f5292bd6b6d69b3ebc

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/netatalk3
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Filesystem
  DEPENDS:=+libdb47 +libgcrypt +libopenssl $(LIBRPC_DEPENDS) +libacl +PACKAGE_libpam:libpam +PACKAGE_libavahi-client:libavahi-client libevent2 +libwrap
  TITLE:=netatalk $(PKG_VERSION)
  URL:=http://netatalk.sourceforge.net
  MAINTAINER:=Tobias Waldvogel <tobias.waldvogel@gmail.com>
endef

define Package/netatalk3/decription
  Netatalk is a freely-available Open Source AFP fileserver.
  It also provides a kernel level implementation of the AppleTalk
  Protocol Suite.
endef

define Package/netatalk3/conffiles
/etc/netatalk/afp.conf
endef

TARGET_CFLAGS += $(LIBRPC_INCLUDE) -std=gnu99 -Wl,-rpath-link=$(STAGING_DIR)/usr/lib
TARGET_LDFLAGS += $(LIBRPC)

CONFIGURE_ARGS += \
	--disable-afs \
	--sysconfdir=/etc/netatalk \
	--enable-hfs \
	--disable-debugging \
	--disable-debug \
	--disable-shell-check \
	--disable-a2boot \
	--disable-cups \
	--enable-tcp-wrappers \
	--with-cnid-default-backend=dbd \
	--with-bdb="$(STAGING_DIR)/usr/" \
	--with-libgcrypt-dir="$(STAGING_DIR)/usr" \
	--with-ssl-dir="$(STAGING_DIR)/usr" \
	--with-libevent-header="$(STAGING_DIR)/usr/include" \
	--with-libevent-lib="$(STAGING_DIR)/usr/lib" \
	--with-uams-path="/usr/lib/uams" \
	--disable-admin-group \
	--disable-srvloc \
	--without-shadow \
	--without-ldap

ifdef CONFIG_PACKAGE_avahi-daemon
	CONFIGURE_ARGS += --enable-zeroconf
else
	CONFIGURE_ARGS += --disable-zeroconf
endif

ifdef CONFIG_PACKAGE_libpam
	CONFIGURE_ARGS += --with-pam=$(STAGING_DIR)/
else
	CONFIGURE_ARGS += --without-pam
endif

define Package/netatalk3/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib/uams
	$(INSTALL_DIR) $(1)/etc/netatalk
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/afppasswd/.libs/afppasswd $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/etc/netatalk/.libs/netatalk $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/etc/afpd/.libs/afpd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/etc/cnid_dbd/.libs/cnid_dbd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/etc/cnid_dbd/.libs/cnid_metad $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/libatalk/.libs/libatalk.so* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/etc/uams/.libs/*.so $(1)/usr/lib/uams/
	ln -sf uams_dhx_pam.so  $(1)/usr/lib/uams/uams_dhx.so
	ln -sf uams_dhx2_pam.so $(1)/usr/lib/uams/uams_dhx2.so
	$(INSTALL_CONF) ./files/afp.conf $(1)/etc/netatalk/
	$(INSTALL_BIN) ./files/afpd.init $(1)/etc/init.d/afpd
endef

$(eval $(call BuildPackage,netatalk3))
