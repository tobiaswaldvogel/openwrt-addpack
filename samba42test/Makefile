#
# Copyright (C) 2007-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=samba42test
PKG_VERSION:=4.2.1
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://ftp.samba.org/pub/samba \
	http://ftp.samba.org/pub/samba/old-versions
PKG_SOURCE:=samba-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/samba-$(PKG_VERSION)
PKG_MD5SUM:=

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_DEPENDS:=heimdal/host luci python
PO2LMO:=$(STAGING_DIR_HOST)/bin/po2lmo

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

#MAKE_PATH:=source3
#CONFIGURE_PATH:=source3

PKG_BUILD_BIN:=$(PKG_BUILD_DIR)/$(MAKE_PATH)/bin

define Package/samba42test-server
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Samba $(PKG_VERSION) SMB/CIFS server with ADS support
  URL:=http://www.samba.org/
  VARIANT:=samba42test-server
  DEPENDS:=+USE_EGLIBC:librt +libpopt +libuuid +libopenldap-sasl +krb5-mit-libs +krb5-mit-client +libiconv +libacl +libreadline +libaio +libtirpc +wsdd
endef

define Package/samba42test-client
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Samba $(PKG_VERSION) SMB/CIFS client
  URL:=http://www.samba.org/
  DEPENDS:=+libreadline +libncurses +libopenldap-sals +krb5-mit-libs +libaio +zlib
endef

define Package/samba42test-server/config
config PACKAGE_SAMBA42TEST_MAX_DEBUG_LEVEL
	int "Maximum level of compiled-in debug messages"
	depends on PACKAGE_samba42test-server || PACKAGE_samba42test-client
	default -1
endef

define Package/samba42-server/description
 The Samba software suite is a collection of programs that implements the
 SMB protocol for UNIX systems, allowing you to serve files and printers to
 Windows, NT, OS/2 and DOS clients. This protocol is sometimes also referred
 to as the LanManager or Netbios protocol.
endef

define Package/luci-app-$(PKG_NAME)
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  DEPENDS:=+samba42-server
  TITLE:=Samba 4
endef

TARGET_CFLAGS += \
	-DMAX_DEBUG_LEVEL=$(CONFIG_PACKAGE_SAMBA42TEST_MAX_DEBUG_LEVEL) \
	-I$(STAGING_DIR)/usr/include/krb5 \
	-DACTIVE_DIRECTORY \
	-DDFS_SUPPORT \
	-DNETLOGON_SUPPORT \
	-DSAMR_SUPPORT \
	-DLSA_SUPPORT \
	-D_GNU_SOURCE \
	-ffunction-sections -fdata-sections

CONFIGURE_VARS += \
	PYTHON_CONFIG="$(STAGING_DIR)/usr/bin/python-config" \
	python_LDFLAGS="" \
	python_LIBDIR="" \
	KRB5_CONFIG="$(STAGING_DIR)/usr/bin/krb5-config"

SAMBA4_CONFIGURE_ARGS = \
	--disable-rpath \
	--cross-compile \
	--cross-answers=$(PKG_BUILD_DIR)/cache.txt \
	--disable-rpath \
	--disable-rpath-install \
	--exec-prefix=/usr \
	--prefix=/ \
	--disable-avahi \
	--disable-cups \
	--disable-iprint \
	--disable-gnutls \
	--with-relro \
	--without-pie \
	--with-configdir=/etc/samba \
	--with-lockdir=/var/lock \
	--with-logfilebase=/var/log \
	--with-piddir=/var/run \
	--with-privatedir=/etc/samba \
	--with-sendfile-support \
	--with-aio-support \
	--without-cluster-support \
	--without-pam \
	--without-winbind \
	--without-dnsupdate \
	--with-ads \
	--with-ldap \
	--with-libiconv=$(STAGING_DIR)/usr/lib/libiconv \
	--with-acl-support \
	--with-quotas \
	--with-syslog \
	--with-system-mitkrb5 \
	--with-shared-modules=pdb_tdbsam,pdb_wbc_sam,idmap_nss,nss_info_template,vfs \
	--with-static-modules=vfs_default,vfs_dirsort,vfs_posixacl \
	--bundled-libraries='!asn1_compile,!compile_et'

#	--builtin-libraries= \


CROSS = " \
	\nChecking simple C program: OK \
	\nrpath library support: OK \
	\n-Wl,--version-script support: OK \
	\nChecking getconf LFS_CFLAGS: NO \
	\nChecking for large file support without additional flags: OK \
	\nChecking for -D_LARGE_FILES: OK \
	\nChecking correct behavior of strtoll: NO \
	\nChecking for working strptime: OK \
	\nChecking for C99 vsnprintf: OK \
	\nChecking for HAVE_SHARED_MMAP: OK \
	\nChecking for HAVE_MREMAP: OK \
	\nChecking for HAVE_INCOHERENT_MMAP: NO \
	\nChecking for HAVE_SECURE_MKSTEMP: OK \
	\nChecking for HAVE_IFACE_GETIFADDRS: OK \
	\nChecking for kernel change notify support: OK \
	\nChecking for Linux kernel oplocks: OK \
	\nChecking for kernel share modes: OK \
	\nChecking if can we convert from CP850 to UCS-2LE: OK \
	\nChecking if can we convert from UTF-8 to UCS-2LE: OK \
	\nChecking whether we can use Linux thread-specific credentials with 32-bit system calls: OK \
	\nChecking whether we can use Linux thread-specific credentials: OK \
	\nChecking whether setreuid is available: OK \
	\nChecking whether setresuid is available: OK \
	\nChecking whether seteuid is available: OK \
	\nChecking whether fcntl locking is available: OK \
	\nChecking for the maximum value of the 'time_t' type: OK \
	\nChecking whether the realpath function allows a NULL argument: OK \
	\nChecking whether POSIX capabilities are available: OK \
	\nChecking for ftruncate extend: OK \
	\nvfs_fileid checking for statfs() and struct statfs.f_fsid: OK \
	\ngetcwd takes a NULL argument: OK \
	\nChecking value of NSIG: \"65\" \
	\nChecking value of _NSIG: \"65\" \
	\nChecking value of SIGRTMAX: \"64\" \
	\nChecking value of SIGRTMIN: \"34\" \
	\nChecking whether the WRFILE-keytab is supported: OK \
	\n"

define Build/Configure
	(cd $(PKG_BUILD_DIR); \
		echo -e >cache.txt $(CROSS) " \
			\nChecking uname release type: \"$(LINUX_VERSION)\" \
			\nChecking uname machine type: \"$(ARCH)\" \
			\nChecking uname version type: \"#1 Tue Nov 1 12:34:56 UTC 2013\" \
			\nChecking uname sysname type: \"Linux\" \
			\n" ; \
		$(CONFIGURE_VARS) \
		./buildtools/bin/waf configure \
		$(SAMBA4_CONFIGURE_ARGS) \
	)
endef

define Package/samba42test-server/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_BIN)/$(PKG_NAME)_multicall $(1)/usr/sbin
	ln -sf $(PKG_NAME)_multicall $(1)/usr/sbin/smbd
	ln -sf $(PKG_NAME)_multicall $(1)/usr/sbin/nmbd
	ln -sf $(PKG_NAME)_multicall $(1)/usr/sbin/smbpasswd
	$(INSTALL_DIR) $(1)/usr/bin
	ln -sf ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbcontrol
	ln -sf ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbtree
	ln -sf ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbstatus
	ln -sf ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbcquotas
	ln -sf ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbcacls
	ln -sf ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/nmblookup
	ln -sf ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/testparm
	$(INSTALL_DIR) $(1)/etc/openldap/schema
	$(CP) $(PKG_BUILD_DIR)/examples/LDAP/samba.schema $(1)/etc/openldap/schema
	$(INSTALL_DIR) $(1)/etc/samba
	ln -sf /var/etc/secrets.tdb $(1)/etc/samba/secrets.tdb
	ln -sf /var/etc/schannel_store.tdb $(1)/etc/samba/schannel_store.tdb
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/$(PKG_NAME).config $(1)/etc/config/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/samba
	$(INSTALL_DATA) ./files/smb.conf.template $(1)/etc/samba
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/codepages/lowcase.dat $(1)/etc/samba
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/codepages/upcase.dat $(1)/etc/samba
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/codepages/valid.dat $(1)/etc/samba
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
endef

define Package/samba42test-server/conffiles
/etc/samba/smb.conf.template
endef

define Package/samba42test-client/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_BIN)/smbclient $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_BIN)/nmblookup $(1)/usr/sbin
endef

define Package/luci-app-$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) files/$(PKG_NAME).controller.lua $(1)/usr/lib/lua/luci/controller/$(PKG_NAME).lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DATA) files/$(PKG_NAME).cbi.lua $(1)/usr/lib/lua/luci/model/cbi/$(PKG_NAME).lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	for transl in files/*.po; do \
		transl_dest=`basename $$$$transl .po`.lmo; \
		$(PO2LMO) $$$$transl $(1)/usr/lib/lua/luci/i18n/$$$$transl_dest; \
	done
endef

$(eval $(call BuildPackage,$(PKG_NAME)-client))
$(eval $(call BuildPackage,$(PKG_NAME)-server))
$(eval $(call BuildPackage,luci-app-$(PKG_NAME)))
