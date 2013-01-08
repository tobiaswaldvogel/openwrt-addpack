#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=diskdev_cmds
PKG_VERSION:=332.25
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.opensource.apple.com/tarballs/$(PKG_NAME)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

EXTRA_CFLAGS += -I$(PKG_BUILD_DIR)/include
MAKE_FLAGS += -f Makefile.lnx

define Package/hfs-fsck
  SECTION:=utilities
  CATEGORY:=Utilities
  TITLE:=Utility for checking and repairing HFS filesystems
  URL:=http://www.opensource.apple.com
  DEPENDS:=+libbsd
endef

define Package/hfs-mkfs
  SECTION:=utilities
  CATEGORY:=Utilities
  TITLE:=Utility for creating HFS filesystems
  URL:=http://www.opensource.apple.com
  DEPENDS:=+libopenssl
endef

define Package/hfs-fsck/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fsck_hfs.tproj/fsck_hfs $(1)/usr/sbin/fsck.hfs
endef

define Package/hfs-mkfs/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/newfs_hfs.tproj/newfs_hfs $(1)/usr/sbin/mkfs.hfs
endef

$(eval $(call BuildPackage,hfs-fsck))
$(eval $(call BuildPackage,hfs-mkfs))
