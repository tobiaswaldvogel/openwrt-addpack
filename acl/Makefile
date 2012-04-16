#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=acl
PKG_VERSION:=2.2.51
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).src.tar.gz
PKG_SOURCE_URL:=http://download.savannah.gnu.org/releases/acl/
PKG_BUILD_DEPENDS:=attr

include $(INCLUDE_DIR)/package.mk

EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/libacl
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Library for Manipulating POSIX Access Control Lists
  URL:=http://download.savannah.gnu.org
  DEPENDS:= +libattr
endef

define Package/libacl/description
 Library for Manipulating Filesystem Extended aclibutes
endef

define Package/acl
  SECTION:=utilities
  CATEGORY:=Utilities
  TITLE:=Commands for Manipulating POSIX Access Control Lists
  URL:=http://download.savannah.gnu.org
  DEPENDS:= +libacl
endef

define Package/acl/description
 Commands for Manipulating Filesystem Extended aclibutes
endef

define Package/libacl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libacl/.libs/libacl.so* $(1)/usr/lib
endef

define Package/acl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/chacl/.libs/chacl $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/getfacl/.libs/getfacl $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/setfacl/.libs/setfacl $(1)/usr/bin
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/acl
	$(CP) $(PKG_BUILD_DIR)/include/acl/acl.h $(TOOLCHAIN_DIR)/include/sys
	$(CP) $(PKG_BUILD_DIR)/include/acl/libacl.h $(1)/usr/include/acl
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libacl/.libs/libacl.{a,so*} $(1)/usr/lib/
endef

$(eval $(call BuildPackage,acl))
$(eval $(call BuildPackage,libacl))
