--[[

LuCI hdmon

]]--

m = Map("hd-mon", translate("Hard disk monitoring"), translate("Monitors disks and raid arrays and sends an email in case of failure."))

local fs = require "luci.fs"
local devices = {}
luci.util.update(devices, fs.glob("/dev/sd?") or {})

m:section(SimpleSection).template = "hd/raid_status"
m:section(SimpleSection).template = "hd/disc_temp_status"

s = m:section(TypedSection, "raid", translate("Raid arrays"))
s.anonymous = true
s:option(Flag, "enabled", translate("Enable monitoring"))

s = m:section(TypedSection, "smart", translate("S.M.A.R.T."))
s.anonymous = true
s.addremove = true
s:option(Flag, "enabled", translate("Enable monitoring"))
disk = s:option(Value, "disk", translate("Disk"))
disk.rmempty = fales
for _, dev in ipairs(devices) do
	disk:value(dev)
end

temp_info = s:option(Value, "temp_info", translate("Threshold for temperature info"))
temp_info.default = "40"
temp_warn = s:option(Value, "temp_warn", translate("Threshold for temperature warning"))
temp_warn.default = "45"

s = m:section(TypedSection, "hdmon", translate("Notify options"))
s.anonymous = true
s:option(Value, "email", translate("Email address to notify"))
s:option(Flag, "test", translate("Send test mail"))
s = m:section(TypedSection, "ssmtp", translate("Mail options"))
s.anonymous = true
sender = s:option(Value, "mx", translate("Mail server"))
sender:value("smtp.gmail.com:587")
s:option(Value, "sender", translate("Sender address"))
s:option(Value, "user", translate("User ID"))
pwd = s:option(Value, "pwd", translate("Password"))
pwd.password = true
s:option(Flag, "TLS", translate("Use TLS"))

return m
