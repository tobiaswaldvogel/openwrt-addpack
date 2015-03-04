require "lualdap"
require "math"
require "md4"
require "sha1"

local reqval = luci.http.formvalue

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

	luci.i18n.loadc("idm")
	if nixio.fs.access("/var/run/ldapi") and not (refresh == "y") then
		menu = entry({"admin", "identities"}, alias("admin", "identities", "user"), _("Identities"), 80)
		menu.i18n = "idm"
		menu.index = true
		page = entry({"admin", "identities", "user" }, call("action_user"), _("User and Groups"), 10)
		page.i18n = "idm"
		page.dependent = true
	else
		menu = entry({"admin", "identities"},  alias("admin", "identities", "init"), _("Identities"), 80)
		menu.i18n = "idm"
		menu.index = true
	end

	page = entry({"admin", "identities", "init" }, call("action_init"), _("Initialize"), 20)
	page.i18n = "idm"
	page.dependent = true	
	entry({"admin", "identities", "init", "status" }, call("action_status"))
end

function action_user()
	local cur_act = reqval("act") or "none"
	local cur_obj = reqval("obj") or "none"
	local new_act = "none"
	local new_obj = "none"
        local rc,msg,ld = lualdap.open("ldapi:///", "EXTERNAL")

	if cur_act == "none" then
	elseif (cur_act == "grprm") or (cur_act == "usrrm") or (cur_act == "hostrm") then
		ld:delete(cur_obj)
	elseif cur_act == "grpmodok" then
		grpmod(ld, cur_obj, reqval("gid"), reqval("desc"))
	elseif cur_act == "grpaddok" then
		grpadd(ld, reqval("cn"), reqval("gid"), reqval("desc"))
	elseif cur_act == "usrmodok" then
		usrmod(ld, cur_obj, reqval("gn"), reqval("sn"), reqval("uidn"), reqval("gid"), reqval("kpn"), reqval("pw"), reqval("homedir"), reqval("shell"))
	elseif cur_act == "usraddok" then
		usradd(ld, reqval("uid"), reqval("gn"), reqval("sn"), reqval("uidn"), reqval("gid"), reqval("pw"), reqval("homedir"), reqval("shell"))
	elseif cur_act == "hostmodok" then
		hostmod(ld, cur_obj, reqval("desc"), reqval("uidn"), reqval("gid"), reqval("pw"), reqval("kpn"))
	elseif cur_act == "hostaddok" then
		hostadd(ld, reqval("uid"), reqval("desc"), reqval("uidn"), reqval("gid"), reqval("pw"))
	else
		new_act = cur_act
		new_obj = cur_obj
	end

	local groups  = { }
	local grp_lbl = { }
	local gid, desc

	for dn, attrs in ld:search { attrs = attr_group, filter="objectClass=posixGroup", scope="s" } do
		gid  = tonumber(attrs[attr_group[2]])
		desc = attrs[attr_group[3]]

		groups[gid] = {
                        dn    = dn,
                        cn    = attrs[attr_group[1]],
                        desc  = desc,
		}

		grp_lbl[gid] = desc .. " (" .. gid .. ")"
	end

        local attr_host  = { "uid", "description", "uidNumber", "gidNumber", "krbPrincipalName" }
	local hosts = { }

	for dn, attrs in ld:search { attrs = attr_host, filter="(&(!(objectClass=inetOrgPerson))(uid=*))", scope="s" } do
		local name = attrs[attr_host[1]]
		local gid  = tonumber(attrs[attr_host[4]])

		if name:sub(-1) == "$" then
			name = name:sub(1,-2)
		end

		hosts[name] = {
                        dn = dn,
                        desc   = attrs[attr_host[2]],
                        uidn   = tonumber(attrs[attr_host[3]]),
                        gid    = tonumber(gid),
                        kpn    = attrs[attr_host[5]],
                }

	end	

	local attr_user = { "uid","givenName","sn", "uidNumber", "gidNumber", "krbPrincipalName","homeDirectory", "loginShell" }
	local users = { }

	for dn, attrs in ld:search { attrs = attr_user, filter="(&(objectClass=inetOrgPerson)(uid=*))", scope="s" } do
		users[attrs[attr_user[1]]] = {
                        dn = dn,
                        gn =  attrs[attr_user[2]],
                        sn =  attrs[attr_user[3]],
                        uidn = tonumber(attrs[attr_user[4]]),
                        gid = tonumber(attrs[attr_user[5]]),
                        kpn = attrs[attr_user[6]],
                        homedir = attrs[attr_user[7]],
                        shell = attrs[attr_user[8]],
                }
        end

	luci.template.render("idm/idm_main", {act=new_act, obj=new_obj, grp_lbl=grp_lbl, groups=groups, users=users, hosts=hosts})
end

function action_init()
	math.randomseed(os.time())
	local init_state = reqval("init_state") or "none"
	local dns_domain = reqval("dns_domain") or "home.net"
	local domain = reqval("domain") or dns_domain
	local basedn = reqval("basedn") or "dc=" .. dns_domain:gsub("[\.]", ",dc=")
	local ldappw = reqval("ldappw") or rnd_pw(12)
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
	local domainl = domain:lower()
	local domainu = domain:upper()
	local host = luci.sys.hostname():gsub("[\.].*", "")
	local hostl = host:lower()
	local hostu = host:upper()
	local keytab  = "/etc/krb5.keytab"
	local SID
	local tries

	dc = dns_domain:gsub("[\.](.*)", "")

	l:write("Stopping LDAP ... ")
	os.execute("/etc/init.d/ldap stop")
	l:write("done\n")

	l:write("Stopping Kerberos KDC ... ")
	os.execute("/etc/init.d/krb5kdc stop")
	l:write("done\n")

	l:write("Cleanup Kerberos and LDAP data ... ")
	os.execute("rm -rf /etc/openldap/data/*")
	os.execute("rm -f /etc/config/krb5")
	os.execute("rm -f /etc/config/ldap")
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
	tries = 5
	while tries > 0 do
		rc,msg,ld = lualdap.open("ldapi:///", "EXTERNAL")
		if rc == 0 then break end
		tries = tries - 1
		os.execute("sleep 1")
		if tries == 0 then
			l:write(msg .. "\n")
			return 1
		end
	end
	l:write(msg .. "\n")

	l:write("Adding " .. basedn .. " ... ")
	rc,msg = ld:add (basedn, { objectClass = { "dcObject", "organization" }, o = "root", dc = dc })
	l:write(msg .. "\n")
	if not (rc == 0) then return 1 end

	groupou = "ou=Groups," .. basedn
	l:write("Adding " .. groupou .. " ... ")
	rc, msg = ld:add (groupou, { objectClass = "organizationalUnit" })
	l:write(msg .. "\n")
	if not (rc == 0) then return 1 end

	userou= "ou=Users," .. basedn
	l:write("Adding " .. userou .. " ... ")
	rc, msg = ld:add (userou, { objectClass = "organizationalUnit" })
	l:write(msg .. "\n")
	if not (rc == 0) then return 1 end

	compou = "ou=Computers," .. basedn
	l:write("Adding " .. compou .. " ... ")
	rc, msg = ld:add (compou, { objectClass = "organizationalUnit" })
	l:write(msg .. "\n")
	if not (rc == 0) then return 1 end

	srvuserou = ("ou=System Users," .. basedn)
	l:write("Adding " .. srvuserou .. " ... ")
	rc, msg = ld:add (srvuserou, { objectClass = "organizationalUnit" })
	l:write(msg .. "\n")
	if not (rc == 0) then return 1 end

	adm_dn = "cn=Manager," .. srvuserou
	l:write("Adding " .. adm_dn .. " ... ")
	rc, msg = ld:add (adm_dn, { objectClass = "inetOrgPerson", sn = "LDAP Manager", userPassword = "{SHA}" .. base64(sha1.digest(ldappw, true)) })
	l:write(msg .. "\n")
	if not (rc == 0) then return 1 end

	krb_container_dn = "cn=Kerberos," .. basedn
	l:write("Creating Kerberos Realm container " .. krb_container_dn ..  " ... ")
	rc, msg = ld:add (krb_container_dn, { objectClass = "krbContainer" })
	l:write(msg .. "\n")
	if not (rc == 0) then return 1 end

	l:write("Configuring Kerberos ... ")
	nixio.fs.writefile("/etc/config/krb5", "")
        c = luci.model.uci.cursor()
        c:section("krb5", "realm", domain:gsub("[\.]", "_"), { name=domain, kdc={"localhost"}, kadmind={"localhost"}, ldap={"ldapi:///"},
		 enctype={"rc4-hmac:normal"}, master_key_type="rc4-hmac" })
        c:save("krb5")
        c:commit("krb5")
	os.execute("/etc/init.d/krb5conf start")
        l:write("done\n")

	initcmd = "kdb5_ldap_util -H ldapi:/// create -r " .. domain .. " -containerref \"" .. basedn .. "\" -s -P " .. rnd_pw(20)
	l:write(luci.sys.exec(initcmd))

	l:write("Starting Kerberos KDC ... ")
	os.execute("/etc/init.d/krb5kdc start")
	l:write("done\n")

	l:write("Restarting Samba to create domain object ... ")
	c = luci.model.uci.cursor()
	c:set("samba", c:get_first("samba", "samba"), "workgroup", domain)
	c:save("samba")
	c:commit("samba")
	os.execute("/etc/init.d/samba4 restart")

	tries = 10
	while tries > 0 do
		SID = get_sambaSID(ld)
		if SID ~= "" then break end
		os.execute("sleep 1")
		tries = tries - 1
		if tries == 0 then return 1 end
	end
	l:write("done\n")

	l:write("Creating well-known groups ... \n")

	dn = "cn=users," .. groupou
	l:write("&nbsp;&nbsp;Adding " .. dn .. " ... ")
	rc, msg = ld:add (dn, { objectClass = {"top", "posixGroup", "sambaGroupMapping"}, gidNumber = 100, sambaSID = SID .. "-513", sambaGroupType = 2, displayName = "Domain users", description = "Domain users" })
	l:write(msg .. "\n")
	if not (rc == 0) then return 1 end

	dn = "cn=computers," .. groupou
	l:write("&nbsp;&nbsp;Adding " .. dn .. " ... ")
	rc, msg = ld:add (dn, { objectClass = {"top", "posixGroup", "sambaGroupMapping"}, gidNumber = 102, sambaSID = SID .. "-515", sambaGroupType = 2, displayName = "Domain computers", description = "Domain computers" })
	l:write(msg .. "\n")
	if not (rc == 0) then return 1 end

	l:write("Creating machine account for " .. host .. " ... ")
	rc,msg,host_dn = hostadd(ld, host, host, 2000, 102, nil)
	l:write(msg .. "\n")
	if not (rc == 0) then return 1 end

	l:write("Adding CIFS KPNs for " .. host .. " ... ")
	ldap_kpn = "ldap/" .. hostl .. "." .. domainl .. "@" .. domain
	rc, msg = ld:modify(host_dn, { '+', krbPrincipalName = { ldap_kpn,
				"cifs/" .. hostl .. "." .. domainl .. "@" .. domain,
				"cifs/" .. hostu .. "." .. domainl .. "@" .. domain,
				"cifs/" .. hostl .. "." .. domainu .. "@" .. domain,
				"cifs/" .. hostu .. "." .. domainu .. "@" .. domain,
				"cifs/" .. hostl .. "@" .. domain,
				"cifs/" .. hostu .. "@" .. domain
			}})
	l:write(msg .. "\n")
	if not (rc == 0) then return 1 end
	
	l:write("Creating Kerberos keytab for " .. host .. " ... ")
	os.execute("rm " .. keytab)
	cmd = "kadmin.local -q \"ktadd -k " .. keytab .. " " .. ldap_kpn .. "\""
	rc = os.execute(cmd)
	if not (rc == 0) then
		l:write("failed to execute: " .. cmd .. "\n")
		return 1
	end
	l:write("Success\n")
	
	l:write("Restarting Samba ... ")
	os.execute("/etc/init.d/samba restart")
	l:write("done\n")
	return 0
end

function base64(data)
	local b='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'

	return ((data:gsub('.', function(x)
		local r,b='',x:byte()
		for i=8,1,-1 do r=r..(b%2^i-b%2^(i-1)>0 and '1' or '0') end
		return r;
	end)..'0000'):gsub('%d%d%d?%d?%d?%d?', function(x)
		if (#x < 6) then return '' end
		local c=0
		for i=1,6 do c=c+(x:sub(i,i)=='1' and 2^(6-i) or 0) end
		return b:sub(c+1,c+1)
	end)..({ '', '==', '=' })[#data%3+1])
end

function get_defaultNamingContext(ld)
	local dn, attrs

	for dn, attrs in ld:search { attrs = { "namingContexts" }, scope="b" } do
		return attrs["namingContexts"];
	end
end

function NT4_hash(pwd)
	local hash = md4.new()
	local i,n,c1,c2

	i = 1
	while i <= pwd:len() do
		n = pwd:byte(i)

		if n < 0x80 then
			c2 = 0
			c1 = n
		elseif (n >= 0xc0) and (n < 0xe0) then
			c2 = math.floor((n - 0xc0) / 0x04)
			c1 = (n % 0x04) * 0x40
			i = i + 1
			c1 = c1 + (pwd:byte(i) % 0x40)
		elseif (n >= 0xe0) and (n < 0xf0) then
			c2 = (n - 0xe0) * 0x10
			i = i + 1
			n = pwd:byte(i)
			c2 = c2 + (math.floor(n / 0x04) % 0x10)
			c1 = (n % 0x04) * 0x40
			i = i +1
			c1 = c1 + (pwd:byte(i) % 0x40)
		end
		hash:update(string.char(c1))
		hash:update(string.char(c2))
		i = i + 1
	end
	return hash:digest():upper()
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
                return attrs["sambaSID"],attrs["sambaDomainName"]
        end
	return "", ""
end

function grpmod(ld, dn, gid, desc)
	local attrs = { '=' }

	attrs["gidNumber"]	= gid
	attrs["sambaSID"]	= get_sambaSID(ld) .. "-" .. gid
	attrs["displayName"]	= desc
	attrs["description"]	= desc
	ld:modify(dn, attrs)
end

function grpadd(ld, cn, gid, desc)
	local ou = get_ou(ld, "Groups")
	local dn = "cn=" ..  cn .. "," .. ou
        local attrs = {}

        attrs["objectClass"]	= { "top", "posixGroup", "sambaGroupMapping" }
        attrs["cn"]		= cn
        attrs["gidNumber"]	= gid
	attrs["sambaSID"]	= get_sambaSID(ld) .. "-" .. gid
        if (not desc) or (desc == "") then desc = cn end
	attrs["description"]	= desc
	attrs["displayName"]	= desc
	attrs["sambaGroupType"]	= 2

        local rc,msg = ld:add(dn, attrs)
        return rc,msg,dn
 end

function usrmod(ld, dn, gn, sn, uidn, gid, kpn, pw, homedir, shell)
	local attrs = { '=' }

	attrs["uidNumber"]        = uidn
	attrs["gidNumber"]        = gid
	if (not gn) or (gn == "") then gn = uid end
	attrs["givenName"]        = gn
	if (not sn) or (sn == "") then sn = uid end
	attrs["sn"]               = sn
	attrs["homeDirectory"]    = homedir
	attrs["loginShell"]       = shell
	attrs["krbPrincipalName"] = kpn

	if pw and pw:len() > 0 then
		attrs["sambaNTPassword"]  = NT4_hash(pw)
		attrs["sambaPwdLastSet"]  = os.time()
		attrs["userPassword"]     = "{SHA}" .. base64(sha1.digest(pw, true))
	end

	local rc, msg = ld:modify(dn, attrs)

	if pw:len() > 0 then
		luci.sys.exec("kadmin.local -q \"cpw -pw " .. pw .. " " .. kpn .. "\"")
	end
end

function usradd(ld, uid, gn, sn, uidn, gid, pw, homedir, shell)
	local ou = get_ou(ld, "Users")
	local realm = get_realm(ld)
	local sid, domain = get_sambaSID(ld)
	local dn = "uid=" .. uid .. "," .. ou
	local kpn = uid .. "@" .. realm
	local attrs = {}

	attrs["objectClass"]		= { "inetOrgPerson", "posixAccount", "krbPrincipalAux", "sambaSamAccount" }
	attrs["cn"]			= uid
	attrs["uidNumber"]		= uidn
	attrs["gidNumber"]		= gid
	if (not gn) or (gn == "") then gn = uid end
	attrs["givenName"]		= gn
	if (not sn) or (sn == "") then sn = uid end
	attrs["sn"]			= sn
	attrs["homeDirectory"]		= homedir
	attrs["loginShell"]		= shell
	attrs["krbPrincipalName"]	= kpn
	attrs["sambaAcctFlags"]		= "[U          ]"
	attrs["sambaSID"]		= sid .. "-" .. uidn + 32768
	attrs["sambaDomainName"]	= domain
	if pw and pw:len() > 0 then
		attrs["sambaNTPassword"]	= NT4_hash(pw)
		attrs["sambaPwdLastSet"]	= os.time()
		attrs["userPassword"]		= "{SHA}" .. base64(sha1.digest(pw, true))
	end

	local rc,msg = ld:add (dn, attrs)
	if (rc == 0) then
		luci.sys.exec("kadmin.local -q \"cpw -pw " .. pw .. " " .. kpn .. "\"")
	end
	return rc,msg,dn
end

function hostmod(ld, dn, desc, uidn, gid, pw, kpn)
	local attrs = { '=' }

	attrs["description"]	= desc or ""
	attrs["uidNumber"]	= uidn
	attrs["gidNumber"]	= gid
	if kpn and not (kpn == "") then
		 attrs["krbPrincipalName"] = kpn
	end
	if pw and pw:len() > 0 then
		attrs["sambaNTPassword"]  = NT4_hash(pw)
		attrs["sambaPwdLastSet"]  = os.time()
	end

	local rc, msg = ld:modify(dn, attrs)

	if rc == 0 and pw and pw:len() > 0 then
		if not kpn then
			for dn, attrs in ld:search { attrs = { "krbPrincipalName" }, base = dn, scope="b" } do
				kpn = attrs["krbPrincipalName"]
				if type(kpn) == "table" then kpn = kpn[1] end
			end
		end
		luci.sys.exec("kadmin.local -q \"cpw -pw " .. pw .. " " .. kpn .. "\"")
	end
	return rc,msg
end

function hostadd(ld, uid, desc, uidn, gid, pw)
	local ou = get_ou(ld, "Computers")
	local realm = get_realm(ld)
	local sid, domain = get_sambaSID(ld)
	local dn = "uid=" .. uid .. "$," .. ou
	local kpn     = "host/" .. uid:lower() .. "." .. realm:lower() .. "@" .. realm
	local attrs = {}

	if not pw or pw == "" then pw = 
		math.randomseed(os.time())
		pw = rnd_pw(12)
	end
	if not desc or desc == "" then
		desc = uid
	end

	attrs["objectClass"]		= { "top", "account", "posixAccount", "krbPrincipalAux", "sambaSAMAccount" }
	attrs["cn"]			= uid .. "$"
	attrs["uid"]			= uid .. "$"
	attrs["uidNumber"]		= uidn
	attrs["gidNumber"]		= gid
	attrs["homeDirectory"]		= "/dev/null"
	attrs["krbPrincipalName"]	= kpn
	attrs["description"]		= desc
	attrs["sambaAcctFlags"]		= "[W          ]"
	attrs["sambaNTPassword"]	= NT4_hash(pw)
	attrs["sambaSID"]    		= sid .. "-" .. uidn + 32768
	attrs["sambaDomainName"]	= domain
	attrs["sambaPwdLastSet"]	= os.time()
	
	local rc,msg = ld:add (dn, attrs)

	if rc == 0 then
		luci.sys.exec("kadmin.local -q \"cpw -pw " .. pw .. " " .. kpn .. "\"")
	end
	return rc,msg,dn
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
