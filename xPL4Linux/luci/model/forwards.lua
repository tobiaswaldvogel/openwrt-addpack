local uci = luci.model.uci.cursor_state()

m = Map("xpl")

s = m:section(TypedSection, "forward", translate("Forward"), translate("Forwards xPL commands to multiple targets"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tsection2"
s.header_field = "source"

a = s:option(Value, "source", translate("xPL source"))
a.rmempty = true
a.validate = function(self, value)
  return value:match("^.+-.+\..+$")
end

t = s:option(DynamicList, "target", translate("Target"))
t.rmempty = true
t.validate = function(self, value)
  if type(value) == "string" then
    return value:match("^.+-.+\..+$")
  end
  return value 
end

return m
