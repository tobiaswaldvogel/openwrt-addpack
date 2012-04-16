--[[

LuCI hdmon

]]--

m = Map("hd-mon", translate("hdmon"), translate("hdmon_desc"))

local fs = require "luci.fs"

if luci.fs.access("/proc/mdstat") then 
	local mdstat =  luci.util.execl("cat /proc/mdstat")
	local raiddevs = {}
	local raidstates = {}

	for i,line in ipairs(mdstat) do
		if line:sub(5,5) == ":" then
			table.insert(raiddevs, line:sub(1,3))
			table.insert(raidstates, line:sub(7))
		end
	end

	v = m:section(Table, raiddevs, translate("raid_arrays"))
	raiddev = v:option(DummyValue, "raiddev", translate("raiddev"))
	function raiddev.cfgvalue(self, section)
		return raiddevs[section]
	end

	raidstate = v:option(DummyValue, "raidstate", translate("raidstate"))
	function raidstate.cfgvalue(self, section)
		return raidstates[section]
	end
end

s = m:section(TypedSection, "raid", translate("raid"))
s.anonymous = true
s:option(Flag, "enabled", translate("enable_mon"))

s = m:section(TypedSection, "smart", translate("smart"))
s.anonymous = true
s.addremove = true
s:option(Flag, "enabled", translate("enable_mon"))
disk = s:option(Value, "disk", translate("disk"))
disk.rmempty = fales
for _, dev in ipairs(luci.fs.glob("/dev/ide/host1/bus0/target?/lun0/disc")) do
	disk:value(dev)
end

s = m:section(TypedSection, "hdmon", translate("monreceiver"))
s.anonymous = true
s:option(Value, "email", translate("email", "Email"))
s:option(Flag, "test", translate("email_test"))
s = m:section(TypedSection, "ssmtp", translate("monsender"))
s.anonymous = true
sender = s:option(Value, "mx", translate("mx"))
sender:value("smtp.gmail.com:587")
s:option(Value, "sender", translate("mailsender"))
s:option(Value, "user", translate("mailuser"))
pwd = s:option(Value, "pwd", translate("mailpwd"))
pwd.password = true
s:option(Flag, "TLS", translate("usetls"))

return m
