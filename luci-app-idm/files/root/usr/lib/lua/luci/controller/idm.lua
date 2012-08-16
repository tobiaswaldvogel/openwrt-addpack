require "lualdap"
require "math"

module("luci.controller.idm", package.seeall)

attr_group = { "cn", "gidNumber", "description" }
file_status  = "/var/run/idm_init_status"
file_running = "/var/run/idm_init_running"
file_error   = "/var/run/idm_init_error"
file_stdout  = "/var/run/idm_init_stdout"
state = ""

function index()
	local page
	local refresh = nixio.fs.readfile("/var/run/idm_init_running")

	if nixio.fs.access("/var/run/ldapi") and not (refresh == "y") then
		entry({"admin", "identities"}, alias("admin", "identities", "user"), _("Identities"), 80).index = true
		page = entry({"admin", "identities", "user" }, call("action_user"), _("User and Groups"), 10)
		page.i18n = "idm"
		page.dependent = true
	else
		entry({"admin", "identities"},  alias("admin", "identities", "init"), _("Identities"), 80).index = true
	end

	page = entry({"admin", "identities", "init" }, call("action_init"), _("Initialize"), 20)
	page.i18n = "idm"
	page.dependent = true	
	entry({"admin", "identities", "init", "status" }, call("action_status"))
end

function action_user()
	local action = luci.http.formvalue("act") or "none"
	local object = luci.http.formvalue("obj") or "none"

	nixio.fs.writefile("/tmp/luadump", action .. "\n" .. object .. "\n")

	if action == "none" then
	elseif action == "grprm" then
		objrm(object)
	elseif action == "usrrm" then
		objrm(object)
	elseif action == "grpmodok" then
		grpmod(object, luci.http.formvalue("gid"), luci.http.formvalue("desc"))
	elseif action == "grpaddok" then
		grpadd(luci.http.formvalue("cn"), luci.http.formvalue("gid"), luci.http.formvalue("desc"))
	elseif action == "usrmodok" then
		usrmod(object, luci.http.formvalue("gn"), luci.http.formvalue("sn"),
			luci.http.formvalue("uidn"), luci.http.formvalue("gid"),
			luci.http.formvalue("kpn"), luci.http.formvalue("pw"),
			luci.http.formvalue("homedir"), luci.http.formvalue("shell"))
	elseif action == "usraddok" then
		usradd(luci.http.formvalue("uid"), luci.http.formvalue("gn"),
			luci.http.formvalue("sn"), luci.http.formvalue("uidn"),
			luci.http.formvalue("gid"), luci.http.formvalue("pw"),
			luci.http.formvalue("homedir"), luci.http.formvalue("shell"))
	else
		luci.template.render("idm/idm_main", {act=action, obj=object})
		return
	end

	luci.template.render("idm/idm_main", {act="none", obj="none"})
end

function action_init()
	math.randomseed(os.time())
	local init_state = luci.http.formvalue("init_state") or "none"
	local dns_domain = luci.http.formvalue("dns_domain") or "local.net"
	local domain = luci.http.formvalue("domain") or dns_domain
	local basedn = luci.http.formvalue("basedn") or "dc=" .. dns_domain:gsub("[\.]", ",dc=")
	local ldappw = luci.http.formvalue("ldappw") or rnd_pw(12)
	local run

	domain = domain:upper()

	if init_state == "run" then
		status = "Setting up identity management ...\n&nbsp;\n"
		run = true
		init_state = "refresh"
		nixio.fs.writefile(file_running, "y")
		nixio.fs.writefile(file_status, status)
	elseif init_state == "refresh" then
		status = nixio.fs.readfile(file_status)
		if not (nixio.fs.readfile(file_running) == "y") then
			init_state = "finish"
		end
	end

	luci.template.render("idm/idm_init", {init_state=init_state, act=action, dns_domain=dns_domain, domain=domain, basedn=basedn, ldappw=ldappw, status=status})
	if not run then return end

        local pid, i, o, s, l, rc

        pid = nixio.fork()
        if pid > 0 then return end

	i = nixio.open("/dev/null", "r")
	o = nixio.open("/dev/null", "w")
	s = nixio.open(file_error, "w")
        nixio.dup(i, nixio.stdin)
        nixio.dup(o, nixio.stdout)
	nixio.dup(s, nixio.stderr)
        i:close()
	o:close()
	s:close()

	l = nixio.open(file_status, "a")
	status,rc = pcall(do_init, l, dns_domain, domain, basedn, ldappw)
	if not status then
		local e = nixio.fs.readfile(file_error)

		l:write("\n&nbsp;\nException occurred\n")
		if e then l:write(e) end
		rc = 1
	end

	if rc == 0 then
		 l:write("\n&nbsp;\nIdentity management setup completed\n")
	else
		 l:write("\n&nbsp;\nIdentity management setup failed\n")
	end

	l:close()
	nixio.fs.writefile(file_running, "n")
end

function do_init(l, dns_domain, domain, basedn, ldappw)
        local rc, msg, rdn, c
	local host = luci.sys.hostname():gsub("[\.].*", "")

	dc = dns_domain:gsub("[\.](.*)", "")

	l:write("Stopping LDAP ... ")
	os.execute("/etc/init.d/ldap stop")
	l:write("done\n")

	l:write("Stopping Kerberos KDC ... ")
	os.execute("/etc/init.d/krb5kdc stop")
	l:write("done\n")

	l:write("Cleanup LDAP data ... ")
	os.execute("rm -rf /overlay/etc/openldap/data/*")
	l:write("done\n")

	l:write("Configuring LDAP ... ")
	os.execute("touch /etc/config/ldap")
	c = luci.model.uci.cursor()
	c:section("ldap", "server", host, { enabled="1", serverid="1", basedn=basedn })
	c:save("ldap")
	c:commit("ldap")
	l:write("done\n")

	l:write("Starting LDAP ... ")
	luci.sys.exec("/etc/init.d/ldap start")
	l:write("done\nConnecting ... ")
	rc,msg,ld = lualdap.open("ldapi:///", "EXTERNAL")
	l:write(msg .. "\n")
	if not rc == 0 then return 1 end

	l:write("Adding " .. basedn .. " ... ")
	rc,msg = ld:add (basedn, { objectClass = { "dcObject", "organization" }, o = "root", dc = dc })
	l:write(msg .. "\n")
	if not rc == 0 then return 1 end

	groupou = "ou=Groups," .. basedn
	l:write("Adding " .. groupou .. " ... ")
	rc, msg = ld:add (groupou, { objectClass = "organizationalUnit" })
	l:write(msg .. "\n")
	if not rc == 0 then return 1 end

	userou= "ou=Users," .. basedn
	l:write("Adding " .. userou .. " ... ")
	rc, msg = ld:add (userou, { objectClass = "organizationalUnit" })
	l:write(msg .. "\n")
	if not rc == 0 then return 1 end

	compou = "ou=Computers," .. basedn
	l:write("Adding " .. compou .. " ... ")
	rc, msg = ld:add (compou, { objectClass = "organizationalUnit" })
	l:write(msg .. "\n")
	if not rc == 0 then return 1 end

	srvuserou = ("ou=System Users," .. basedn)
	l:write("Adding " .. srvuserou .. " ... ")
	rc, msg = ld:add (srvuserou, { objectClass = "organizationalUnit" })
	l:write(msg .. "\n")
	if not rc == 0 then return 1 end

	adm_dn = "cn=Manager," .. srvuserou
	l:write("Adding " .. adm_dn .. " ... ")
	rc, msg = ld:add (adm_dn, { objectClass = "inetOrgPerson", sn = "LDAP Manager" })
	l:write(msg .. "\n")
	if not rc == 0 then return 1 end
	l:write("Setting password ... ")
	rc, msg = ld:modify(adm_dn, { '=', userPassword = luci.sys.exec("slappasswd -s " .. ldappw) })
	l:write(msg .. "\n")
	if not rc == 0 then return 1 end

	dn = "cn=users," .. groupou
	l:write("Adding " .. dn .. " ... ")
	rc, msg = ld:add (dn, { objectClass = {"top", "posixGroup"}, gidNumber = 1000, description = "Users" })
	l:write(msg .. "\n")
	if not rc == 0 then return 1 end

	dn = "cn=computers," .. groupou
	l:write("Adding " .. dn .. " ... ")
	rc, msg = ld:add (dn, { objectClass = {"top", "posixGroup"}, gidNumber = 100, description = "Computers" })
	l:write(msg .. "\n")
	if not rc == 0 then return 1 end

	krb_container_dn = "cn=Kerberos," .. basedn
	l:write("Creating Kerberos Realm container " .. krb_container_dn ..  " ... ")
	rc, msg = ld:add (krb_container_dn, { objectClass = "krbContainer" })
	l:write(msg .. "\n")
	if not rc == 0 then return 1 end

	l:write("Configuring Kerberos ... ")
	nixio.fs.writefile("/etc/config/krb5", "")
        c = luci.model.uci.cursor()
        c:section("krb5", "realm", domain:gsub("[\.]", "_"), { name=domain, kdc= {host}, kadmind={host}, ldap={"ldapi:///"} })
        c:save("krb5")
        c:commit("krb5")
	os.execute("/etc/init.d/krb5conf start")
        l:write("done\n")

	initcmd = "kdb5_ldap_util -H ldapi:/// create -r " .. domain .. " -containerref \"" .. basedn .. "\" -s -P " .. rnd_pw(20)
	l:write("Initializing Kerberos Realm " .. domain .. " with  " .. initcmd .. "\n")
	l:write(luci.sys.exec(initcmd))

	l:write("Starting Kerberos KDC ... ")
	os.execute("/etc/init.d/krb5kdc start")
	l:write("done\n")

	l:write("Restarting Samba to create domain object ... ")
	os.execute("/etc/init.d/samba restart")
	l:write("done\n")

	l:write("Creating machine account for " .. host .. " ... ")
	rc,msg = hostadd(host, dns_domain, 100, 100, l)
	l:write(msg .. "\n")
	if not rc == 0 then return 1 end

	l:write("Restarting Samba ... ")
	os.execute("/etc/init.d/samba restart")
	l:write("done\n")
	return 0
end

function get_defaultNamingContext(ld)
	local dn, attrs

	for dn, attrs in ld:search { attrs = { "namingContexts" }, scope="b" } do
		return attrs["namingContexts"];
	end
end

function get_ou(ld, name)
	local dn, attrs

	for dn, attrs in ld:search { attrs = { "ou" }, filter = "ou=" .. name, scope = "s" } do
		return  dn
	end
	return get_defaultNamingContext(ld)
end

function get_realm(ld)
	local dn, attrs

	for dn, attrs in ld:search { attrs = { "cn" }, base = basedn, filter="objectClass=krbRealmContainer", scope="s" } do
		return attrs["cn"]
	end
end

function get_sambaSID(ld)
        local dn, attrs

        for dn, attrs in ld:search { attrs = { "sambaSID", "sambaDomainName" }, base = basedn, filter="objectClass=sambaDomain", scope="s" } do
                return attrs["sambaSID"],attrs["sambaDomain"]
        end
end

function objrm(dn)
        rc,msg,ld = lualdap.open("ldapi:///", "EXTERNAL")

        if not rc == 0 then return end

	ld:delete(dn)	
end

function grpmod(dn, gid, desc)
	local rc,msg,ld = lualdap.open("ldapi:///", "EXTERNAL")

	if not rc == 0 then return end
	rc, msg = ld:modify(dn, { '=', gidNumber = gid, description = desc })
end

function grpadd(cn, gid, desc)
        local rc,msg,ld = lualdap.open("ldapi:///", "EXTERNAL")
	local ou = get_ou(ld, "Groups")

        if not rc == 0 then return end

        rc, msg = ld:add("cn=" .. cn .. "," .. ou, {
		objectClass = { "top", "posixGroup" },
		gidNumber = gid,
		description = desc
	})
end

function changepw(ld, dn, kpn, uid, pw)
	local rc, msg

	--[[ Kerberos PW ]]--
	luci.sys.exec("kadmin.local -q \"cpw -pw " .. pw .. " " .. kpn .. "\"")

	--[[ Samba pw ]]--
	tmppwfile = "/tmp/userpw"
	nixio.fs.writefile(tmppwfile, pw .. "\n" .. pw .. "\n")
	smbpwadd = "smbpasswd -s -a " .. uid .. "<" .. tmppwfile
	luci.sys.exec(smbpwadd)
	nixio.fs.remove(tmppwfile)

	 --[[ LDAP pw ]]--
	rc, msg = ld:modify(dn, { '=', userPassword = luci.sys.exec("slappasswd -s " .. pw) })
end

function usrmod(dn, gn, sn, uidn, gid, kpn, pw, homedir, shell)
	local rc,msg,ld = lualdap.open("ldapi:///", "EXTERNAL")

	if not rc == 0 then return end
	rc, msg = ld:modify(dn, { '=',
		givenName = gn or "",
		sn = sn or "",
		uidNumber = uidn,
		gidNumber = gid,
		krbPrincipalName = kpn,
		homeDirectory = homedir,
		loginShell = shell
	})

	if pw:len() > 0 then
		local uid

		for dn, attrs in ld:search { attrs = { "uid" }, base = dn, scope="b" } do
			uid = attrs["uid"]
		end
		if uid then
			changepw(ld, dn, kpn, uid, pw)
		end
	end
end

function usradd(uid, gn, sn, uidn, gid, pw, homedir, shell)
	local rc,msg,ld = lualdap.open("ldapi:///", "EXTERNAL")

	if not rc == 0 then return end

	local ou = get_ou(ld, "Users")
	local realm = get_realm(ld)
	local dn = "uid=" .. uid .. "," .. ou
	local kpn = uid .. "@" .. realm
	local attrs = {}

	attrs.objectClass	= { "inetOrgPerson", "posixAccount", "krbPrincipalAux" }
	attrs.cn		= uid
	attrs.uidNumber		= uidn
	attrs.gidNumber		= gid
	if gn and not (gn == "")  then attrs.givenName = gn end
	if (not sn) or (sn == "") then sn = uid end
	attrs.sn		= sn
	attrs.homeDirectory	= homedir
	attrs.loginShell	= shell
	attrs.krbPrincipalName	= kpn


	rc,msg = ld:add (dn, attrs)
	if (rc == 0) then
		changepw(ld, dn, kpn, uid, pw)
	end
end

function hostadd(host, domain, uidn, gid, l)
	local rc,msg,ld = lualdap.open("ldapi:///", "EXTERNAL")

	if not (rc == 0) then return rc,msg end

	local ou = get_ou(ld, "Computers")
	local realm = get_realm(ld)
	local sid,smb_domain = get_sambaSID(ld)
	local dn = "uid=" .. host .. "$," .. ou
	local hostl = host:lower()
	local hostu = host:upper()
	local domainl = domain:lower()
	local domainu = domain:upper()
	local keytab  = "/etc/krb5.keytab"
	local kpn     = "ldap/" .. hostl .. "." .. domainl .. "@" .. realm
	local cmd

	rc,msg = ld:add(dn, {
			objectClass      = { "top", "account", "posixAccount", "krbPrincipalAux", "sambaSAMAccount" },
			cn		 = host .. "$",
			uid		 = host .. "$",
			uidNumber	 = uidn,
			gidNumber	 = gid,
			homeDirectory	 = "/dev/null",
			sambaSID	 = sid,
			sambaDomainName	 = smb_domain,
			krbPrincipalName = { kpn,
				"host/" .. hostl .. "." .. domainl .. "@" .. realm,
				"cifs/" .. hostl .. "." .. domainl .. "@" .. realm,
				"cifs/" .. hostu .. "." .. domainl .. "@" .. realm,
				"cifs/" .. hostl .. "." .. domainu .. "@" .. realm,
				"cifs/" .. hostu .. "." .. domainu .. "@" .. realm,
				"cifs/" .. hostl .. "@" .. realm,
				"cifs/" .. hostu .. "@" .. realm,
			}
		})

	os.execute("rm " .. keytab)
	cmd = "kadmin.local -q \"ktadd -k " .. keytab .. " " .. kpn .. "\""
	rc = os.execute(cmd)
	if not rc == 0 then
		msg = "Failed to execute: " .. cmd
	end

	return rc,msg
end

function concat_kpn(kpn)
	if not (type(kpn) == "table") then
		return kpn
	end

	local concat

	for key,value in pairs (kpn) do
		if not concat then
			concat = value
		else
			concat = concat .. "<br />" .. value
		end
	end
	return concat
end

function grouplist(callback)
	local rc,msg,ld = lualdap.open("ldapi:///", "EXTERNAL")

	if not rc == 0 then return end

	for dn, attrs in ld:search { attrs = attr_group, filter="objectClass=posixGroup", scope="s" } do
		callback({
			dn   = dn, 
			cn   = attrs[attr_group[1]],
			gid  = tonumber(attrs[attr_group[2]]),
			desc = attrs[attr_group[3]],
		})
	end
end

function userlist(callback)
	local attr_user = { "uid","givenName","sn", "uidNumber", "gidNumber", "krbPrincipalName","homeDirectory", "loginShell" }
	local groups = {}
	local rc,msg,ld = lualdap.open("ldapi:///", "EXTERNAL")

	if not rc == 0 then return end

	for dn, attrs in ld:search { attrs = attr_group, filter="objectClass=posixGroup", scope="s" } do
		local gid  = tonumber(attrs[attr_group[2]])
		local desc = attrs[attr_group[3]]
		groups[gid] = desc .. " (" .. gid .. ")"
	end

	for dn, attrs in ld:search { attrs = attr_user, filter="(&(objectClass=inetOrgPerson)(uid=*))", scope="s" } do
		callback({
			dn = dn,
			uid = attrs[attr_user[1]],
			gn =  attrs[attr_user[2]],
			sn =  attrs[attr_user[3]],
			uidn = tonumber(attrs[attr_user[4]]),
			gid = tonumber(attrs[attr_user[5]]),
			kpn = concat_kpn(attrs[attr_user[6]]),
			homedir = attrs[attr_user[7]],
			shell = attrs[attr_user[8]],
			groups = groups
		})

	end
end

function hostlist(callback)
	local attr_host  = { "uid", "description", "uidNumber", "gidNumber", "krbPrincipalName" }
	local groups = {}
	local gid, desc
	local rc,msg,ld = lualdap.open("ldapi:///", "EXTERNAL")

	if not rc == 0 then return end

	for dn, attrs in ld:search { attrs = attr_group, filter="objectClass=posixGroup", scope="s" } do
		gid  = tonumber(attrs[attr_group[2]]);
		desc = attrs[attr_group[3]];
		groups[gid] = desc .. " (" .. gid .. ")"
	end

	for dn, attrs in ld:search { attrs = attr_host, filter="(&(!(objectClass=inetOrgPerson))(uid=*))", scope="s" } do
		local name = attrs[attr_host[1]]
		local gid  = tonumber(attrs[attr_host[4]])

		if name:sub(-1) == "$" then
			name = name:sub(1,-2)
		end

		callback ({
			uid    = name,
			desc   = attrs[attr_host[2]],
			uidn   = attrs[attr_host[3]],
			gid    = gid,
			kpn    = concat_kpn(attrs[attr_host[5]]),
			groups = groups
		})
	end
end

function rnd_pw(len)
	local pass = ""
	local c

	repeat
		repeat
			c = string.format("%c",math.random(1,4096))
		until string.match(c,"%w")
		pass = pass .. c
	until string.len(pass) >= len
	return pass
end

