include $(TOPDIR)/rules.mk

PKG_NAME:=ca-certs-godaddy
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/ca-certs-godaddy 
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Root CA certificates
  TITLE:=Root CA certificates for GoDaddy CA
endef

define Package/ca-certs-godaddy/description
  Root CA certificates for GoDaddy CA
endef

define Build/Compile
endef

define Package/ca-certs-godaddy/install
	$(INSTALL_DIR) $(1)/etc/ssl/certs
	$(CP) ./files/* $(1)/etc/ssl/certs
endef

$(eval $(call BuildPackage,ca-certs-godaddy))
