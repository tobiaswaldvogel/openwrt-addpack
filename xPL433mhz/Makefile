#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=xpl433mhz
PKG_VERSION=1.0
PKG_RELEASE=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/xpl433mhz
  SECTION:=net
  CATEGORY:=Network
  TITLE:=xPL Server for 433mhz remote devices
  DEPENDS:=+libxpl +alsa-lib
endef

define Package/xpl433mhz/description
  xPL Server for 433mhz remote devices
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	( cd $(PKG_BUILD_DIR); \
		$(TARGET_CC) \
			-I$(STAGING_DIR)/usr/include $(TARGET_CFLAGS) \
			-L$(STAGING_DIR)/usr/lib \
			-Wall -Wno-unused-result -Werror \
			-l xPL -lasound -pthread \
			-o xplmulti \
			main.c xpld.c xpl433mhz.c args.c )
	( cd $(PKG_BUILD_DIR); \
		$(TARGET_CC) \
			-I$(STAGING_DIR)/usr/include $(TARGET_CFLAGS) \
			-L$(STAGING_DIR)/usr/lib \
			-Wall -Wno-unused-result -Werror \
			-l xPL -lasound \
			-o test test.c )
endef


define Package/xpl433mhz/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/xplmulti $(1)/usr/sbin/
	$(LN) xplmulti $(1)/usr/sbin/xpl433mhz
endef

$(eval $(call BuildPackage,xpl433mhz))
