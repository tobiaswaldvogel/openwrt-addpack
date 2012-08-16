include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-hd-mon
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install
PKG_BUILD_DEPENDS:=+smartmontools +ssmtp

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-hd-mon
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  DEPENDS:=+smartd +mdadm +ssmtp
  TITLE:=RAID and SMART monitoring and email notification
endef

define Package/luci-app-hd-mon/description
 RAID and SMART monitoring and email notification
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-hd-mon/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) -r ./files/root/* $(1)
	$(INSTALL_BIN) ./files/hd-mon.init $(1)/etc/init.d/hd-mon
endef

define Package/luci-app-hd-mon/postinst
#!/bin/sh
rm $${IPKG_INSTROOT}/etc/rc.d/S95smartd 2>/dev/null
rm $${IPKG_INSTROOT}/etc/init.d/smartd 2>/dev/null
rm $${IPKG_INSTROOT}/etc/ssmtp/ssmtp.conf 2>/dev/null
ln -s /tmp/ssmtp.conf $${IPKG_INSTROOT}/etc/ssmtp/ssmtp.conf
ln -s /tmp/ssmtp.revaliases $${IPKG_INSTROOT}/etc/ssmtp/revaliases
ln -s ../init.d/hd-mon $${IPKG_INSTROOT}/etc/rc.d/S50hd-mon
grep -q 'hd-mon' $${IPKG_INSTROOT}/etc/config/ucitrack 2>/dev/null
[ $$? -ne 0 ] && {
	echo "" >>$${IPKG_INSTROOT}/etc/config/ucitrack
	echo "config hd-mon" >>$${IPKG_INSTROOT}/etc/config/ucitrack
	echo "       option init hd-mon" >>$${IPKG_INSTROOT}/etc/config/ucitrack
} 
endef

$(eval $(call BuildPackage,luci-app-hd-mon))
