#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=xPL4Linux
PKG_VERSION=1.3a
PKG_RELEASE=1
PKG_BUILD_DEPENDS:=luci
PO2LMO:=$(STAGING_DIR_HOST)/bin/po2lmo

PKG_SOURCE=xPLLib.tgz
PKG_SOURCE_URL:=http://www.xpl4java.org/xPL4Linux/downloads
PKG_BUILD_DIR=$(BUILD_DIR)/xPLLib

include $(INCLUDE_DIR)/package.mk

define Package/xpl4linux
  SECTION:=net
  CATEGORY:=Network
  TITLE:=xPL multicall
  URL:=http://www.xpl4java.org/xPL4Linux
  DEPENDS:=+alsa-lib +libmbedtls
endef

define Package/xpl4linux/description
  xPL multicall
endef

define Package/libxpl
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Framework for xPL on Linux
  URL:=http://www.xpl4java.org/xPL4Linux
  DEPENDS:=
endef

define Package/libxpl/description
  Framework for xPL on Linux
endef

define Package/luci-app-xpl
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  DEPENDS:=+xpl4linux
  TITLE:=xPL server for USB RF device and BroadLink
endef

define Package/luci-app-xpl/description
 xPL server for USB RF device and BroadLink
endef

#define Build/Prepare
#	$(call Build/Prepare/Default)
#	mkdir -p $(PKG_BUILD_DIR)/multicall
#	#$(CP) src/* $(PKG_BUILD_DIR)/multicall/
#endef

define Build/Configure
endef

XPL_LIB=\
	xPL-io.c \
	xPL-utils.c \
	xPL-service.c \
	xPL-message.c \
	xPL-listeners.c \
	xPL-store.c \
	xPL-config.c \
	xPL-hub.c



XPL_MULTI=\
	main.c \
	xpld.c \
	code_ir_rf.c \
	crc.c \
	xpl433mhz.c \
	xplbl.c \
	xplfwd.c \
	alsa.c \
	args.c \
	log.c \
	xplsend.o

XPL_DEPS=\
	-L$(STAGING_DIR)/usr/lib \
	-ldl \
	-lrt \
	-lpthread \
	-lasound

EXTRA_CFLAGS=\
	-Wall -Wno-unused-result \
	-Werror -Wno-format-security \
	-Wno-logical-not-parentheses \
	-fPIC -fpic

define Build/Compile
	(cd  $(PKG_BUILD_DIR); \
		$(TARGET_CC) $(TARGET_CFLAGS) $(EXTRA_CFLAGS) \
		-I$(STAGING_DIR)/usr/include -L$(STAGING_DIR)/usr/lib \
		-Dmain=xplsend_main -DprintUsage=xplsend_printUsage \
		-c -o xplsend.o examples/xPLSend.c; \
		$(TARGET_CC) $(TARGET_CFLAGS) $(EXTRA_CFLAGS) \
		-I$(STAGING_DIR)/usr/include -I$(PKG_BUILD_DIR) \
		-o xplmulti $(XPL_LIB) $(XPL_MULTI) $(XPL_DEPS); \
		$(TARGET_CC) $(TARGET_CFLAGS) $(EXTRA_CFLAGS) \
		-I$(STAGING_DIR)/usr/include -L$(STAGING_DIR)/usr/lib \
		-shared -o libxPL.so $(XPL_LIB); \
	)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/libxPL.so $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/xPL.h $(1)/usr/include/

endef

define Package/libxpl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libxPL.so $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/examples/xPL_Logger $(1)/usr/bin
endef

define Package/xpl4linux/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/xplmulti $(1)/usr/bin
	$(LN) xplmulti $(1)/usr/bin/xpld
	$(LN) xplmulti $(1)/usr/bin/xplsend
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/xpld.init $(1)/etc/init.d/xpld
endef

define Package/xpl4linux/postinst
#!/bin/sh
grep -q 'xpld' $${IPKG_INSTROOT}/etc/config/ucitrack 2>/dev/null
[ $$? -ne 0 ] && {
	echo >>$${IPKG_INSTROOT}/etc/config/ucitrack ""
	echo >>$${IPKG_INSTROOT}/etc/config/ucitrack "config xpld"
	echo >>$${IPKG_INSTROOT}/etc/config/ucitrack "       option init 'xpld'"
}
exit 0
endef

define Package/luci-app-xpl/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) luci/xpl.lua    $(1)/usr/lib/lua/luci/controller/xpl.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/xpl
	$(INSTALL_DATA) luci/model/forwards.lua    $(1)/usr/lib/lua/luci/model/cbi/xpl/forwards.lua
	$(INSTALL_DATA) luci/model/irrf_codes.lua   $(1)/usr/lib/lua/luci/model/cbi/xpl/irrf_codes.lua
	$(INSTALL_DATA) luci/model/irrf_values.lua  $(1)/usr/lib/lua/luci/model/cbi/xpl/irrf_values.lua
	$(INSTALL_DATA) luci/model/schedule.lua    $(1)/usr/lib/lua/luci/model/cbi/xpl/schedule.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/cbi
	$(INSTALL_DATA) luci/view/tsection2.htm    $(1)/usr/lib/lua/luci/view/cbi/tsection2.htm
	$(INSTALL_DATA) luci/view/ucisection2.htm $(1)/usr/lib/lua/luci/view/cbi/ucisection2.htm
	
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	for transl in luci/*.po; do \
		[ ! -e $$$$transl ] && continue; \
		transl_dest=`basename $$$$transl .po`.lmo; \
		$(PO2LMO) $$$$transl $(1)/usr/lib/lua/luci/i18n/$$$$transl_dest; \
	done
endef

$(eval $(call BuildPackage,libxpl))
$(eval $(call BuildPackage,xpl4linux))
$(eval $(call BuildPackage,luci-app-xpl))

