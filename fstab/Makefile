# Copyright (C) 2009-2010 OpenWrt.org
# All rights reserved.

include $(TOPDIR)/rules.mk

PKG_NAME:=fstab
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/fstab
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=fstab mount script
endef

define Package/fstab/description
  fstab automount script
endef


define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/fstab/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/fstab.init $(1)/etc/init.d/fstab
endef

$(eval $(call BuildPackage,fstab))
