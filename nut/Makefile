#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nut
PKG_VERSION_MAJOR=2.6
PKG_VERSION_MINOR=.5
PKG_VERSION:=$(PKG_VERSION_MAJOR)$(PKG_VERSION_MINOR)
PKG_RELEASE:=1
PKG_INSTALL:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.networkupstools.org/source/$(PKG_VERSION_MAJOR)
PKG_MD5SUM:=

include $(INCLUDE_DIR)/package.mk

define Package/nut/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=UPS
  TITLE:=Network UPS tools
  URL:=http://www.networkupstools.org/
  DEPENDS:=+libopenssl +libltdl
endef

define Package/nut
  $(call Package/nut/Default)
endef

CONFIGURE_ARGS+=--with-pidpath=/var/run

define Package/nut/install
	$(INSTALL_DIR) $(1)/etc
	$(CP)  $(PKG_INSTALL_DIR)/etc/*.sample $(1)/etc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/nut.init $(1)/etc/init.d/nut
	$(INSTALL_DIR) $(1)/usr
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/upsd $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/upsmon $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/upssched $(1)/usr/sbin
endef

define BuildModel

  define Package/nut-$(1)
    $(call Package/nut/Default)

    DEPENDS+=nut
    ifneq ($(2),)
      DEPENDS+=$(2)
    endif

    TITLE+= - $(1)
  endef

  define Package/nut-$(1)/install
	$(INSTALL_DIR) $$(1)/lib/nut
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/$(1) $$(1)/lib/nut
  endef

  $$(eval $$(call BuildPackage,nut-$(1)))

endef

$(eval $(call BuildPackage,nut))
$(eval $(call BuildModel,apcsmart-old))
$(eval $(call BuildModel,bcmxcp))
$(eval $(call BuildModel,bcmxcp_usb,+libusb-compat))
$(eval $(call BuildModel,belkin))
$(eval $(call BuildModel,belkinunv))
$(eval $(call BuildModel,bestfcom))
$(eval $(call BuildModel,bestfortress))
$(eval $(call BuildModel,bestuferrups))
$(eval $(call BuildModel,bestups))
$(eval $(call BuildModel,blazer_ser))
$(eval $(call BuildModel,blazer_usb,+libusb-compat))
$(eval $(call BuildModel,clone))
$(eval $(call BuildModel,clone-outlet))
$(eval $(call BuildModel,dummy-ups))
$(eval $(call BuildModel,etapro))
$(eval $(call BuildModel,everups))
$(eval $(call BuildModel,gamatronic))
$(eval $(call BuildModel,genericups))
$(eval $(call BuildModel,isbmex))
$(eval $(call BuildModel,ivtscd))
$(eval $(call BuildModel,liebert))
$(eval $(call BuildModel,liebert-esp2))
$(eval $(call BuildModel,masterguard))
$(eval $(call BuildModel,metasys))
$(eval $(call BuildModel,mge-shut))
$(eval $(call BuildModel,mge-utalk))
$(eval $(call BuildModel,microdowell))
$(eval $(call BuildModel,netxml-ups,+libneon))
$(eval $(call BuildModel,nut-scanner))
$(eval $(call BuildModel,oldmge-shut))
$(eval $(call BuildModel,oneac))
$(eval $(call BuildModel,optiups))
$(eval $(call BuildModel,powercom))
$(eval $(call BuildModel,powerpanel))
$(eval $(call BuildModel,rhino))
$(eval $(call BuildModel,richcomm_usb,+libusb-compat))
$(eval $(call BuildModel,safenet))
$(eval $(call BuildModel,skel))
$(eval $(call BuildModel,solis))
$(eval $(call BuildModel,tripplite))
$(eval $(call BuildModel,tripplitesu))
$(eval $(call BuildModel,tripplite_usb,+libusb-compat))
$(eval $(call BuildModel,upsc))
$(eval $(call BuildModel,upscmd))
$(eval $(call BuildModel,upscode2))
$(eval $(call BuildModel,upsdrvctl))
$(eval $(call BuildModel,upslog))
$(eval $(call BuildModel,upsrw))
$(eval $(call BuildModel,upssched-cmd))
$(eval $(call BuildModel,usbhid-ups,+libusb-compat))
$(eval $(call BuildModel,victronups))

