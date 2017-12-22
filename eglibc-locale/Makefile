#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

REVISION_2.19 = 25243
REVISION_2.21 = 16d0a0c
REVISION_2.22 = b995d95

PKG_NAME:=eglibc-locale
PKG_VERSION:=$(call qstrip,$(CONFIG_GLIBC_VERSION))
PKG_REVISION:=$(REVISION_$(PKG_VERSION))
PKG_RELEASE:=1

#PKG_BUILD_DEPENDS:=eglibc-locale/host

GLIBC_SOURCE_DIR:=$(BUILD_DIR_TOOLCHAIN)/glibc-$(PKG_VERSION)-$(PKG_REVISION)
HOST_BUILD_DIR:=$(BUILD_DIR_TOOLCHAIN)/localedef
LOCALE_CONF=$(HOST_BUILD_DIR)/locale/config.h

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

HOST_EXTRA_CFLAGS = -I . -I ../ \
	-DHAVE_CONFIG_H \
	-DVERSION='"$(PKG_VERSION)"' \
	-D_libc_intl_domainname='"libc"' \
	-Dprogram_invocation_short_name='"localedef"' \
	-D_LARGEFILE64_SOURCE

define Host/Prepare
	mkdir -p $(HOST_BUILD_DIR)/include
	$(CP) $(GLIBC_SOURCE_DIR)/locale $(HOST_BUILD_DIR)/
	$(CP) $(GLIBC_SOURCE_DIR)/crypt $(HOST_BUILD_DIR)/
	$(CP) $(GLIBC_SOURCE_DIR)/include/programs/xmalloc.h $(HOST_BUILD_DIR)/locale/programs/
	echo >>$(LOCALE_CONF) '#define LOCALEDIR "$(GLIBC_SOURCE_DIR)/localedata"'
	echo >>$(LOCALE_CONF) '#define CHARMAP_PATH "$(GLIBC_SOURCE_DIR)/localedata/charmaps"'
	echo >>$(LOCALE_CONF) '#define LOCALE_PATH "$(GLIBC_SOURCE_DIR)/localedata"'
	echo >>$(LOCALE_CONF) '#define REPERTOIREMAP_PATH "$(GLIBC_SOURCE_DIR)/localedata"'
	echo >>$(LOCALE_CONF) '#define REPORT_BUGS_TO ""'
	echo >>$(LOCALE_CONF) '#define PKGVERSION "$(PKG_VERSION)"'
endef

define Host/Configure
endef

define Host/Compile
	( \
		cd $(HOST_BUILD_DIR)/locale/programs; \
		LDFLAGS="$(HOST_LDFLAGS)" $(HOSTCC) $(HOST_CFLAGS) $(HOST_EXTRA_CFLAGS) \
		localedef.c xmalloc.c xstrdup.c charmap.c linereader.c \
		repertoire.c simple-hash.c locarchive.c \
		-o localedef  \
	)
endef

#charmap.c charmap-dir.c locarchive.c 
#define Host/Compile
#	( \
#		cd $(BUILD_DIR_TOOLCHAIN)/glibc-$(PKG_VERSION)-$(PKG_REVISION); \
#		LDFLAGS="$(HOST_LDFLAGS)" CFLAGS="-O2" make -C locale \
#	)
#endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/usr/bin
	$(CP) $(HOST_BUILD_DIR)/localedef $(STAGING_DIR_HOST)/usr/bin
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/lib/locale
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/eglibc-locale-en-utf8/install
	$(INSTALL_DIR) $(1)/lib/locale
	/usr/bin/localedef \
		--no-archive \
		-f $(GLIBC_SOURCE_DIR)/localedata/charmaps/UTF-8 \
		-i $(GLIBC_SOURCE_DIR)/localedata/locales/en_US \
		$(1)/lib/locale/en_US.utf8
endef

define Package/eglibc-locale-de-utf8/install
	$(INSTALL_DIR) $(1)/lib/locale
	/usr/bin/localedef \
		--no-archive \
		-f $(GLIBC_SOURCE_DIR)/localedata/charmaps/UTF-8 \
		-i $(GLIBC_SOURCE_DIR)/localedata/locales/de_DE \
		$(1)/lib/locale/en_US.utf8
endef


$(eval $(call HostBuild))
$(eval $(call BuildPackage,eglibc-locale-en-utf8))
$(eval $(call BuildPackage,eglibc-locale-de-utf8))
