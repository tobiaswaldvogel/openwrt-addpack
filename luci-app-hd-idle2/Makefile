include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-hd-idle2
PKG_VERSION=1.1
PKG_RELEASE:=2

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-hd-idle2
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  DEPENDS:=+hdparm +hd-idle2
  TITLE:=HD Spindown package based on hd-idle
endef

define Package/luci-app-hd-idle2/description
 Spindown package for internal discs on NAS device based on hd-idle
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-hd-idle2/install
	$(CP) -r ./files/root/* $(1)
endef

define Package/luci-app-hd-idle2/postinst
#!/bin/sh
grep -q 'hd-idle' $${IPKG_INSTROOT}/etc/config/ucitrack 2>/dev/null
[ $$? -ne 0 ] && {
	echo "" >>$${IPKG_INSTROOT}/etc/config/ucitrack
	echo "config hd-idle" >>$${IPKG_INSTROOT}/etc/config/ucitrack
	echo "       option init hd-idle" >>$${IPKG_INSTROOT}/etc/config/ucitrack
} 
endef

$(eval $(call BuildPackage,luci-app-hd-idle2))
