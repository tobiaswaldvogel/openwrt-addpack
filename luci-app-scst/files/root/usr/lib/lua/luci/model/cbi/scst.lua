local uci = luci.model.uci.cursor_state()

local m, s, p, b, i

math.randomseed(os.time())

m = Map("scst", translate("iSCSI target"), translate("iSCSI target"))

function contains(t, s)
	local i,j

	for i,j in pairs(t) do
		if j == s then return true end
	end
	return false
end

function m.on_save(self)
	local devices = { }
	local sep = { }
	local scsisn
	local md5, rest, s1, s2

	self.uci:foreach("scst", "device", function(s)
		scsisn = s["scsisn"]
		if scsisn == nil then
			rest = s["path"]
			md5 = ""
			if rest then
				while scsisn == nil and rest do	
					s1,s2 = rest:match("(.-)([^\/]+)$")
					if s2 == nil then
						s2 = rest
						rest = nil
					else
						rest = s1:sub(1, -2)
					end	
					md5 = s2 .. md5
					scsisn = luci.util.exec("echo " .. md5 .. " | md5sum"):match("[0-9a-fA-F]*"):sub(-8)
				        if contains(devices, scsisn) then
						scsisn = nil
					end
				end
				self.uci:set("scst", s[".name"], "scsisn", scsisn)
			end
		end
		if scsisn then
			devices[s["name"]] = scsisn
		end
	end)
end

s = m:section(TypedSection, "global", translate("Global settings"))
s.addremove = false
s.anonymous = true
i = s:option(Value, "id", translate("System ID"))
i.rmempty = false
i.default = "iqn.2012-12.org.openwrt"

s = m:section(TypedSection, "device", translate("Devices"), translate("You can create new image files with: fallocate -l &lt;size e.g. 16GB&gt; &lt;image file&gt;"))
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection"

s:option(Value, "name", translate("Name")).rmempty = true
t = s:option(ListValue, "type", translate("Type"))
t.default = "file"
t:value("file", translate("Image file"))
f = s:option(FileBrowser, "path", translate("Path"))
f.datatype = "file"
f.template = "cbi/browser_t"
b = s:option(ListValue, "blocksize", translate("Blocksize"), translate("Use 512 Byte for VMWare"))
b.default = 512
b:value("512", translate("512 Byte"))
b:value("4096", translate("4 kByte"))
b.rmempty = true
n = s:option(Value, "scsisn", translate("SCSI serial"), translate("8 hex digits"))
n.size = 8
n.datatype="rangelength(8,8)"
n.validate = function(self, value)
  if #value ~= 8  then return nil end
  return value:match("^[0-9a-fA-F]+$")
end

s = m:section(TypedSection, "target", translate("Targets"))
s.addremove = true
s.anonymous = true
s:option(Value, "name", translate("Name")).rmempty = true
l = s:option(DynamicList, "lun", translate("Lun"))
m.uci:foreach("scst", "device", function(s)
	l:value(s.name, s.name)                        
end)

a = s:option(Flag, "auth_in", translate("Authenticate initiator"))
a.enabled="1"
a.disabled="0"
a.default="0"
u = s:option(Value, "id_in", translate("ID"))
u.rmempty = true
u.datatype = "minlength(1)"
u:depends("auth_in", "1")
p = s:option(Value, "secret_in", translate("Secret"), translate("Min. 12 chars."))
p.rmempty = true
p.datatype = "minlength(12)"
p:depends("auth_in", "1")

a = s:option(Flag, "auth_out", translate("Initiator requires authentication"))
a.enabled="1"
a.disabled="0"
a.default="0"
u = s:option(Value, "id_out", translate("ID"))
u.rmempty = true
u.datatype = "minlength(1)"
u:depends("auth_out", "1")
p = s:option(Value, "secret_out", translate("Secret"), translate("Min. 12 chars."))
p.rmempty = true
p.datatype = "minlength(12)"
p:depends("auth_out", "1")

return m
