#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id$

include $(TOPDIR)/rules.mk

PKG_NAME:=mediatomb
PKG_VERSION:=0.12.1
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/mediatomb
PKG_MD5SUM:=e927dd5dc52d3cfcebd8ca1af6f0d3c2
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=libjs libexif libsqlite3 id3lib libiconv file
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/mediatomb
  SECTION:=multimedia
  CATEGORY:=Multimedia
  DEPENDS:=+libstdcpp +libexpat +libcurl +libexif +libstdcpp +libsqlite3 +taglib +libiconv +file +libjs
  TITLE:=A powerfull UPNP media server
  URL:=http://www.mediatomb.cc/
  VARIANT:=sqlite
endef

define Package/mediatomb-mysql
  $(call Package/mediatomb)  
  DEPENDS:=+libstdcpp +libexpat +libcurl +libexif +libstdcpp +libsqlite3 +taglib +libiconv +file +libjs +libmysqlclient-r
  TITLE:=A powerfull UPNP media server (with MySQL support)
  VARIANT:=mysql
endef

EXTRA_LDFLAGS += -ldl
CONFIGURE_ARGS += \
	--prefix=/usr \
	--with-iconv-h=$(STAGING_DIR)/usr/lib/libiconv-stub/include/ --with-iconv-libs=$(STAGING_DIR)/usr/lib/libiconv-stub/lib/ \
	--with-js-h=$(STAGING_DIR)/usr/include/js --with-js-libs=$(STAGING_DIR)/usr/lib \
	--disable-mysql \
	--enable-inotify \
	--enable-taglib \
	--disable-id3lib \
	--enable-libexif \
        --enable-libmagic \
	--disable-libextractor \
	--enable-protocolinfo-extension \
	--enable-libjs \
	--disable-ffmpeg \
	--disable-tombdebug \
	--disable-upnpdebug \
	--disable-debug-log \
	--disable-youtube

ifeq ($(BUILD_VARIANT),mysql)
        CONFIGURE_ARGS := $(filter-out --disable-mysql, ${CONFIGURE_ARGS})
        CONFIGURE_ARGS += --enable-mysql
endif

define Package/mediatomb/install
	$(INSTALL_DIR) $(1)/etc/mediatomb
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/share
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mediatomb $(1)/usr/bin/
	$(CP)  $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share/
	$(INSTALL_DATA)  ./files/config.xml $(1)/etc/mediatomb/
	$(INSTALL_BIN)  ./files/mediatomb.init $(1)/etc/init.d/mediatomb
endef

define Package/mediatomb-mysql/install
	$(call Package/mediatomb/install,$(1))
endef

$(eval $(call BuildPackage,mediatomb))
$(eval $(call BuildPackage,mediatomb-mysql))
