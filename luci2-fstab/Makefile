include $(TOPDIR)/rules.mk

PKG_NAME:=luci2-fstab
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/luci2-fstab
  SECTION:=luci2
  CATEGORY:=LuCI2
  TITLE:=LuCI2 fstab module
  DEPENDS:=luci2
endef

define Package/luci2-fstab/description
  LuCI2 module for mount point management
endef

define Build/Compile
endef

define Package/luci2-fstab/install
	$(INSTALL_DIR) $(1)/usr/share/rpcd/menu.d
	$(INSTALL_DATA) ./files/system.fstab.json.menu $(1)/usr/share/rpcd/menu.d/sytem.fstab.json

	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) ./files/system.fstab.json.acl $(1)/usr/share/rpcd/acl.d/system.fstab.json

	$(INSTALL_DIR) $(1)/www/luci2/template
	$(INSTALL_DATA) ./files/system.fstab.htm $(1)/www/luci2/template

	$(INSTALL_DIR) $(1)/www/luci2/view
	$(INSTALL_DATA) ./files/system.fstab.js $(1)/www/luci2/view
endef

$(eval $(call BuildPackage,luci2-fstab))
