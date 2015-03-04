#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openssh
PKG_VERSION:=6.7p1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://ftp.openbsd.org/pub/OpenBSD/OpenSSH/portable/ \
		http://ftp.belnet.be/pub/OpenBSD/OpenSSH/portable/
PKG_MD5SUM:=

PKG_LICENSE:=BSD ISC
PKG_LICENSE_FILE:=LICENCE


PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_BUILD_DEPENDS:=libopenssl

include $(INCLUDE_DIR)/package.mk

define Package/openssh-server-pam-krb5
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=+libopenssl +zlib +SSP_SUPPORT:libssp +libpthread +openssh-keygen +libpam +krb5-mit-libs +libopenldap-sasl
	MAINTAINER:=Peter Wagner <tripolar@gmx.at>
	URL:=http://www.openssh.com/
	SUBMENU:=SSH
	TITLE:=OpenSSH server (with PAM and KRB5 support)
	VARIANT:=with-pam-krb5
endef

define Package/openssh-server-pam-krb5/conffiles
/etc/ssh/moduli
/etc/ssh/sshd_config
/etc/ssh/ssh_host*
endef

CONFIGURE_ARGS+= \
	$(DISABLE_NLS) \
	--sysconfdir=/etc/ssh \
	--with-privsep-user=sshd \
	--with-privsep-path=/var/empty \
	--disable-strip \
	--disable-etc-default-login \
	--disable-lastlog \
	--disable-utmp \
	--disable-utmpx \
	--disable-wtmp \
	--disable-wtmpx \
	--without-bsd-auth \
	--with-kerberos5=$(STAGING_DIR)/usr \
	--with-pam \
	--with-ssl-engine

ifneq ($(CONFIG_SSP_SUPPORT),y)
CONFIGURE_ARGS += \
	--without-stackprotect
endif

CONFIGURE_VARS += LD="$(TARGET_CC)"
TARGET_LDFLAGS += -lpthread
EXTRA_CFLAGS += -I$(STAGING_DIR)/usr/include/krb5
EXTRA_LDFLAGS += -lkrb5support

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		LIBS="" \
		sftp-server
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		STRIP_OPT="" \
		all install
endef

define Package/openssh-server-pam-krb5/install
	$(INSTALL_DIR) $(1)/etc/ssh
	chmod 0700 $(1)/etc/ssh
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/ssh/sshd_config $(1)/etc/ssh/
	sed -i 's,#PasswordAuthentication yes,PasswordAuthentication no,g' $(1)/etc/ssh/sshd_config
	sed -i 's,#UsePAM no,UsePAM yes,g' $(1)/etc/ssh/sshd_config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/sshd.init $(1)/etc/init.d/sshd
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/sshd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/pam.d
	$(LN) other $(1)/etc/pam.d/sshd
endef

$(eval $(call BuildPackage,openssh-server-pam-krb5))
