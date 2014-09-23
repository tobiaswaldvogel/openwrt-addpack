--[[

LuCI ssmtp

]]--

m = Map("ssmtp", translate("Email notifications"), translate("Email notifications settings for ssmtp."))

s = m:section(TypedSection, "ssmtp", translate("Mail options"))
s.anonymous = true
s:option(Value, "email", translate("Email address to notify"))
sender = s:option(Value, "mx", translate("Mail server"))
sender:value("smtp.gmail.com:587")
s:option(Value, "sender", translate("Sender address"))
s:option(Value, "user", translate("User ID"))
pwd = s:option(Value, "pwd", translate("Password"))
pwd.password = true
s:option(Flag, "TLS", translate("Use TLS"))

function m.on_commit(self,map)
	require("luci.sys").call('/sbin/reload_config')
end

return m
