# $Id$

include $(TOPDIR)/rules.mk

PKG_NAME:=libunistring
PKG_VERSION:=0.9.3
PKG_RELEASE:=1
PKG_MD5SUM:=db8eca3b64163abadf8c40e5cecc261f

PKG_SOURCE_URL:=http://ftp.gnu.org/gnu/libunistring
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_FIXUP:=libtool

PKG_BUILD_DIR:=$(BUILD_DIR)/libunistring-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

# This package is built without depending on libiconv-full, which is probably a mistake
define Package/libunistring
SECTION:=libs
CATEGORY:=Libraries
TITLE:=libunistring
URL:=http://www.gnu.org/software/libunistring/
MAINTAINER:=Espen Jürgensen <espenjurgensen+openwrt@gmail.com>
endef

define Package/libunistring/description
  This library provides functions for manipulating Unicode strings and for manipulating C strings according to the Unicode standard.
endef

TARGET_CFLAGS += $(FPIC)

define Build/Configure
	$(call Build/Configure/Default, \
		--enable-shared \
		--enable-static \
	)
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) DESTDIR="$(PKG_INSTALL_DIR)" install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/include/unistring
	$(CP) $(PKG_INSTALL_DIR)/usr/include/unistring/*.h $(1)/usr/include/unistring/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libunistring.* $(1)/usr/lib/
endef

define Package/libunistring/install
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/include/unistring
	$(CP) $(PKG_INSTALL_DIR)/usr/include/unistring/*.h $(1)/usr/include/unistring/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libunistring.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libunistring))
