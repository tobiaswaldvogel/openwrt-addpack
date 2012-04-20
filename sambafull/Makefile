include $(TOPDIR)/rules.mk

PKG_NAME:=samba
PKG_VERSION:=3.6.4
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://samba.org/samba/ftp/stable \
    ftp://se.samba.org/pub/samba/stable \
    ftp://ftp.easynet.be/samba/stable \
    http://us1.samba.org/samba/ftp/stable
PKG_SOURCE:=samba-$(PKG_VERSION).tar.gz
PKG_BUILD_DEPENDS:=+libpopt +libuuid +libopenldap +krb5-libs +libiconv +libacl +libreadline

include $(INCLUDE_DIR)/package.mk

define Package/sambafull/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Filesystem
  URL:=http://www.samba.org/
endef

define Package/sambafull/description
 The Samba software suite is a collection of programs that implements the
 SMB protocol for unix systems, allowing you to serve files and printers to
 Windows, NT, OS/2 and DOS clients. This protocol is sometimes also referred
 to as the LanManager or Netbios protocol.
endef

define Package/sambafull
  $(call Package/sambafull/Default)
  TITLE:=NetBIOS/SMB server and client shared files
  DEPENDS:=+libgcc +libpopt +libuuid +libopenldap +krb5-libs +libiconv +libreadline
endef

define Package/sambafull-server
  $(call Package/sambafull/Default)
  TITLE:=NetBIOS/SMB file and print server
  DEPENDS:=sambafull +sambafull +sambafull-libs 
endef

define Package/sambafull-client
  $(call Package/sambafull/Default)
  TITLE:=NetBIOS/SMB simple client
  DEPENDS:=sambafull +sambafull
endef

define Package/sambafull-libs
  $(call Package/sambafull/Default)
  TITLE:=NetBIOS/SMB libraries for client programs
  DEPENDS:=sambafull +sambafull
endef

define Package/sambafull-scan
  $(call Package/sambafull/Default)
  TITLE:=Automount for samba shares
  DEPENDS:=sambafull +sambafull +sambafull-client +nmap
endef

CONFIGURE_PATH := source3
MAKE_PATH := source3
EXTRA_LDFLAGS += -L$(STAGING_DIR)/usr/lib/libiconv/lib \
		 -Wl,-rpath-link=$(STAGING_DIR)/usr/lib -lkrb5 -lcom_err -ldl
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/krb5

define Build/Configure
	(cd $(PKG_BUILD_DIR)/source3; rm -rf config.{cache,status}; ./autogen.sh )
	$(call Build/Configure/Default, \
		--prefix=/usr \
		--libdir=/usr/lib \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--mandir=/usr/man \
		--sysconfdir=/etc \
		--with-configdir=/etc/samba \
		--with-piddir=/var/run \
		--with-privatedir=/etc/samba \
		--localstatedir=/var \
		--enable-shared \
		--with-krb5=$(STAGING_DIR)/usr \
		--with-libiconv=$(STAGING_DIR)/usr/lib/libiconv \
		--with-ldap \
		--with-ads \
		--with-lockdir=/var/lock \
		--with-logfilebase=/var/log/samba \
		--without-libaddns \
		--disable-avahi \
		--disable-cups \
		--disable-static \
		--enable-largefile \
		--with-acl-support \
		--with-quotas \
		--with-syslog \
		--without-winbind \
		--without-pam \
		--disable-swat \
		samba_cv_HAVE_SENDFILE=yes \
		samba_cv_HAVE_SENDFILE64=yes \
		samba_cv_HAVE_BROKEN_LINUX_SENDFILE=no \
		samba_cv_CC_NEGATIVE_ENUM_VALUES=yes \
		samba_cv_HAVE_GETTIMEOFDAY_TZ=yes \
		samba_cv_USE_SETREUID=yes \
		samba_cv_HAVE_KERNEL_OPLOCKS_LINUX=yes \
		samba_cv_HAVE_IFACE_IFCONF=yes \
		samba_cv_HAVE_WRFILE_KEYTAB=no \
		ac_cv_sizeof_off_t=4 \
		samba_cv_SIZEOF_OFF_T=4 \
		samba_cv_HAVE_OFF64_T=yes \
		samba_cv_have_longlong=yes \
		linux_getgrouplist_ok=no \
		libreplace_cv_HAVE_IPV6=no \
		libreplace_cv_HAVE_SECURE_MKSTEMP=yes \
		libreplace_cv_HAVE_IFACE_IFCONF=yes \
		smb_krb5_cv_enctype_to_string_takes_krb5_context_arg=no \
		smb_krb5_cv_enctype_to_string_takes_size_t_arg=yes \
		ac_cv_lib_ext_krb5_krb5_mk_req_extended=yes \
		ac_cv_path_KRB5CONFIG=$(STAGING_DIR)/usr \
		ac_cv_file__proc_sys_kernel_core_pattern=yes \
	)
endef

define Build/Compile
	$(call Build/Compile/Default, all)
endef

define Package/sambafull-server/conffiles
  /etc/samba
endef

define Package/sambafull/install
	$(INSTALL_DIR) $(1)/etc/samba
	$(INSTALL_DATA) ./files/smb.conf $(1)/etc/samba/smb.conf
endef

define Package/sambafull-libs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/source3/bin/libtalloc.so* $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/source3/bin/libtdb.so* $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/source3/bin/libsmb.so $(1)/usr/lib
endef

define Package/sambafull-server/install
	$(INSTALL_DIR) $(1)/etc/{samba,init.d,config}
	$(INSTALL_BIN) ./files/samba.init $(1)/etc/init.d/samba
	$(INSTALL_DATA) ./files/samba.config $(1)/etc/config/samba
	$(INSTALL_DATA) ./files/smb.conf.template $(1)/etc/samba/
	cd $(1)/etc/samba; rm smb.conf; ln -s /tmp/smb.conf smb.conf
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/source3/bin/smbpasswd.shared $(1)/usr/bin/smbpasswd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/source3/bin/smbcontrol.shared $(1)/usr/bin/smbcontrol
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/source3/bin/smbcquotas.shared $(1)/usr/bin/smbcquotas
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/source3/bin/smbstatus.shared $(1)/usr/bin/smbstatus
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/source3/bin/smbtree.shared $(1)/usr/bin/smbtree
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/source3/bin/testparm.shared $(1)/usr/bin/testparm
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/source3/bin/nmbd.shared $(1)/usr/sbin/nmbd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/source3/bin/smbd.shared $(1)/usr/sbin/smbd
endef

define Package/sambafull-client/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/source3/bin/smbclient $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/source3/bin/nmblookup $(1)/usr/bin
endef

$(eval $(call BuildPackage,sambafull))
$(eval $(call BuildPackage,sambafull-server))
$(eval $(call BuildPackage,sambafull-client))
$(eval $(call BuildPackage,sambafull-libs))
