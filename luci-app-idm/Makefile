include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-idm
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-idm
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  DEPENDS:=+openldap-sasl-server +krb5-mit-server +krb5-mit-ldap +lualdap +luadigest
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
	$(CP) -r ./files/root/* $(1)
endef

$(eval $(call BuildPackage,luci-app-idm))
