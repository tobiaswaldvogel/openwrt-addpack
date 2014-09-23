# 
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=xfsprogs-multicall
PKG_RELEASE:=1
PKG_VERSION:=3.1.10
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git://oss.sgi.com/xfs/cmds/xfsprogs
PKG_SOURCE_SUBDIR:=xfsprogs-$(PKG_VERSION)
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_SOURCE:=xfsprogs-$(PKG_VERSION).tar.gz
PKG_MD5SUM:=

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/xfsprogs-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/xfs-tools
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Filesystem
  DEPENDS:=+libuuid +libpthread +librt
  URL:=http://oss.sgi.com/projects/xfs
  TITLE:=Multicall for xfs_repair xfs_db and xfs_mkfs
endef

CONFIGURE_ARGS += \
	--enable-gettext=no \
	--enable-lib64=no

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include \
	-D_LARGEFILE64_SOURCE \
	-D_FILE_OFFSET_BITS=64 \
	-D_GNU_SOURCE

MAKE_FLAGS += \
	DEBUG= Q= \
	PCFLAGS="-Wall" \
	PKG_PLATFORM=linux \
	ENABLE_GETTEXT=no

define Build/Configure
	cp $(PKG_BUILD_DIR)/include/install-sh   $(PKG_BUILD_DIR)
	$(LN) $(STAGING_DIR)/host/share/libtool/config/config.guess $(PKG_BUILD_DIR)/config.guess
	$(LN) $(STAGING_DIR)/host/share/libtool/config/config.sub   $(PKG_BUILD_DIR)/config.sub
	$(call Build/Configure/Default)
endef

define Package/xfs-tools/install
	mkdir -p $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/sbin/xfs_tools $(1)/usr/sbin
	$(LN) xfs_tools $(1)/usr/sbin/xfs_repair
	$(LN) xfs_tools $(1)/usr/sbin/fsck.xfs
	$(LN) xfs_tools $(1)/usr/sbin/xfs_db
	$(LN) xfs_tools $(1)/usr/sbin/xfs_growfs
	$(LN) xfs_tools $(1)/usr/sbin/mkfs.xfs
endef

$(eval $(call BuildPackage,xfs-tools))
