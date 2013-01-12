#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gsoap2
PKG_MAJOR_VERSION=2.8
PKG_MINOR_VERSION=12
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=1

PKG_SOURCE:=gsoap_$(PKG_VERSION).zip
PKG_BUILD_DIR:=$(BUILD_DIR)/gsoap-$(PKG_MAJOR_VERSION)
PKG_SOURCE_URL:=@SF/gsoap2
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/libgsoap
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libgsoap
  URL:=http://www.cs.fsu.edu/~engelen/soap.html
  DEPENDS:=+libstdcpp
endef

define Package/libgsoap/description
 libgsoap
endef

#define Build/Configure
#        $(call Build/Configure/Default, \
#		--disable-sasl \
#		--disable-tls \
#	)
#endef

#define Build/InstallDev
#	$(SED) 's,/usr,${STAGING_DIR}/usr,g' $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/vmime.pc
#	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
#endef

define Package/libgsoap/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,libgsoap))
