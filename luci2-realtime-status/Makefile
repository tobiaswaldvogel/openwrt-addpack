include $(TOPDIR)/rules.mk

CAT=status
APP=realtime
PKG_NAME:=luci2-$(APP)-$(CAT)
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/luci2-$(APP)-$(CAT)
  SECTION:=luci2
  CATEGORY:=LuCI2
  TITLE:=LuCI2 $(APP) $(CAT) module
  DEPENDS:=luci2
endef

define Package/luci2-$(APP)-$(CAT)/description
  LuCI2 module for $(APP) $(CAT)
endef

define Build/Compile
endef

define Package/luci2-$(APP)-$(CAT)/install
	$(INSTALL_DIR) $(1)/usr/share/rpcd/menu.d
	$(INSTALL_DATA) ./files/$(CAT).$(APP).json.menu $(1)/usr/share/rpcd/menu.d/$(CAT).$(APP).json

	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) ./files/$(CAT).$(APP).json.acl $(1)/usr/share/rpcd/acl.d/$(CAT).$(APP).json

	$(INSTALL_DIR) $(1)/www/luci2/template
	$(INSTALL_DATA) ./files/$(CAT).$(APP).htm $(1)/www/luci2/template

	$(INSTALL_DIR) $(1)/www/luci2/view
	$(INSTALL_DATA) ./files/$(CAT).$(APP).js $(1)/www/luci2/view
endef

$(eval $(call BuildPackage,luci2-$(APP)-$(CAT)))
