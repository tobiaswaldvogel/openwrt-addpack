--[[

LuCI hdidle

]]--

module("luci.controller.hdidle", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/hdidle") then
                local devices = {}
                luci.util.update(devices, require("luci.fs").glob("/dev/sd?") or {})

                nixio.fs.writefile("/etc/config/hdidle", "")
                c = luci.model.uci.cursor()
                for _, dev in ipairs(devices) do
                        c:section("hdidle", "device", nil, { disk=luci.fs.basename(dev), enabled="1", idle_time_unit="minutes", idle_time_interval="30" })
                        c:save("hdidle")
                end
                c:commit("hdidle")
        end

	local e
	
	e = entry({"admin", "services", "hdidle"}, cbi("hdidle"), _("Hard disk idle"), 120)
	e.i18n = "hdidle"
	e.dependent = true
end
