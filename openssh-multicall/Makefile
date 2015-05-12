#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openssh
PKG_VERSION:=6.8p1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://ftp.openbsd.org/pub/OpenBSD/OpenSSH/portable/ \
		http://ftp.belnet.be/pub/OpenBSD/OpenSSH/portable/
PKG_MD5SUM:=08f72de6751acfbd0892b5f003922701

PKG_LICENSE:=BSD ISC
PKG_LICENSE_FILES:=LICENCE

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_BUILD_DEPENDS:=libopenssl
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/openssh-multicall
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=+zlib +SSP_SUPPORT:libssp +libopenssl +libpthread +libpam +krb5-mit-libs
	MAINTAINER:=Peter Wagner <tripolar@gmx.at>
	URL:=http://www.openssh.com/
	SUBMENU:=SSH
	TITLE:=OpenSSH multicall (with PAM and KRB5 support)
	VARIANT:=multicall
endef

define Package/openssh-multicall/conffiles
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
	--with-openssl \
	--with-ssh1
#	--with-ssl-engine
#	--with-Werror
#	--without-hardening

ifneq ($(CONFIG_SSP_SUPPORT),y)
CONFIGURE_ARGS += \
	--without-stackprotect
endif

CONFIGURE_VARS += LD="$(TARGET_CC)"
MAKE_FLAGS += LD="$(TARGET_CC)"

define Package/openssh-multicall/install
	$(INSTALL_DIR) $(1)/etc/pam.d
	$(LN) other $(1)/etc/pam.d/sshd
	$(INSTALL_DIR) $(1)/etc/ssh
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/ssh/moduli $(1)/etc/ssh/
	$(INSTALL_DIR) $(1)/etc/ssh
	chmod 0700 $(1)/etc/ssh
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/ssh/sshd_config $(1)/etc/ssh/
	sed -i 's,#PasswordAuthentication yes,PasswordAuthentication no,g' $(1)/etc/ssh/sshd_config
	sed -i 's,#UsePAM no,UsePAM yes,g' $(1)/etc/ssh/sshd_config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/sshd.init $(1)/etc/init.d/sshd
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openssh $(1)/usr/sbin/
	$(LN) openssh $(1)/usr/sbin/sshd
	$(INSTALL_DIR) $(1)/usr/bin
	$(LN) ../sbin/openssh $(1)/usr/bin/ssh-add
	$(LN) ../sbin/openssh $(1)/usr/bin/ssh-agent
	$(LN) ../sbin/openssh $(1)/usr/bin/ssh-keygen
	$(LN) ../sbin/openssh $(1)/usr/bin/ssh-keyscan
	$(LN) ../sbin/openssh $(1)/usr/bin/ssh-keysign
	$(LN) ../sbin/openssh $(1)/usr/bin/ssh-pkcs11-helper
	$(LN) ../sbin/openssh $(1)/usr/bin/ssh
	$(LN) ../sbin/openssh $(1)/usr/bin/scp
	$(LN) ../sbin/openssh $(1)/usr/bin/sftp
	$(LN) ../sbin/openssh $(1)/usr/bin/sftp
	$(INSTALL_DIR) $(1)/usr/lib
	$(LN) ../sbin/openssh $(1)/usr/lib/sftp-server
	$(INSTALL_DIR) $(1)/usr/libexec
	$(LN) ../sbin/openssh $(1)/usr/lib/sftp-server
endef

$(eval $(call BuildPackage,openssh-multicall))
