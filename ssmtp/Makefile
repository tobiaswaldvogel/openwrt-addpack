include $(TOPDIR)/rules.mk

PKG_NAME:=ssmtp
PKG_VERSION:=2.64
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.bz2
PKG_SOURCE_URL:=http://ftp.debian.org/debian/pool/main/s/ssmtp
PKG_MD5SUM:=65b4e0df4934a6cd08c506cabcbe584f

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += $(TARGET_CPPFLAGS)

define Package/$(PKG_NAME)
  SECTION:=mail
  CATEGORY:=Mail
  DEPENDS:=+libopenssl
  TITLE:=A minimal and secure mail sender
  URL:=http://packages.debian.org/ssmtp
endef

define Package/ssmtp/description
 A secure, effective and simple way of getting mail off a system to your
 mail hub. It contains no suid-binaries or other dangerous things - no
 mail spool to poke around in, and no daemons running in the background.
 mail is simply forwarded to the configured mailhost. Extremely easy
 configuration.
endef

define Package/luci-app-$(PKG_NAME)
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  DEPENDS:=+ssmtp
  TITLE:=E-Mail notification (ssmtp)
endef

define Package/luci2-$(PKG_NAME)
  SECTION:=luci2
  CATEGORY:=LuCI2
  TITLE:=LuCI2 $(PKG_NAME) module
  DEPENDS:=luci2 +ssmtp
endef

CONFIGURE_VARS += \
	LIBS="$(TARGET_LDFLAGS) -lcrypto -lssl"

CONFIGURE_ARGS += \
	--enable-ssl

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/ssmtp.init $(1)/etc/init.d/ssmtp

	$(INSTALL_DIR) $(1)/etc/ssmtp
	ln -sf /var/etc/ssmtp.conf $(1)/etc/ssmtp/ssmtp.conf
	ln -sf /var/etc/revaliases $(1)/etc/ssmtp/revaliases
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ssmtp $(1)/usr/sbin/
	ln -sf ssmtp $(1)/usr/sbin/sendmail
endef

define Package/luci-app-$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) luci/$(PKG_NAME).controller.lua $(1)/usr/lib/lua/luci/controller/$(PKG_NAME).lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DATA) luci/$(PKG_NAME).cbi.lua $(1)/usr/lib/lua/luci/model/cbi/$(PKG_NAME).lua
#	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
#	$(CP) files/$(APP_NAME).*.lmo $(1)/usr/lib/lua/luci/i18n/
endef

define Package/luci2-$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/rpcd/menu.d
	$(INSTALL_DATA) ./luci2/services.ssmtp.json.menu $(1)/usr/share/rpcd/menu.d/services.ssmtp.json
	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) ./luci2/services.ssmtp.json.acl $(1)/usr/share/rpcd/acl.d/services.ssmtp.json
	$(INSTALL_DIR) $(1)/www/luci2/template
	$(INSTALL_DATA) ./luci2/services.ssmtp.htm $(1)/www/luci2/template/
	$(INSTALL_DIR) $(1)/www/luci2/view
	$(INSTALL_DATA) ./luci2/services.ssmtp.js $(1)/www/luci2/view/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,luci-app-$(PKG_NAME)))
$(eval $(call BuildPackage,luci2-$(PKG_NAME)))
