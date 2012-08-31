#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=quota
PKG_VERSION:=4.00
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/linuxquota
#PKG_MD5SUM:=291e249ea60f4d0637e467356a8ae41a

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-tools
PKG_FIXUP:=libtool
PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=nfs-kernel-server/host

include $(INCLUDE_DIR)/package.mk

define Package/quota
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Tools and patches for the Linux Diskquota system
  URL:=http://linuxquota.sourceforge.net/
  DEPENDS:=+libwrap +libext2fs +libcom_err $(LIBRPC_DEPENDS)
endef

define Package/quota/description
	Tools and patches for the Linux Diskquota system
endef

CONFIGURE_ARGS += \
	--prefix=$(PKG_INSTALL_DIR) \
	--enable-strip-binaries=no \
	--sysconfdir=$(PKG_INSTALL_DIR)/etc \
	--mandir=$(PKG_INSTALL_DIR)/usr/man

TARGET_CFLAGS += $(LIBRPC_INCLUDE)
EXTRA_LDFLAGS += $(LIBRPC) -lext2fs $(STAGING_DIR)/usr/lib/libcom_err.so.2 -L$(PKG_BUILD_DIR)
MAKE_FLAGS += \
	RPCGEN_PATH=$(STAGING_DIR_HOST)/bin/rpcgen \
        RPCGEN=$(STAGING_DIR_HOST)/bin/rpcgen

#define Build/InstallDev
#	$(INSTALL_DIR) $(1)/usr/include
#	$(CP) $(PKG_INSTALL_DIR)/usr/include/quota2.h $(1)/usr/include/
#	$(INSTALL_DIR) $(1)/usr/lib
#	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libquota2.{a,so*} $(1)/usr/lib/
#endef

define Package/quota/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/{quota,quotasync} $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/sbin/{convertquota,edquota,quot,quotacheck} $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/sbin/{quotaon,quotastats,repquota,rpc.rquotad,setquota,warnquota,xqmstats} $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/sbin/quotaoff $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libquota.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,quota))
