#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lualdap
PKG_VERSION:=1.1.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://files.luaforge.net/releases/lualdap/lualdap/LuaLDAP$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/lualdap
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=LUA LDAP Library
  URL:=http://www.padl.com
endef

define Package/lualdap/description
  LUA LDAP Library
endef

TARGET_CFLAGS += -Wall -fPIC -W -Waggregate-return -Wcast-align -Wmissing-prototypes -Wnested-externs -Wshadow -Wwrite-strings -ansi -DLDAP_DEPRECATED=1

define Build/Configure
	(cd $(PKG_BUILD_DIR); \
	echo "LIBNAME=$(PKG_NAME).so" > config; \
	echo "LIB_OPTION=-L$(STAGING_DIR)/usr/lib -shared -llua" >> config; \
	echo "OPENLDAP_LIB=-lldap" >> config)
	$(call Build/Configure/Default)
endef

define Build/Compile
	$(call Build/Compile/Default, src/$(PKG_NAME).so)
endef

define Package/lualdap/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/lualdap.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,lualdap))
