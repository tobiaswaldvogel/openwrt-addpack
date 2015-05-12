#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id$

include $(TOPDIR)/rules.mk

PKG_NAME:=exfat
PKG_VERSION:=3d1810c4f2e59a3d03bbe4d2c97a9c3d5c4fbe7f
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=$(PKG_VERSION)
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/dorimanx/exfat-nofuse.git
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.bz2

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

FS_MENU:=Filesystems

define KernelPackage/fs-exfat
  SUBMENU:=$(FS_MENU)
  TITLE:=exfat filesystem support
  DEPENDS:=+kmod-nls-base
  FILES:=$(PKG_BUILD_DIR)/exfat.ko
  AUTOLOAD:=$(call AutoLoad,30,exfat)
endef

MAKE_FLAGS += KVER=$(LINUX_VERSION) KDIR=$(LINUX_DIR)

$(eval $(call KernelPackage,fs-exfat))
