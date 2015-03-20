--[[

LuCI ssmtp

]]--

module("luci.controller.ssmtp", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/ssmtp") then
		nixio.fs.writefile("/etc/config/ssmtp", "config ssmtp")
	end
	
	local e

	e = entry({"admin", "services", "ssmtp"}, cbi("ssmtp"), _("Email notification"), 120)
	e.i18n = "ssmtp"
	e.dependent = true
end
