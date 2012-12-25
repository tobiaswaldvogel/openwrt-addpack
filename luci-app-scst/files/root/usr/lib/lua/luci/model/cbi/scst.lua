local uci = luci.model.uci.cursor_state()
local m, s, p, b, i

m = Map("scst", translate("iSCSI target"), translate("iSCSI target"))

s = m:section(TypedSection, "global", translate("Global settings"))
s.addremove = false
s.anonymous = true
i = s:option(Value, "id", translate("System ID"))
i.rmempty = false
i.default = "iqn.2012-12.org.openwrt"

s = m:section(TypedSection, "device", translate("Devices"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection"
s:option(Value, "name", translate("Name")).rmempty = true
t = s:option(ListValue, "type", translate("Type"))
t.default = "file"
t:value("file", translate("Image file"))
s:option(Value, "path", translate("Path")).rmempty = true
b = s:option(Value, "blocksize", translate("Blocksize"))
b.rmempty = true
b.default = 4096

s = m:section(TypedSection, "target", translate("Targets"))
s.addremove = true
s.anonymous = true
s:option(Value, "name", translate("Name")).rmempty = true
l = s:option(DynamicList, "lun", translate("Lun"))
m.uci:foreach("scst", "device", function(s)
	l:value(s.name, s.name)                        
end)

return m
