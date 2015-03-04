include $(TOPDIR)/rules.mk

PKG_NAME:=luci2-realtime-stat
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/luci2-realtime-stat
  SECTION:=luci2
  CATEGORY:=LuCI2
  TITLE:=LuCI2 realtime stat module
  DEPENDS:=luci2
endef

define Package/luci2-realtime-stat/description
  LuCI2 module for realtime statistics
endef

define Build/Compile
endef

define Package/luci2-realtime-stat/install
	$(INSTALL_DIR) $(1)/usr/share/rpcd/menu.d
	$(INSTALL_DATA) ./files/usr/share/rpcd/menu.d/services.p910nd.json $(1)/usr/share/rpcd/menu.d/

	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) ./files/usr/share/rpcd/acl.d/services.p910nd.json $(1)/usr/share/rpcd/acl.d/

	$(INSTALL_DIR) $(1)/www/luci2/template
	$(INSTALL_DATA) ./files/www/luci2/template/services.p910nd.htm $(1)/www/luci2/template

	$(INSTALL_DIR) $(1)/www/luci2/view
	$(INSTALL_DATA) ./files/www/luci2/view/services.p910nd.js $(1)/www/luci2/view
endef

$(eval $(call BuildPackage,luci2-realtime-stat))
