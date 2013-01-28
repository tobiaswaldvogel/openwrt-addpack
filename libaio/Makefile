# 
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libaio
PKG_VERSION:=0.3.109
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://ftp.de.debian.org/debian/pool/main/liba/$(PKG_NAME)/$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_MD5SUM:=

#PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libaio
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Library for kernel AIO suppoer
  URL:=http://http://lse.sourceforge.net/io/aio.html
endef

#CONFIGURE_ARGS += --enable-gss
#CONFIGURE_VARS += \
#	GSSGLUE_LIBS="-lkrb5 -lk5crypto -l:libcom_err.so.3 -lkeyutils -lresolv -gssapi_krb5" \
#	GSSGLUE_CFLAGS=-I$(STAGING_DIR)/usr/include/krb5/

#EXTRA_LDFLAGS := -lgssapi_krb5

define Package/libaio/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libaio.so* $(1)/usr/lib
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libaio.{a,so*} $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(SED) 's,/usr,${STAGING_DIR}/usr,g' $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libaio.pc
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libaio.pc $(1)/usr/lib/pkgconfig/libaio.pc
endef

$(eval $(call BuildPackage,libaio))
