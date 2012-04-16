--[[

LuCI Spindown

]]--

m = Map("hd-idle", translate("hd-idle_title"), translate("hd-idle_desc"))

function getpwrstate(dev)
	local hdparm = luci.util.execl("hdparm -C " .. dev)

	for i,line in ipairs(hdparm) do
		local pos = line:find("drive state is:")

		if pos then
			return "hd-idle_" .. line:sub(pos + 15):gsub("^%s*([a-z]*)(.-)%s*$", "%1")
		end
	end

	return translate("hd-idle_unknown")
end

local fs = require "luci.fs"
local devices = {}
luci.util.update(devices, fs.glob("/dev/sd?") or {})

v = m:section(Table, devices, translate("hd-idle_pwr_state"))
disk = v:option(DummyValue, "Disk", translate("hd-idle_disk"))
function disk.cfgvalue(self, section)
	return devices[section]:sub(6)
end

state = v:option(DummyValue, "Power state", translate("hd-idle_pwr_state"))
function state.cfgvalue(self, section)
	return translate(getpwrstate(devices[section]))
end

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
