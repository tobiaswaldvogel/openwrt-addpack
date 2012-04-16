#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=spcdns
PKG_VERSION:=
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME).tar.gz
PKG_SOURCE_URL:=http://www.conman.org/software/spcdns/download
#PKG_MD5SUM:=

include $(INCLUDE_DIR)/package.mk

define Package/luaspcdns
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=LUA DNS library
  URL:=http://www.conman.org/software/spcdns
  DEPENDS:=+lua
endef

define Package/luaspcdns/description
  A DNS library for LUA
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		LIBDIR="$(TARGET_LDFLAGS)" \
		CC="$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) -std=gnu99" \
		LD="$(TARGET_CROSS)ld -shared" \
		all
endef


define Package/luaspcdns/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/org
	$(INSTALL_DIR) $(1)/usr/lib/lua/org/conman
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/built/dns.so $(1)/usr/lib/lua/org/conman
endef

$(eval $(call BuildPackage,luaspcdns))
