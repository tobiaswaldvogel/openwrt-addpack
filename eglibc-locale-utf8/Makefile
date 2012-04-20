#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=eglibc-locale-utf8
PKG_VERSION:=$(call qstrip,$(CONFIG_EGLIBC_VERSION))
PKG_REVISION:=$(call qstrip,$(CONFIG_EGLIBC_REVISION))
PKG_RELEASE:=1

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)-r$(PKG_REVISION)


HOST_BUILD_DIR:=$(BUILD_DIR_TOOLCHAIN)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/eglibc-locale-utf8
  SECTION:=utils
  CATEGORY:=Utilities
#  DEPENDS:=
  TITLE:=Locale en_US.utf8 for eglibc
  URL:=http://www.eglibc.org
endef

define Package/eglibc-locale-utf8/description
  Locale en_US.utf8 for eglibc
endef

#HOST_BUILD_DIR1:=$(HOST_BUILD_DIR)-initial
#HOST_BUILD_DIR2:=$(HOST_BUILD_DIR)-final

#EGLIBC_CFLAGS:= \
#	$(TARGET_CFLAGS)

#EGLIBC_CONFIGURE:= \
#	BUILD_CC="$(HOSTCC)" \
#	$(TARGET_CONFIGURE_OPTS) \
#	CFLAGS="$(EGLIBC_CFLAGS)" \
#	libc_cv_slibdir="/lib" \
#	use_ldconfig=no \
#	$(HOST_BUILD_DIR)/libc/configure \
#		--prefix= \
#		--build=$(GNU_HOST_NAME) \
#		--host=$(REAL_GNU_TARGET_NAME) \
#		--with-headers=$(TOOLCHAIN_DIR)/include \
#		--disable-profile \
#		--without-gd \
#		--without-cvs \
#		--enable-add-ons \

define Stage1/Configure
	mkdir -p $(HOST_BUILD_DIR1)
	$(CP) $(HOST_BUILD_DIR)/libc/option-groups.config $(HOST_BUILD_DIR1)/
	( cd $(HOST_BUILD_DIR1); rm -f config.cache; \
		$(EGLIBC_CONFIGURE) \
	);
endef

define Stage1/Compile
endef

define Stage1/Install
	mkdir -p $(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev/{include,lib}
	$(EGLIBC_MAKE) -C $(HOST_BUILD_DIR1) \
		install_root="$(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev" \
		install-bootstrap-headers=yes \
		install-headers 
	$(EGLIBC_MAKE) -C $(HOST_BUILD_DIR1) \
		csu/subdir_lib
	( cd $(HOST_BUILD_DIR1); \
		$(CP) csu/crt1.o csu/crti.o csu/crtn.o $(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev/lib/ \
	)
	$(TARGET_CC) -nostdlib -nostartfiles -shared -x c /dev/null \
		-o $(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev/lib/libc.so
endef

define Stage2/Configure
	mkdir -p $(HOST_BUILD_DIR2)
	$(CP) $(HOST_BUILD_DIR)/libc/option-groups.config $(HOST_BUILD_DIR2)/
	( cd $(HOST_BUILD_DIR2); rm -f config.cache; \
		$(EGLIBC_CONFIGURE) \
	);
endef

define Stage2/Compile
	$(EGLIBC_MAKE) -C $(HOST_BUILD_DIR2) all
endef

define Stage2/Install
	$(EGLIBC_MAKE) -C $(HOST_BUILD_DIR2) \
		install_root="$(TOOLCHAIN_DIR)" \
		install
	( cd $(TOOLCHAIN_DIR) ; \
		for d in lib usr/lib ; do \
		  for f in libc.so libpthread.so libgcc_s.so ; do \
		    if [ -f $$$$d/$$$$f -a ! -L $$$$d/$$$$f ] ; then \
		      $(SED) 's,/usr/lib/,,g;s,/lib/,,g' $$$$d/$$$$f ; \
		    fi \
		  done \
		done \
	)
endef

define Host/Prepare
endef

define Host/Configure
	
endef

define Host/Compile
endef

define Host/Install
endef

define Host/Clean
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,eglibc-locale-utf8))
