#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luadigest
PKG_VERSION:=2.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/luadigest
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lua Digest Library
  DEPENDS:=+libpolarssl +liblua
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)/
endef

#TARGET_CFLAGS += -fPIC
#		MD4_SOURCE=`find ../mbedtls* -name md4.c`; \
#		$(TARGET_CC) $(TARGET_CFLAGS) -DPOLARSSL_MD4_C -c $$$$MD4_SOURCE -o md4_mbedtls.o; \

define Build/Compile
	(cd  $(PKG_BUILD_DIR); \
		$(TARGET_CC) $(TARGET_CFLAGS) -fPIC -DDIGEST=md4 -DDIGEST_LEN=16 -c luadigest.c -o md4.o; \
		$(TARGET_CC) $(TARGET_CFLAGS) -fPIC -shared -Wl,-soname,md4.so -o md4.so md4.o -llua -lmbedtls; \
		$(TARGET_CC) $(TARGET_CFLAGS) -fPIC -DDIGEST=md5 -DDIGEST_LEN=16 -c luadigest.c -o md5.o; \
		$(TARGET_CC) $(TARGET_CFLAGS) -fPIC -shared -Wl,-soname,md5.so -o md5.so md5.o -llua -lmbedtls; \
		$(TARGET_CC) $(TARGET_CFLAGS) -fPIC -DDIGEST=sha1 -DDIGEST_LEN=20 -c luadigest.c -o sha1.o; \
		$(TARGET_CC) $(TARGET_CFLAGS) -fPIC -shared -Wl,-soname,sha1.so -o sha1.so sha1.o -llua -lmbedtls; \
	)
endef

define Package/luadigest/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/md4.so $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/md5.so $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sha1.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,luadigest))
