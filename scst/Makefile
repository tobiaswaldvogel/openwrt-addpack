#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id$

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=scst
PKG_REV:=4725
PKG_VER:=2.2.x
PKG_VERSION:=$(PKG_VER)-$(PKG_REV)
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=svn
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
ifeq ($(PKG_VER),trunk)
PKG_SOURCE_URL:=https://scst.svn.sourceforge.net/svnroot/scst/trunk
else
PKG_SOURCE_URL:=https://scst.svn.sourceforge.net/svnroot/scst/branches/$(PKG_VER)
endif
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.bz2
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

OTHER_MENU:=Other modules

define Package/scst
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+kmod-scst +kmod-iscsi-scst +kmod-scst-vdisk
  TITLE:=SCST open source iSCSI target
  URL:=http://scst.sourceforge.net/
endef

define KernelPackage/scst
  SUBMENU:=$(OTHER_MENU)
  TITLE:=SCST kernel module
  FILES:=$(PKG_BUILD_DIR)/scst/src/scst.ko
endef

define KernelPackage/iscsi-scst
  SUBMENU:=$(OTHER_MENU)
  TITLE:=SCST iscsi support
  FILES:=$(PKG_BUILD_DIR)/iscsi-scst/kernel/iscsi-scst.ko
  DEPENDS:=+kmod-scst
endef

define KernelPackage/scst-vdisk
  SUBMENU:=$(OTHER_MENU)
  TITLE:=SCST vdisk support
  FILES:=$(PKG_BUILD_DIR)/scst/src/dev_handlers/scst_vdisk.ko
  DEPENDS:=+kmod-scst +kmod-lib-crc32c
endef

NO_SIGN_COMPARE:='s!-Wno-missing-field-initializers)!-Wno-missing-field-initializers -Wno-sign-compare)!g'
NO_DEBUG:='s!EXTRA_CFLAGS += -DCONFIG_SCST_DEBUG -g -fno-inline -fno-inline-functions!\#EXTRA_CFLAGS += -DCONFIG_SCST_DEBUG -g -fno-inline -fno-inline-functions!g'
NO_EXTRA_CHECKS:='s!EXTRA_CFLAGS += -DCONFIG_SCST_EXTRACHECKS!\#EXTRA_CFLAGS += -DCONFIG_SCST_EXTRACHECKS!g'
NO_DEBUG_O:='s!scst-y        += scst_debug.o!\#scst-y        += scst_debug.o!g'

define Build/Configure
        $(call Build/Configure/Default)

	$(SED) $(NO_SIGN_COMPARE) $(PKG_BUILD_DIR)/scst/src/Makefile
	$(SED) $(NO_DEBUG_O)      $(PKG_BUILD_DIR)/scst/src/Makefile
	$(SED) $(NO_EXTRA_CHECKS) $(PKG_BUILD_DIR)/scst/src/Makefile
	$(SED) $(NO_DEBUG)        $(PKG_BUILD_DIR)/scst/src/Makefile

	$(SED) $(NO_SIGN_COMPARE) $(PKG_BUILD_DIR)/scst/src/dev_handlers/Makefile
	$(SED) $(NO_EXTRA_CHECKS) $(PKG_BUILD_DIR)/scst/src/dev_handlers/Makefile
	$(SED) $(NO_DEBUG)        $(PKG_BUILD_DIR)/scst/src/dev_handlers/Makefile

	$(SED) $(NO_SIGN_COMPARE) $(PKG_BUILD_DIR)/iscsi-scst/kernel/Makefile
	$(SED) $(NO_EXTRA_CHECKS) $(PKG_BUILD_DIR)/iscsi-scst/kernel/Makefile
	$(SED) $(NO_DEBUG)        $(PKG_BUILD_DIR)/iscsi-scst/kernel/Makefile
endef

MAKE_FLAGS += KVER=$(LINUX_VERSION) KDIR=$(LINUX_DIR)

define Build/Compile
	$(call Build/Compile/Default,scst iscsi)
endef

define Package/scst/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iscsi-scst/usr/iscsi-scstd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
endef

define Package/scst/postinst
#!/bin/sh
grep -q 'scst' $${IPKG_INSTROOT}/etc/config/ucitrack 2>/dev/null
[ $$? -ne 0 ] && {
	echo >>$${IPKG_INSTROOT}/etc/config/ucitrack ""
	echo >>$${IPKG_INSTROOT}/etc/config/ucitrack "config scst"
	echo >>$${IPKG_INSTROOT}/etc/config/ucitrack "       option init 'scst'"
}
exit 0
endef

$(eval $(call BuildPackage,scst))
$(eval $(call KernelPackage,scst))
$(eval $(call KernelPackage,scst-vdisk))
$(eval $(call KernelPackage,iscsi-scst))
