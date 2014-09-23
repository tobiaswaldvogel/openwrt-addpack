--[[

LuCI mdadm

]]--

module("luci.controller.mdadm", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/mdadm") then
		nixio.fs.writefile("/etc/config/mdadm", "")
                c = luci.model.uci.cursor()
		c:section("mdadm", "mdadm", nil, { enabled=1, testmail=1 })
		c:save("mdadm")
                c:commit("mdadm")
	end
	
	local e

	e = entry({"admin", "services", "mdadm"}, cbi("mdadm"), _("RAID monitoring"), 120)
	e.i18n = "mdadm"
	e.dependent = true
end
