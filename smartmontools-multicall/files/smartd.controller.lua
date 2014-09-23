--[[

LuCI smartd

]]--

module("luci.controller.smartd", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/smartd") then
		local devices = {}
		luci.util.update(devices, require("luci.fs").glob("/dev/sd?") or {})
		
		nixio.fs.writefile("/etc/config/smartd", "")
		c = luci.model.uci.cursor()
		for _, dev in ipairs(devices) do
			c:section("smartd", "device", nil, { disk=luci.fs.basename(dev), testmail="1", temp_info="40", temp_warn="45" })
			c:save("smartd")
		end
		c:commit("smartd")
	end
	
	local e

	e = entry({"admin", "services", "smartd"}, cbi("smartd"), _("S.M.A.R.T. monitoring"), 120)
	e.i18n = "smartd"
	e.dependent = true
end
