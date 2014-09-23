include $(TOPDIR)/rules.mk

PKG_NAME:=luci-nas
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/luci-nas
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=1. Collections
  DEPENDS:= \
	+luci-mod-admin-full \
	+luci-theme-bootstrap +luci-theme-openwrt \
	+luci-proto-ppp +luci-lib-nixio \
	+luci-app-hdidle +luci-app-smartd +luci-app-mdadm +luci-app-idm \
	+luci-app-samba4 +luci-app-transmission \
	+zoneinfo-simple +zoneinfo-core +zoneinfo-europe +zoneinfo-northamerica \
	+vsftp-aio +wget +nfs4-server \
	+lighttpd +lighttpd-mod-cgi +lighttpd-mod-redirect
TITLE:=luci-nas
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-nas/install
	$(INSTALL_DIR) $(1)/etc/lighttpd
	$(CP) ./files/lighttpd.conf $(1)/etc/lighttpd
	$(CP) ./files/mime.conf $(1)/etc/lighttpd
endef

$(eval $(call BuildPackage,luci-nas))
