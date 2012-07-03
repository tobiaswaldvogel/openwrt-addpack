--[[

LuCI hdmon

]]--

module("luci.controller.hd-mon", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/hd-mon") then
		return
	end

	local e
	
	e = entry({"admin", "services", "hd-mon"}, cbi("hd-mon"), _("Hard disk monitoring"), 120)
	e.i18n = "hd-mon"
	e.dependent = true
end
