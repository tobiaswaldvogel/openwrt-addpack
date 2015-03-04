include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-idm
PKG_VERSION=1.0
PKG_RELEASE:=1
PKG_BUILD_DEPENDS:=luci
PO2LMO:=$(STAGING_DIR_HOST)/bin/po2lmo

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-idm
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  DEPENDS:=+openldap-sasl-server +krb5-mit-server +krb5-mit-ldap +lualdap +luadigest +nss-ldap +pam-krb5
  TITLE:=Identity management for LDAP/Kerberos
endef

define Package/luci-app-idm/description
 Identity management for LDAP/Kerberos
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-idm/install
	$(INSTALL_DIR) $(1)/www/luci-static/resources/cbi
	$(CP) files/*.gif $(1)/www/luci-static/resources/cbi/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) files/idm.controller.lua $(1)/usr/lib/lua/luci/controller/idm.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/idm
	$(INSTALL_DATA) files/*.htm $(1)/usr/lib/lua/luci/view/idm
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	for transl in files/*.po; do \
		transl_dest=`basename $$$$transl .po`.lmo; \
		$(PO2LMO) $$$$transl $(1)/usr/lib/lua/luci/i18n/$$$$transl_dest; \
	done
endef

$(eval $(call BuildPackage,luci-app-idm))
