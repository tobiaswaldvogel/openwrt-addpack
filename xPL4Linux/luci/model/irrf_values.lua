local uci = luci.model.uci.cursor_state()

m = Map("xpl")

s = m:section(TypedSection, "map", translate("Command sets"), translate("Friendly names for command values"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tsection2"
s.header_field = "id"

id = s:option(Value, "id", translate("Command set"))

mapping = s:option(DynamicList, "mapping", translate("Values"))
mapping.addremove = true
mapping.anonymous = true


return m
