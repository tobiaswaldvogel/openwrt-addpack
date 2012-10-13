include $(TOPDIR)/rules.mk

PKG_NAME:=luafcgid
PKG_VERSION:=1
PKG_REV:=745de1bb751c623453c42152cc3fcc36896bab08

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=git://github.com/STPeters/luafcgid.git
PKG_SOURCE_SUBDIR:=luafcgid-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_PROTO:=git

include $(INCLUDE_DIR)/package.mk

define Package/luafcgid
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lua FastCGI daemon
  DEPENDS:=+fcgi +liblua
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/$(PKG_NAME) \
		$(PKG_BUILD_DIR)/main.c \
		$(PKG_BUILD_DIR)/config.c \
		$(PKG_BUILD_DIR)/pool.c \
		$(PKG_BUILD_DIR)/request.c \
		$(PKG_BUILD_DIR)/parser.c \
		-lm -lpthread -lfcgi -llua -ldl
endef


define Package/luafcgid/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/luafcgid $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
endef

$(eval $(call BuildPackage,luafcgid))
