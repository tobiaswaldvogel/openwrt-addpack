module("luci.controller.scst", package.seeall)

function index()
	local page

        if not nixio.fs.access("/etc/config/scst") then
		nixio.fs.writefile("/etc/config/scst", "config global\n")
        end

	page = entry({"admin", "services", "scst"}, cbi("scst"), _("iSCSI target"), 60)
	page.i18n = "scst"
	page.dependent = true
end
