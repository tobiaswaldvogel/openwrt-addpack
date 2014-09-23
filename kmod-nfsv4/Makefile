#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=kernel-nfs
PKG_RELEASE:=1

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

include $(INCLUDE_DIR)/package.mk

FS_MENU:=Filesystems

define KernelPackage/fs-nfsv4-common
  SUBMENU:=$(FS_MENU)
  TITLE:=Common NFS V4 filesystem modules
  KCONFIG+=\
	CONFIG_LOCKD \
	CONFIG_LOCKD_V4=y\
	CONFIG_SUNRPC \
	CONFIG_SUNRPC_GSS
  FILES+= $(LINUX_DIR)/net/sunrpc/auth_gss/auth_rpcgss.ko \
		$(LINUX_DIR)/net/sunrpc/sunrpc.ko \
		$(LINUX_DIR)/lib/oid_registry.ko \
		$(LINUX_DIR)/fs/nfs_common/nfs_acl.ko \
		$(LINUX_DIR)/fs/lockd/lockd.ko
  AUTOLOAD=$(call AutoLoad,30,auth_rpcgss, sunrpc, oid_registry, lockd)
endef

define KernelPackage/fs-nfsv4-common/description
 Kernel modules for NFS V4 & NFSD V4 kernel support
endef

define KernelPackage/fs-nfsv4
  SUBMENU:=$(FS_MENU)
  TITLE:=NFS filesystem support
  DEPENDS:=+kmod-fs-nfsv4-common +kmod-dnsresolver
  KCONFIG:= \
	CONFIG_NFS_FS \
	CONFIG_NFS_V4=y\
	CONFIG_NFS_V4_1=y\
	CONFIG_NFS_V4_2=y\
	CONFIG_NFS_USE_LEGACY_DNS=n \
	CONFIG_NFS_USE_NEW_IDMAPPER=n
  FILES:= \
	$(LINUX_DIR)/fs/nfs/nfs.ko \
	$(LINUX_DIR)/fs/nfs/nfsv4.ko
  AUTOLOAD:=$(call AutoLoad,40,nfs nfsv4)
endef

define KernelPackage/fs-nfsv4/description
 Kernel module for NFS4 support
endef

define KernelPackage/fs-nfsv4d
  SUBMENU:=$(FS_MENU)
  TITLE:=NFS kernel server support
  DEPENDS:=+kmod-fs-nfsv4-common +kmod-fs-exportfs
  KCONFIG:= \
	CONFIG_NFSD \
	CONFIG_NFSD_V4=y
	CONFIG_NFSD_FAULT_INJECTION=n
  FILES:=$(LINUX_DIR)/fs/nfsd/nfsd.ko
  AUTOLOAD:=$(call AutoLoad,40,nfsd)
endef

define KernelPackage/fs-nfsv4d/description
 Kernel module for NFS kernel server support
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

$(eval $(call KernelPackage,fs-nfsv4-common))
$(eval $(call KernelPackage,fs-nfsv4))
$(eval $(call KernelPackage,fs-nfsv4d))