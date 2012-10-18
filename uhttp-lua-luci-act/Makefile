include $(TOPDIR)/rules.mk

PKG_NAME:=uhttp-lua-luci-act
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/uhttp-lua-luci-act
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=uHTTPd - Activate Lua integration
  DEPENDS:=+luci-mod-admin-core +luci-sgi-uhttpd
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/uhttp-lua-luci-act/install
	$(CP) -r ./files/root/* $(1)
endef

$(eval $(call BuildPackage,uhttp-lua-luci-act))
