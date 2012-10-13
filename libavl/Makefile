# $Id$

include $(TOPDIR)/rules.mk

PKG_NAME:=libavl
PKG_VERSION:=0.3.5
PKG_RELEASE:=1
PKG_MD5SUM:=882c68ea7f71876ca110f3b84d7ab12d

PKG_SOURCE_URL:=http://alioth.debian.org/~jblache/forked-daapd \
		http://www.gyfgafguf.dk/openwrt/mirror
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.gz
PKG_FIXUP:=libtool

PKG_BUILD_DIR:=$(BUILD_DIR)/avl-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/libavl
SECTION:=libs
CATEGORY:=Libraries
TITLE:=AVLTree
MAINTAINER:=Espen Jürgensen <espenjurgensen+openwrt@gmail.com>
URL:=http://www.jblache.org/projects/daapd/
endef

define Package/libavl/description
  AVLTree is a small implementation of AVL trees for the C programming language.
endef

TARGET_CFLAGS += $(FPIC)

define Build/Configure
	$(call Build/Configure/Default, \
		--enable-shared \
		--enable-static \
	)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/avl.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libavl.so* $(1)/usr/lib/
	(cd $(1)/usr/lib; $(LN) libavl.so.1.5 libavl.so)
	(cd $(1)/usr/lib; $(LN) libavl.so.1.5 libavl.so.1)
endef

define Package/libavl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libavl.so*  $(1)/usr/lib/
	(cd $(1)/usr/lib; $(LN) libavl.so.1.5 libavl.so)
	(cd $(1)/usr/lib; $(LN) libavl.so.1.5 libavl.so.1)
endef

$(eval $(call BuildPackage,libavl))
