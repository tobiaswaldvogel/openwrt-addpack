local uci = luci.model.uci.cursor_state()

m = Map("xpl")

s = m:section(TypedSection, "gateway", translate("xPL gateways"), translate("xPL gateways"))
s.anonymous = true

if nixio.fs.access("/usr/bin/xpl433mhz") then
	xpl433 = s:option(Flag, "enable_xpl433mhz", translate("Enable xPL USB 433Mhz gateway"))
end

if nixio.fs.access("/usr/bin/xplbl") then
	bl = s:option(Flag, "enable_xplbl", translate("Enable xPL to Broadlink IR/RF gateway"))
end

return m
