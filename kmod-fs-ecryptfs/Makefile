#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=fs-ecryptfs
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/fs-ecryptfs
  SUBMENU:=$(FS_MENU)
  TITLE:=eCrypt filesystem support
  KCONFIG:=CONFIG_ECRYPT_FS
  FILES:=$(LINUX_DIR)/fs/ecryptfs/ecryptfs.ko
  AUTOLOAD:=$(call AutoLoad,30,ecryptfs,1)
endef

define KernelPackage/fs-ecryptfs/description
  Kernel module for eCrypt support
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile
endef

$(eval $(call KernelPackage,fs-ecryptfs))
