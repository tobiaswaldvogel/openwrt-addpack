include $(TOPDIR)/rules.mk

PKG_NAME:=luci-sgi-fcgi
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/luci-sgi-fcgi
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=7. Server Interfaces
  DEPENDS:=+luafcgid
  TITLE:=FastCGI Gateway behind existing Webserver
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-sgi-fcgi/install
	$(CP) -r ./files/root/* $(1)
endef

$(eval $(call BuildPackage,luci-sgi-fcgi))
