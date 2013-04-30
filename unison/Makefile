#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=unison
PKG_VERSION:=2.40.102
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.seas.upenn.edu/~bcpierce/unison//download/releases/stable/

include $(INCLUDE_DIR)/package.mk

#EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib

#CONFIGURE_ARGS += --enable-gettext=no

define Package/unison
  SECTION:=utilities
  CATEGORY:=Utilities
  TITLE:=Unison file synchronizer
  URL:=http://www.cis.upenn.edu/~bcpierce/unison/
#  DEPENDS:=
endef

define Build/compile
	(cd $PKG_BUILD_DIR; make UISTYLE=text NATIVE=false)
endef

define Package/unison/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/chunison/.libs/chunison $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/getfunison/.libs/getfunison $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/setfunison/.libs/setfunison $(1)/usr/bin
endef

$(eval $(call BuildPackage,unison))
