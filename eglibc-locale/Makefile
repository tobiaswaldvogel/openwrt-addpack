#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=eglibc-locale
PKG_VERSION:=$(call qstrip,$(CONFIG_EGLIBC_VERSION))
PKG_REVISION:=$(call qstrip,$(CONFIG_EGLIBC_REVISION))
PKG_RELEASE:=1

PKG_BUILD_DEPENDS:=eglibc-locale/host

EGLIBC_SOURCE_DIR:=$(BUILD_DIR_TOOLCHAIN)/eglibc-$(PKG_VERSION)-r$(PKG_REVISION)
HOST_BUILD_DIR:=$(BUILD_DIR_TOOLCHAIN)/localedef

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/eglibc-locale/default
  SECTION:=locales
  CATEGORY:=Locales
  URL:=http://www.eglibc.org
endef

define Package/eglibc-locale-en-utf8
  $(call Package/eglibc-locale/default)  
  TITLE:=Locale en_US.utf8 for eglibc
endef

define Package/eglibc-locale-de-utf8
  $(call Package/eglibc-locale/default)
  TITLE:=Locale de_DE.utf8 for eglibc
endef

HOST_CONFIGURE_VARS += --with-glibc=$(EGLIBC_SOURCE_DIR)/libc

define Host/Prepare
	mkdir -p $(HOST_BUILD_DIR)
	$(CP) $(EGLIBC_SOURCE_DIR)/localedef/* $(HOST_BUILD_DIR)
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/usr/bin
	$(CP) $(HOST_BUILD_DIR)/localedef $(STAGING_DIR_HOST)/usr/bin	
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/eglibc-locale-en-utf8/install
	(cd $(EGLIBC_SOURCE_DIR)/libc/localedata/locales; \
		$(STAGING_DIR_HOST)/usr/bin/localedef --no-archive -f ../charmaps/UTF-8 -i en_US en_US.utf8 \
	)
	$(INSTALL_DIR) $(1)/lib/locale
	$(CP) $(STAGING_DIR_HOST)/lib/locale/en_US.utf8 $(1)/lib/locale
endef

define Package/eglibc-locale-de-utf8/install
	(cd $(EGLIBC_SOURCE_DIR)/libc/localedata/locales; \
		$(STAGING_DIR_HOST)/usr/bin/localedef --no-archive -f ../charmaps/UTF-8 -i de_DE de_DE.utf8 \
	)
	$(INSTALL_DIR) $(1)/lib/locale
	$(CP) $(STAGING_DIR_HOST)/lib/locale/de_DE.utf8 $(1)/lib/locale
endef


$(eval $(call HostBuild))
$(eval $(call BuildPackage,eglibc-locale-en-utf8))
$(eval $(call BuildPackage,eglibc-locale-de-utf8))
