#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=pata-gemini
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/pata-gemini
  SUBMENU:=Block Devices
  DEPENDS:=
  KCONFIG:=CONFIG_PATA_GEMINI
  TITLE:=PATA driver for gemini SoC
  FILES:=$(LINUX_DIR)/drivers/ata/pata_gemini.ko
endef

define KernelPackage/pata-gemini/description
	PATA driver for gemini SoC
endef

define KernelPackage/pata-gemini/conffiles
	/etc/config/pata_gemini
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile
endef

define KernelPackage/pata-gemini/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/pata_gemini.config $(1)/etc/config/pata_gemini
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/pata_gemini.init $(1)/etc/init.d/pata_gemini
endef

$(eval $(call KernelPackage,pata-gemini))
