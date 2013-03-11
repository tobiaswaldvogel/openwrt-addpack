include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-scst
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install
PKG_BUILD_DEPENDS:=luci

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-scst
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  DEPENDS:=+scst
  TITLE:=iSCSI target
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-scst/install
	$(CP) -r ./files/root/* $(1)
endef

$(eval $(call BuildPackage,luci-app-scst))
