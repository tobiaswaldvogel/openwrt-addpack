#
# Copyright (C) 2007-2015 wongsyrone
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=samba43
PKG_VERSION:=4.3.1
PKG_RELEASE:=2

PKG_SOURCE_URL:=http://ftp.samba.org/pub/samba \
	http://ftp.samba.org/pub/samba/old-versions
PKG_SOURCE:=samba-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/samba-$(PKG_VERSION)
PKG_MD5SUM:=

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_DEPENDS:=python-light libpopt heimdal/host

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
# for "uname release type" check
include $(INCLUDE_DIR)/kernel.mk


define Package/samba43-server
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Samba $(PKG_VERSION) SMB/CIFS server
  URL:=http://www.samba.org/
  DEPENDS:=+zlib +libpthread +libpopt +PACKAGE_samba43-server_asyncio:libaio
#DEPENDS+= +libreadline +libuuid +libtirpc libtirpc
endef

define Package/samba43-server/description
 The Samba software suite is a collection of programs that implements the
 SMB protocol for UNIX systems, allowing you to serve files and printers to
 Windows, NT, OS/2 and DOS clients. This protocol is sometimes also referred
 to as the LanManager or Netbios protocol.
endef

define Package/samba43-server/config
	if PACKAGE_samba43-server
	config PACKAGE_samba43-server_asyncio
		bool "Build with Asynchronous I/O support."
		select KERNEL_AIO
		default n
	endif
endef

TARGET_CFLAGS += \
	-D_GNU_SOURCE \
	-ffunction-sections -fdata-sections

TARGET_LDFLAGS += -Wl,--gc-sections

# Use host compiled python2 as interpreter
# Use target python2-config to retrive cflags, ldflags, libs, etc.
# Must have python2 patched to make python2-config works
#	PYTHON="$(STAGING_DIR_HOST)/bin/python2" \
#	PYTHON_CONFIG="$(STAGING_DIR)/usr/bin/python2-config" \
#	PYTHON_CONFIG="$(STAGING_DIR)/usr/bin/python-config" \
CONFIGURE_VARS += \
	PYTHON="$(STAGING_DIR_HOST)/bin/python2" \
	PYTHON_CONFIG="$(STAGING_DIR)/usr/bin/python2-config" \
	python_LDFLAGS="" \
	python_LIBDIR=""

# Taken from package-defaults.mk
SAMBA_CONFIGURE_ARGS += \
		--target=$(GNU_TARGET_NAME) \
		--program-prefix="" \
		--prefix=$(CONFIGURE_PREFIX) \
		--exec-prefix=$(CONFIGURE_PREFIX) \
		--bindir=$(CONFIGURE_PREFIX)/bin \
		--sbindir=$(CONFIGURE_PREFIX)/sbin \
		--libexecdir=$(CONFIGURE_PREFIX)/lib \
		--sysconfdir=/etc \
		--datadir=$(CONFIGURE_PREFIX)/share \
		--localstatedir=/var \
		--mandir=$(CONFIGURE_PREFIX)/man \
		--infodir=$(CONFIGURE_PREFIX)/info

SAMBA43_CACHE := " \
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
	\nChecking whether the realpath function allows a NULL argument: OK \
	\nChecking whether POSIX capabilities are available: OK \
	\nChecking for the maximum value of the 'time_t' type: NO \
	\nChecking for ftruncate extend: OK \
	\nvfs_fileid checking for statfs() and struct statfs.f_fsid: OK \
	\ngetcwd takes a NULL argument: OK \
	\nChecking value of NSIG: \"65\" \
	\nChecking value of _NSIG: \"65\" \
	\nChecking value of SIGRTMAX: \"64\" \
	\nChecking value of SIGRTMIN: \"34\" \
	\n"
# 	\nChecking for the maximum value of the 'time_t' type: OK  <<-- check it, not for sure. It seems musl-libc don't have max time_t

define Build/Configure
	(cd $(PKG_BUILD_DIR); \
		echo -e >cache.txt $(SAMBA43_CACHE) " \
			\nChecking uname release type: \"$(LINUX_VERSION)\" \
			\nChecking uname machine type: \"$(ARCH)\" \
			\nChecking uname version type: \"#1 Tue Nov 1 12:34:56 UTC 2013\" \
			\nChecking uname sysname type: \"Linux\" \
			\n" ; \
		$(CONFIGURE_VARS) \
		./buildtools/bin/waf configure -j$(shell nproc) \
			--cross-compile \
			--cross-answers=$(PKG_BUILD_DIR)/cache.txt \
			--hostcc=$(HOSTCC) \
			$(SAMBA_CONFIGURE_ARGS) \
			--with-lockdir=/var/lock \
			--with-logfilebase=/var/log \
			--with-piddir=/var/run \
			--with-privatedir=/etc/samba \
			--enable-fhs \
			--without-dmapi \
			--disable-glusterfs \
			--disable-rpath \
			--disable-rpath-install \
			--disable-avahi \
			--disable-cups \
			--disable-iprint \
			--disable-gnutls \
			--with-relro \
			--with-sendfile-support \
			--without-acl-support \
			$(if $(CONFIG_PACKAGE_samba43-server_asyncio),--with-aio-support,--without-aio-support) \
			--without-ads \
			--without-ad-dc \
			--without-automount \
			--without-cluster-support \
			--without-dnsupdate \
			--without-fam \
			--without-gettext \
			--without-iconv \
			--without-ldap \
			--without-pam \
			--without-pam_smbpass \
			--without-pie \
			--without-regedit \
			--without-systemd \
			--without-utmp \
			--without-winbind \
			--nonshared-binary=$(PKG_NAME)_multicall \
			--with-shared-modules=pdb_tdbsam,pdb_wbc_sam,idmap_nss,nss_info_template,auth_wbc \
			--without-quotas \
			--with-syslog \
			--nopyc \
			--nopyo \
	)
endef

# '--nonshared-binary' works like '--disable-shared-libs' in samba 4.0.26
# or use '--builtin-libraries' to specify which library to build into.
# --nonshared-binary=smbd/smbd,nmbd/nmbd,smbpasswd,smbcontrol,smbtree,smbstatus,smbcquotas,smbcacls,nmblookup,testparm,$(PKG_NAME)_multicall

# 'builtin-libraries'
# smbd_base samba-util tdb talloc param pdb
# seems don't need: krb5samba

# speed up
define Build/Compile
	(cd $(PKG_BUILD_DIR); \
		./buildtools/bin/waf build -j$(shell nproc) \
	)
endef

# remove some useless folders after install
define Build/Install
	$(call Build/Install/Default)
	rm -rf $(PKG_INSTALL_DIR)/var $(PKG_INSTALL_DIR)/etc $(PKG_INSTALL_DIR)/usr/lib/{pkgconfig,python2.7}
endef

# Make sure the name of multicall binary matches name in 111-multicall-wscript-build.patch
define Package/samba43-server/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/$(PKG_NAME).config $(1)/etc/config/samba4
	$(INSTALL_DIR) $(1)/etc/samba
	$(INSTALL_DATA) ./files/smb.conf.template $(1)/etc/samba
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/samba4
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/$(PKG_NAME)_multicall $(1)/usr/sbin
	$(LN) $(PKG_NAME)_multicall $(1)/usr/sbin/smbd
	$(LN) $(PKG_NAME)_multicall $(1)/usr/sbin/nmbd
	$(LN) $(PKG_NAME)_multicall $(1)/usr/sbin/smbpasswd
	$(INSTALL_DIR) $(1)/usr/bin
	$(LN) ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbcontrol
	$(LN) ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbtree
	$(LN) ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbstatus
	$(LN) ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbcquotas
	$(LN) ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbcacls
	$(LN) ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/nmblookup
	$(LN) ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/testparm
endef

define Package/samba43-server/conffiles
/etc/config/samba4
/etc/samba/smb.conf.template
/etc/samba/smbpasswd
endef


$(eval $(call BuildPackage,$(PKG_NAME)-server))
