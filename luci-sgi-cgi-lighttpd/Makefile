include $(TOPDIR)/rules.mk

PKG_NAME:=luci-sgi-cgi-lighttpd
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=LightTPD - Activate Lua integration
  DEPENDS:=+luci-base +lighttpd +lighttpd-mod-fastcgi +lighttpd-mod-redirect +lua +fcgiwrap
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/lighttpd/conf.d
	echo -e  >$(1)/etc/lighttpd/conf.d/95-luci-cgi.conf '$$$$HTTP["url"] =~ "^/cgi-bin/" {'
	echo -e >>$(1)/etc/lighttpd/conf.d/95-luci-cgi.conf '\tfastcgi.server = ( "luci" => ((  "socket" => "/var/run/fcgiwrap.sock" )))'
	echo -e >>$(1)/etc/lighttpd/conf.d/95-luci-cgi.conf '}'
	echo -e >>$(1)/etc/lighttpd/conf.d/95-luci-cgi.conf
	echo -e >>$(1)/etc/lighttpd/conf.d/95-luci-cgi.conf 'url.redirect = ('
	echo -e >>$(1)/etc/lighttpd/conf.d/95-luci-cgi.conf '\t"^/($$$$|index.html$$$$)"\t=> "/notfound.html",'
	echo -e >>$(1)/etc/lighttpd/conf.d/95-luci-cgi.conf '\t"^/luci$$$$"\t\t=> "/cgi-bin/luci"'
	echo -e >>$(1)/etc/lighttpd/conf.d/95-luci-cgi.conf ')'

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/fcgiwrap.init $(1)/etc/init.d/fcgiwrap
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
