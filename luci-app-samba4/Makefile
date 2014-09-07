include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-samba4
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install
PKG_BUILD_DEPENDS:=+luci

APP_DIR:=$(BUILD_DIR)/luci/applications/luci-samba/luasrc
LUA_SRC_DIR:=/usr/lib/lua/luci

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-samba4
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  DEPENDS:=+samba4-server
  TITLE:=Network Shares - Samba SMB/CIFS module (samba4)
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-samba4/install
	$(INSTALL_DIR) $(1)$(LUA_SRC_DIR)/controller
	$(CP) $(APP_DIR)/controller/samba.lua $(1)$(LUA_SRC_DIR)/controller
	$(INSTALL_DIR) $(1)$(LUA_SRC_DIR)/model/cbi
	$(CP) $(APP_DIR)/model/cbi/samba.lua $(1)$(LUA_SRC_DIR)/model/cbi
endef

$(eval $(call BuildPackage,luci-app-samba4))
