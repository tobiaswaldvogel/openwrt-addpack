#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libvmime
PKG_VERSION:=0.9.2
PKG_SVN:=svn603
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)+$(PKG_SVN).tar.bz2
PKG_SOURCE_URL:=http://download.zarafa.com/community/final/7.0/7.0.6-32752/sourcecode/
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/libvmime
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libvmime
  URL:=http://www.zarafa.com
endef

define Package/libvmime/description
 libvmime
endef

define Build/Configure
        $(call Build/Configure/Default, \
		--disable-sasl \
		--disable-tls \
	)
endef

define Build/InstallDev
	$(SED) 's,/usr,${STAGING_DIR}/usr,g' $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/vmime.pc
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/libvmime/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,libvmime))
