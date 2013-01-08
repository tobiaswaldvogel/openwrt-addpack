#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=minissdpd
PKG_VERSION:=1.1.20120509
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://miniupnp.free.fr/files
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MD5SUM:=

include $(INCLUDE_DIR)/package.mk

define Package/minissdpd
  SECTION:=net
  CATEGORY:=Network
#  DEPENDS:=+iptables +libip4tc
  TITLE:=Lightweight UPnPD SSDP daemon
  URL:=http://miniupnp.free.fr/
  MAINTAINER:=Daniel Dickinson <openwrt@cshore.neomailbox.net>
endef

#define Package/minissdpd/conffiles
#/etc/config/upnpd
#endef

#define Build/Compile
#	echo "OpenWRT/$(OPENWRTVERSION)" | tr \(\)\  _ >$(PKG_BUILD_DIR)/os.openwrt
#	$(MAKE) -C $(PKG_BUILD_DIR) -f Makefile.linux \
#		$(TARGET_CONFIGURE_OPTS) \
#		STAGING_DIR="$(STAGING_DIR)" \
#		CC="$(TARGET_CC)" \
#		TARGET_OPENWRT=1 \
#		CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -I$(firstword $(wildcard $(KERNEL_BUILD_DIR)/iptables-*/include)) -DIPTABLES_143" \
#		LIBS="$(STAGING_DIR)/usr/lib/libip4tc.so"
#endef

define Package/minissdpd/install
#	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d $(1)/etc/config $(1)/etc/hotplug.d/iface $(1)/etc/hotplug.d/firewall
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/minissdpd $(1)/usr/sbin/minissdpd
#	$(INSTALL_BIN) ./files/minissdpd.init $(1)/etc/init.d/minissdpd
#	$(INSTALL_CONF) ./files/upnpd.config $(1)/etc/config/upnpd
#	$(INSTALL_DATA) ./files/minissdpd.iface.hotplug $(1)/etc/hotplug.d/iface/50-minissdpd
#	$(INSTALL_DATA) ./files/minissdpd.firewall.hotplug $(1)/etc/hotplug.d/firewall/50-minissdpd
#	$(INSTALL_DIR) $(1)/usr/share/minissdpd
endef

$(eval $(call BuildPackage,minissdpd))
