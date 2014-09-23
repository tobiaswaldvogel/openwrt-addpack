# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nfsv4-server
PKG_VERSION:=1.3.0
PKG_RELEASE:=1

PKG_REV:=33f2f6df1cd14243ba2f5df7b597aa048bc276a3

PKG_SOURCE:=nfs-utils-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=git://git.linux-nfs.org/projects/steved/nfs-utils.git
PKG_SOURCE_SUBDIR:=nfs-utils-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_PROTO:=git

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/nfs-utils-$(PKG_VERSION)
BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/nfs-utils-$(PKG_VERSION)

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=$(PKG_NAME)/host

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/nfsv4-server/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Filesystem
  DEPENDS:=+libwrap +libblkid +libuuid $(LIBRPC_DEPENDS)
  URL:=http://nfs.sourceforge.net/
  MAINTAINER:=Peter Wagner <tripolar@gmx.at>
endef

define Package/nfsv4-server
  $(call Package/nfsv4-server/Default)
  TITLE:=Kernel NFS server support
  DEPENDS+= +kmod-fs-nfsv4d +kmod-fs-nfsv4 +rpcbind +libnfsidmap
endef

define Package/nfsv4-server/description
  Kernel NFS server support
endef

define Package/nfsv4-server-utils
$(call Package/nfsv4-server/Default)
  TITLE:=NFS server utils
  DEPENDS:=nfsv4-server
endef

define Package/nfsv4-server-utils/description
  NFS server utils
endef

define Package/nfsv4-server/conffiles
/etc/exports
endef

define Package/nfsv4-utils
  $(call Package/nfsv4-server/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS+= +libevent +libtirpc +libkeyutils
  TITLE:=updated mount utility (includes nfs4)
endef

define Package/nfs-utils/description
  Updated mount.nfs command - allows mounting nfs4 volumes
endef

TARGET_CFLAGS += -I$(PKG_BUILD_DIR)/lib -I$(STAGING_DIR)/usr/include/libevent \
		 -I$(STAGING_DIR)/usr/include/ -Drpc_uint=uint
TARGET_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib -ltirpc \
		  -L$(STAGING_DIR)/usr/lib/libevent

CONFIGURE_ARGS += \
	--enable-gss \
	--enable-nfsv4 \
	--enable-nfsv41 \
	--enable-static \
	--enable-shared \
	--disable-caps \
	--enable-tirpc \
	--disable-gss \
	--without-krb5 \
	--disable-nfsdcld

CONFIGURE_VARS += \
	CONFIG_SQLITE3_TRUE="\#" \
	CONFIG_NFSDCLD_TRUE="\#"

MAKE_FLAGS += \
	OPT="$(TARGET_CFLAGS)" \
	INSTALLSUID="install -m 4755" \
	DESTDIR="$(PKG_INSTALL_DIR)" \
	RPCGEN_PATH=$(STAGING_DIR_HOST)/bin/rpcgen \
	RPCGEN=$(STAGING_DIR_HOST)/bin/rpcgen

HOST_CFLAGS += -Dlinux

HOST_CONFIGURE_ARGS += \
	--disable-gss \
	--disable-nfsv4 \
	--disable-nfsv41 \
	--without-tcp-wrappers

HOST_CONFIGURE_VARS += \
	ac_cv_lib_event_event_dispatch=yes \
	ac_cv_lib_nfsidmap_nfs4_init_name_mapping=yes \
	ac_cv_lib_blkid_blkid_get_library_version=yes \
	ac_cv_header_event_h=yes \
	ac_cv_header_nfsidmap_h=yes \
	ac_cv_header_blkid_blkid_h=yes \
	GSSGLUE_CFLAGS=" " \
	GSSGLUE_LIBS=" " \
	RPCSECGSS_CFLAGS=" " \
	RPCSECGSS_LIBS=" " \
	CONFIG_SQLITE3_TRUE="\#" \
	CONFIG_NFSDCLD_TRUE="\#"

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR)/tools/rpcgen all
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/tools/rpcgen/rpcgen $(STAGING_DIR_HOST)/bin/rpcgen
endef

define Package/nfsv4-server/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/usr/sbin
	$(INSTALL_DATA) ./files/nfsd.exports $(1)/etc/exports
	$(INSTALL_BIN) ./files/nfsd.init $(1)/etc/init.d/nfsd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utils/statd/sm-notify $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utils/statd/statd $(1)/usr/sbin/rpc.statd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utils/nfsd/nfsd $(1)/usr/sbin/rpc.nfsd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utils/mountd/mountd $(1)/usr/sbin/rpc.mountd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utils/exportfs/exportfs $(1)/usr/sbin/
endef

define Package/nfsv4-server-utils/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utils/showmount/showmount $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utils/nfsstat/nfsstat $(1)/usr/sbin
endef

define Package/nfs-utils/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/sbin/mount.nfs $(1)/sbin/
	$(LN) mount.nfs $(1)/sbin/mount.nfs4
	$(LN) mount.nfs $(1)/sbin/umount.nfs
	$(LN) mount.nfs $(1)/sbin/umount.nfs4
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,nfsv4-server))
$(eval $(call BuildPackage,nfsv4-server-utils))
$(eval $(call BuildPackage,nfsv4-utils))

