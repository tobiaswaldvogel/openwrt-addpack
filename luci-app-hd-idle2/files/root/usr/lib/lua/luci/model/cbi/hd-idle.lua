--[[

LuCI Spindown

]]--

require "luci.fs"

m = Map("hd-idle", translate("hd-idle_title"), translate("hd-idle_desc"))

m:section(SimpleSection).template = "hd/disc_power_status"

s = m:section(TypedSection, "hd-idle", translate("hd-idle_settings"))
s.addremove = true
s.anonymous = true

s:option(Flag, "enabled", translate("hd-idle_enable"))

disk = s:option(Value, "disk", translate("hd-idle_disk"))
disk.rmempty = false
for _, dev in ipairs(luci.fs.glob("/dev/sd?")) do
	disk:value(dev:sub(6))
end

idle = s:option(Value, "idle_time_interval", translate("hd-idle_idle_time_interval"))
idle.rmempty = true
idle.size = 4

unit = s:option(ListValue, "idle_time_unit", translate("hd-idle_idle_time_unit"))
unit.default = "minutes"
unit:value("seconds", translate("hd-idle_seconds"))
unit:value("minutes", translate("hd-idle_minutes"))
unit:value("hours", translate("hd-idle_hours"))
unit:value("days", translate("hd-idle_days"))
unit.rmempty = false

return m
