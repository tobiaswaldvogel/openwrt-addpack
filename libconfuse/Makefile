# $Id$

include $(TOPDIR)/rules.mk

PKG_NAME:=confuse
PKG_VERSION:=2.7
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://download.savannah.gnu.org/releases/$(PKG_NAME)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/lib$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Confuse - configuration file parser library
  MAINTAINER:=tobias.waldvogel@gmail.com
  URL:=http://www.nongnu.org/confuse/
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/$(PKG_NAME).h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib$(PKG_NAME).{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/lib$(PKG_NAME).pc $(1)/usr/lib/pkgconfig/
endef

define Package/lib$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib$(PKG_NAME).so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,lib$(PKG_NAME)))
