#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pam-krb5
PKG_VERSION:=4.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://archives.eyrie.org/software/kerberos

include $(INCLUDE_DIR)/package.mk

#EXTRA_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib

EXTRA_CFLAGS += -I$(STAGING_DIR)/usr/include/krb5
#EXTRA_LDFLAGS += \
#        GSSGLUE_LIBS="-lkrb5 -lk5crypto -lcom_err -lkeyutils -lresolv -gssapi_krb5" \


define Package/pam-krb5
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=PAM module for Kerbers 5
  DEPENDS:= +krb5-libs +libpam
  URL:=http://http://www.eyrie.org/~eagle/software/pam-krb5/
endef

define Package/pam-krb5/description
 PAM module for Kerbers 5
endef

EXTRA_CFLAGS=-I$(STAGING_DIR)/usr/include/krb5/

define Package/pam-krb5/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/lib/security
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/.libs/pam_krb5.so $(1)/lib/security
endef

$(eval $(call BuildPackage,pam-krb5))
