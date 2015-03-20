include $(TOPDIR)/rules.mk

PKG_NAME:=luci2-p910nd
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/luci2-p910nd
  SECTION:=luci2
  CATEGORY:=LuCI2
  TITLE:=LuCI2 p910nd print server module
  DEPENDS:=luci2
endef

define Package/luci2-p910nd/description
  LuCI2 module for p910nd print server
endef

define Build/Compile
endef

define Package/luci2-p910nd/install
	$(INSTALL_DIR) $(1)/usr/share/rpcd/menu.d
	$(INSTALL_DATA) ./files/service.p910nd.json.menu $(1)/usr/share/rpcd/menu.d/service.p910nd.json

	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) ./files/service.p910nd.json.acl $(1)/usr/share/rpcd/acl.d/service.p910nd.json

	$(INSTALL_DIR) $(1)/www/luci2/template
	$(INSTALL_DATA) ./files/service.p910nd.htm $(1)/www/luci2/template

	$(INSTALL_DIR) $(1)/www/luci2/view
	$(INSTALL_DATA) ./files/service.p910nd.js $(1)/www/luci2/view
endef

$(eval $(call BuildPackage,luci2-p910nd))
