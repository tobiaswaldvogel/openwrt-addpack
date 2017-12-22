local uci = luci.model.uci.cursor_state()

local s

m = Map("xpl")

s = m:section(TypedSection, "code", translate("Code definition"), translate("IR/RF code definition for xPL gateway"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tsection2"
s.header_field = "id"

s:tab("general", translate("General"))
s:tab("pulse", translate("Pulse definition"))
s:tab("vars", translate("Variables"))

id = s:taboption("general", Value, "id", translate("Code ID"))
id.rmempty = true

t = s:taboption("general", ListValue, "type", translate("Code type"))
t.rmempty = true
t:value("ir", "Infrared")
t:value("rf", "Radio")

bits = s:taboption("general", Value, "bits", translate("Bits"))
bits.rmempty = true
bits.datatype = "max(256)"

init = s:taboption("vars", Value, "init", translate("Init value"))
init.rmempty = true

o = s:taboption("pulse", Value, "header", translate("Header"))
o.rmempty = true

z = s:taboption("pulse", Value, "zero", translate("Zero"))
z.rmempty = true

one = s:taboption("pulse", Value, "one", translate("One"))
one.rmempty = true

trail = s:taboption("pulse", Value, "trail", translate("Trail"))
trail.rmempty = true

raw = s:taboption("pulse", Value, "raw", translate("Raw frame"))
raw.rmempty = true

rpt= s:taboption("pulse", Value, "repeat", translate("Repeat"))
rpt.rmempty = true

vars = s:taboption("vars", DynamicList, "var", translate("Variables"))
vars.addremove = true
vars.anonymous = true

crc =  s:taboption("vars", Value, "crc", translate("CRC"))
init.rmempty = true

return m
