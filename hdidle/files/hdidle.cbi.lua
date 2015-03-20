--[[

LuCI Spindown

]]--

m = Map("hdidle", translate("HD idle"), translate("Spin down disks after a period of idle time."))

s = m:section(SimpleSection)
s.template = "admin_status/disc_status"
s.hide_raid = 1

s = m:section(TypedSection, "device", translate("Power down"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection"

disk = s:option(Value, "disk", translate("Disk"))
disk.rmempty = false


for dev in nixio.fs.glob("/dev/sd?") do
	disk:value(dev:sub(6))
end

s:option(Flag, "enabled", translate("Enable"))
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

function m.on_commit(self,map)
	require("luci.sys").call('/sbin/reload_config')
end

return m
