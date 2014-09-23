--[[

LuCI hdmon

]]--

m = Map("smartd", translate("S.M.A.R.T. Hard disk monitoring"), translate("Monitors disks using S.M.A.R.T."))

require("luci.fs")

local devices = {}
luci.util.update(devices, luci.fs.glob("/dev/sd?") or {})

s = m:section(SimpleSection)
s.template = "admin_status/disc_status"
s.hide_raid = 1

s = m:section(TypedSection, "device")
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"
disk = s:option(Value, "disk", translate("Disk"))
disk.rmempty = fales
for _, dev in ipairs(devices) do
	disk:value(luci.fs.basename(dev))
end
s:option(Flag, "testmail", translate("Send test email"))

temp_info = s:option(Value, "temp_info", translate("Temperature info at C&deg;"))
temp_info.default = "40"
temp_warn = s:option(Value, "temp_warn", translate("Temperature warning at C&deg;"))
temp_warn.default = "45"

function m.on_commit(self,map)
	require("luci.sys").call('/sbin/reload_config')
end

return m
