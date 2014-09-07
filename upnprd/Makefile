#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=upnprd
PKG_VERSION:=20140809
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/phillipberndt/upnprd.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=b11cc041acef74ff012104532a0a023fcb727d25
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/upnprd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=uPnP relay daemon
  URL:=https://github.com/phillipberndt/upnprd/
  DEPENDS:=
endef

define Package/upnprd/description
  uPnP relay daemon
endef

define Build/Configure
endef

define Build/Compile
	(cd  $(PKG_BUILD_DIR); \
		$(TARGET_CC) $(TARGET_CFLAGS) -Wall \
		-I$(STAGING_DIR)/usr/include \
		-L$(STAGING_DIR)/usr/lib \
		-o $(PKG_NAME) \
		$(PKG_NAME).c \
	)
endef

define Package/upnprd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/bin
endef

$(eval $(call BuildPackage,upnprd))
