include $(TOPDIR)/rules.mk

PKG_NAME:=glibc-oabi
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/glibc-oabi
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Glibc OABI version
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/glibc-oabi/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) -a ./files/* $(1)/lib
endef

$(eval $(call BuildPackage,glibc-oabi))
