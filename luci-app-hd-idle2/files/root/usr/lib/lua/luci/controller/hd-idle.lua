--[[

LuCI hd-idle

]]--

module("luci.controller.hd-idle", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/hd-idle") then
		return
	end

	local e
	
	e = entry({"admin", "services", "hd-idle"}, cbi("hd-idle"), _("Hard disk idle"), 120)
	e.i18n = "hd-idle"
	e.dependent = true
end
