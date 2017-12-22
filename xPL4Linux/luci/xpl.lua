module("luci.controller.xpl", package.seeall)

function index()
	local page

        if not nixio.fs.access("/etc/config/xpld") then
		nixio.fs.writefile("/etc/config/xpld", "config global\n")
        end

	entry({"admin", "services", "xpl"},
		alias("admin", "services", "xpl", "general"),
		_("xPL"), 60)

	entry({"admin", "services", "xpl", "general"},
		cbi("xpl/general"),
		_("General"), 10).leaf = true

	entry({"admin", "services", "xpl", "forwards"},
		cbi("xpl/forwards"),
		_("Forwards"), 20).leaf = true

	entry({"admin", "services", "xpl", "schedule"},
		cbi("xpl/schedule"),
		_("Schedule"), 30).leaf = true

	entry({"admin", "services", "xpl", "irrf_codes"},
		cbi("xpl/irrf_codes"),
		_("IR / RF code definitions"), 40).leaf = true

        entry({"admin", "services", "xpl", "irrf_values"},
                cbi("xpl/irrf_values"),
                _("IR / RF command sets"), 50).leaf = true
end
