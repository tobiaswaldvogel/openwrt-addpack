# 
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnfsidmap
PKG_VERSION:=0.25
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.citi.umich.edu/projects/nfsv4/linux/libnfsidmap/
PKG_MD5SUM:=
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libnfsidmap
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Library for mapping names to id's (NFS)
  URL:=http://www.citi.umich.edu/projects/nfsv4/linux/
endef

define Package/libnfsidmap/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnfsidmap.so* $(1)/usr/lib
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnfsidmap.{a,so*} $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(SED) 's,/usr,${STAGING_DIR}/usr,g' $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libnfsidmap.pc
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libnfsidmap.pc $(1)/usr/lib/pkgconfig/libnfsidmap.pc
endef

$(eval $(call BuildPackage,libnfsidmap))
