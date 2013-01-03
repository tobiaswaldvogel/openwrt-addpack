--[[

LuCI Spindown

]]--

require "luci.fs"

m = Map("hd-idle", translate("HD idle"), translate("Spin down disks after a period of idle time."))

m:section(SimpleSection).template = "hd/disc_power_status"

s = m:section(TypedSection, "hd-idle", translate("Settings"))
s.addremove = true
s.anonymous = true

s:option(Flag, "enabled", translate("Enable"))

disk = s:option(Value, "disk", translate("Disk"))
disk.rmempty = false
for _, dev in ipairs(luci.fs.glob("/dev/sd?")) do
	disk:value(dev:sub(6))
end

idle = s:option(Value, "idle_time_interval", translate("Idle time"))
idle.rmempty = true
idle.size = 4

unit = s:option(ListValue, "idle_time_unit", translate("Unit"))
unit.default = "minutes"
unit:value("seconds", translate("Seconds"))
unit:value("minutes", translate("Minutes"))
unit:value("hours", translate("Hours"))
unit:value("days", translate("Days"))
unit.rmempty = false

return m
