#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=chan_dongle
PKG_VERSION:=1.1.r10
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=http://asterisk-chan-dongle.googlecode.com/files/
#PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/asterisk18-chan-dongle
  SUBMENU:=Telephony
  SECTION:=net
  CATEGORY:=Network
  TITLE:=dongle channel provider for asterisk
  URL:=http://wiki.e1550.mobi/doku.php?id=installation#install_chan_dongle
endef

define Package/asterisk18-chan-dongle/description
 dongle channel provider for asterisk.
endef

define Build/Configure
	$(call Build/Configure/Default, \
		 --with-asterisk=$(STAGING_DIR)/usr/include/asterisk-1.8/include \
	)
endef

define Build/Compile
	$(SED) 's,-I/usr/include,,g' $(PKG_BUILD_DIR)/Makefile
	$(MAKE) -C "$(PKG_BUILD_DIR)" all
endef

define Package/asterisk18-chan-dongle/install
	$(INSTALL_DIR) $(1)/usr/lib/asterisk/modules
	$(CP) $(PKG_BUILD_DIR)/chan_dongle.so $(1)/usr/lib/asterisk/modules/
endef

$(eval $(call BuildPackage,asterisk18-chan-dongle))

