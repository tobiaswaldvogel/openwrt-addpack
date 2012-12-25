#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id$

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=lio
PKG_VERSION:=2012-11-28
PKG_RELEASE:=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=?
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://lio.svn.sourceforge.net/svnroot/lio/trunk
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.by2

include $(INCLUDE_DIR)/package.mk

OTHER_MENU:=Other modules

define Package/lio
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+kmod-lio +kmod-iscsi-lio +kmod-lio-vdisk
  TITLE:=lio open source iSCSI target
  URL:=http://lio.sourceforge.net/
endef

define KernelPackage/lio
  SUBMENU:=$(OTHER_MENU)
  TITLE:=lio kernel module
  FILES:=$(PKG_BUILD_DIR)/lio/src/lio.ko
endef

define KernelPackage/iscsi-lio
  SUBMENU:=$(OTHER_MENU)
  TITLE:=lio iscsi support
  FILES:=$(PKG_BUILD_DIR)/iscsi-lio/kernel/iscsi-lio.ko
  DEPENDS:=+kmod-lio
endef

define KernelPackage/lio-vdisk
  SUBMENU:=$(OTHER_MENU)
  TITLE:=lio vdisk support
  FILES:=$(PKG_BUILD_DIR)/lio/src/dev_handlers/lio_vdisk.ko
  DEPENDS:=+kmod-lio +kmod-lib-crc32c
endef

NO_SIGN_COMPARE:='s!-Wno-missing-field-initializers)!-Wno-missing-field-initializers -Wno-sign-compare)!g'
NO_DEBUG:='s!EXTRA_CFLAGS += -DCONFIG_lio_DEBUG -g -fno-inline -fno-inline-functions!\#EXTRA_CFLAGS += -DCONFIG_lio_DEBUG -g -fno-inline -fno-inline-functions!g'
NO_EXTRA_CHECKS:='s!EXTRA_CFLAGS += -DCONFIG_lio_EXTRACHECKS!\#EXTRA_CFLAGS += -DCONFIG_lio_EXTRACHECKS!g'
NO_DEBUG_O:='s!lio-y        += lio_debug.o!\#lio-y        += lio_debug.o!g'

define Build/Configure
        $(call Build/Configure/Default)

	$(SED) $(NO_SIGN_COMPARE) $(PKG_BUILD_DIR)/lio/src/Makefile
	$(SED) $(NO_DEBUG_O)      $(PKG_BUILD_DIR)/lio/src/Makefile
	$(SED) $(NO_EXTRA_CHECKS) $(PKG_BUILD_DIR)/lio/src/Makefile
	$(SED) $(NO_DEBUG)        $(PKG_BUILD_DIR)/lio/src/Makefile

	$(SED) $(NO_SIGN_COMPARE) $(PKG_BUILD_DIR)/lio/src/dev_handlers/Makefile
	$(SED) $(NO_EXTRA_CHECKS) $(PKG_BUILD_DIR)/lio/src/dev_handlers/Makefile
	$(SED) $(NO_DEBUG)        $(PKG_BUILD_DIR)/lio/src/dev_handlers/Makefile

	$(SED) $(NO_SIGN_COMPARE) $(PKG_BUILD_DIR)/iscsi-lio/kernel/Makefile
	$(SED) $(NO_EXTRA_CHECKS) $(PKG_BUILD_DIR)/iscsi-lio/kernel/Makefile
	$(SED) $(NO_DEBUG)        $(PKG_BUILD_DIR)/iscsi-lio/kernel/Makefile
endef

MAKE_FLAGS += KVER=$(LINUX_VERSION) KDIR=$(LINUX_DIR)

define Build/Compile
	$(call Build/Compile/Default,lio iscsi)
endef

define Package/lio/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iscsi-lio/usr/iscsi-liod $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
endef

define Package/lio/postinst
#!/bin/sh
grep -q 'lio' $${IPKG_INSTROOT}/etc/config/ucitrack 2>/dev/null
[ $$? -ne 0 ] && {
	echo >>$${IPKG_INSTROOT}/etc/config/ucitrack ""
	echo >>$${IPKG_INSTROOT}/etc/config/ucitrack "config lio"
	echo >>$${IPKG_INSTROOT}/etc/config/ucitrack "       option init 'lio'"
}
exit 0
endef

$(eval $(call BuildPackage,lio))
$(eval $(call KernelPackage,lio))
$(eval $(call KernelPackage,lio-vdisk))
$(eval $(call KernelPackage,iscsi-lio))
