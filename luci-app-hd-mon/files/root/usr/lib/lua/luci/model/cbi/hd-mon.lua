--[[

LuCI hdmon

]]--

m = Map("hd-mon", translate("hdmon"), translate("hdmon_desc"))

local fs = require "luci.fs"
local devices = {}
luci.util.update(devices, fs.glob("/dev/sd?") or {})

m:section(SimpleSection).template = "hd/raid_status"
m:section(SimpleSection).template = "hd/disc_temp_status"

s = m:section(TypedSection, "raid", translate("raid"))
s.anonymous = true
s:option(Flag, "enabled", translate("enable_mon"))

s = m:section(TypedSection, "smart", translate("smart"))
s.anonymous = true
s.addremove = true
s:option(Flag, "enabled", translate("enable_mon"))
disk = s:option(Value, "disk", translate("disk"))
disk.rmempty = fales
for _, dev in ipairs(devices) do
	disk:value(dev)
end

temp_info = s:option(Value, "temp_info", translate("Threshold for temperature info"))
temp_info.default = "40"
temp_warn = s:option(Value, "temp_warn", translate("Threshold for temperature warning"))
temp_warn.default = "45"

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
