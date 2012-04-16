include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-transmission
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-transmission
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  DEPENDS:=+transmission-daemon +transmission-web
  TITLE:=LuCI - Transmission BitTorrent
endef

define Package/luci-app-transmission/description
 LuCI - Transmission BitTorrent
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-transmission/install
	$(CP) -r ./files/root/* $(1)
endef

define Package/luci-app-transmission/postinst
#!/bin/sh
grep -q 'transmission' $${IPKG_INSTROOT}/etc/config/ucitrack 2>/dev/null
[ $$? -ne 0 ] && {
	echo "" >>$${IPKG_INSTROOT}/etc/config/ucitrack
	echo "config transmission" >>$${IPKG_INSTROOT}/etc/config/ucitrack
	echo "       option init transmission" >>$${IPKG_INSTROOT}/etc/config/ucitrack
}
endef

$(eval $(call BuildPackage,luci-app-transmission))
