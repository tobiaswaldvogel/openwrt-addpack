#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dbench
PKG_VERSION:=4.0
PKG_RELEASE:=1

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.samba.org/ftp/tridge/dbench/

include $(INCLUDE_DIR)/package.mk

EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib
CONFIGURE_ARGS += \
  --prefix=$(PKG_INSTALL_DIR)/usr \
  --bindir=$(PKG_INSTALL_DIR)/usr/bin \
  --mandir=$(PKG_INSTALL_DIR)/usr/man \
  --datadir=$(PKG_INSTALL_DIR)/usr/share

define Package/dbench
  SECTION:=utilities
  CATEGORY:=Utilities
  SUBMENU:=Filesystem
  TITLE:=Filesystem benchmark
  URL:=http://www.samba.org/ftp/tridge/dbench/
  DEPENDS:=+libpopt
endef

define Package/dbench/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dbench $(1)/usr/bin
endef

$(eval $(call BuildPackage,dbench))
