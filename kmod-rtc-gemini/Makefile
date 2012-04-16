#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtc-gemini
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtc-gemini
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Gemini RTC driver
  KCONFIG:=CONFIG_RTC_DRV_GEMINI
  FILES:=$(LINUX_DIR)/drivers/rtc/rtc-gemini.ko
  AUTOLOAD:=$(call AutoLoad,05,rtc-gemini)
endef

define KernelPackage/rtc-gemini/description
  Gemini RTC driver
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile
endef

$(eval $(call KernelPackage,rtc-gemini))
