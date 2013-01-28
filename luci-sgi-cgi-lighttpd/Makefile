include $(TOPDIR)/rules.mk

PKG_NAME:=luci-sgi-cgi-lighttpd
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/luci-sgi-cgi-lighttpd
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=LightTPD - Activate Lua integration
  DEPENDS:=+luci-mod-admin-core +luci-sgi-cgi +lighttpd +lighttpd-mod-cgi +lighttpd-mod-redirect
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-sgi-cgi-lighttpd/install
	$(CP) -r ./files/root/* $(1)
endef

$(eval $(call BuildPackage,luci-sgi-cgi-lighttpd))
