#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lualdap
PKG_VERSION:=2.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/lualdap
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lua LDAP Library
  URL:=http://github.com/tobiaswaldvogel
  DEPENDS:=+libopenldap +liblua
endef

define Package/lualdap/description
  Lua LDAP Library
endef

TARGET_CFLAGS += -Wall -fPIC -W -Waggregate-return -Wcast-align -Wmissing-prototypes -Wnested-externs -Wshadow -Wwrite-strings -ansi

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	(cd  $(PKG_BUILD_DIR); \
		$(TARGET_CC) $(TARGET_CFLAGS) -g -c $(PKG_NAME).c; \
		$(TARGET_CC) $(TARGET_CFLAGS) -shared -Wl,-soname,lualdap.so -o $(PKG_NAME).so $(PKG_NAME).o -llua -lldap \
	)
endef

define Package/lualdap/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lualdap.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,lualdap))
