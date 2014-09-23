# $Id$

include $(TOPDIR)/rules.mk

PKG_NAME:=mxml
PKG_VERSION:=2.8
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://www.msweet.org/files/project3
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/lib$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Mini-XML - Tiny XML Library
  MAINTAINER:=tobias.waldvogel@gmail.com
  URL:=http://www.msweet.org/
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static

define Build/Compile
	$(call Build/Compile/Default, libmxml.so.1.5)
	$(call Build/Compile/Default, libmxml.a)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(PKG_NAME).h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib$(PKG_NAME).so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(PKG_NAME).pc $(1)/usr/lib/pkgconfig/
endef

define Package/lib$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib$(PKG_NAME).so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,lib$(PKG_NAME)))
