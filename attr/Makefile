#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=attr
PKG_VERSION:=2.4.47
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).src.tar.gz
PKG_SOURCE_URL:=http://download.savannah.gnu.org/releases/attr/

include $(INCLUDE_DIR)/package.mk

EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/libattr
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Library for Manipulating Filesystem Extended Attributes
  URL:=http://download.savannah.gnu.org
endef

define Package/libattr/description
 Library for Manipulating Filesystem Extended Attributes
endef

CONFIGURE_ARGS += --enable-gettext=no 

define Package/attr
  SECTION:=utilities
  CATEGORY:=Utilities
  TITLE:=Commands for Manipulating Filesystem Extended Attributes
  URL:=http://download.savannah.gnu.org
  DEPENDS:= +libattr
endef

define Package/attr/description
 Commands for Manipulating Filesystem Extended Attributes
endef

define Package/libattr/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libattr/.libs/libattr.so* $(1)/usr/lib
endef

define Package/attr/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/attr/.libs/attr $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/getfattr/.libs/getfattr $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/setfattr/.libs/setfattr $(1)/usr/bin
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/attr
	$(CP) $(PKG_BUILD_DIR)/include/attr/xattr.h $(1)/usr/include/attr
	$(CP) $(PKG_BUILD_DIR)/include/attr/error_context.h $(1)/usr/include/attr
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libattr/.libs/libattr.{a,so*} $(1)/usr/lib/
endef

$(eval $(call BuildPackage,attr))
$(eval $(call BuildPackage,libattr))
