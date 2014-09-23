--[[

LuCI mdadm

]]--

m = Map("mdadm", translate("RAID monitoring"), translate("Monitors RAID arrays and sends an email notification in case of failure."))

require("luci.fs")

s = m:section(SimpleSection)
s.template = "admin_status/disc_status"
s.hide_disc = 1

s = m:section(TypedSection, "mdadm")
s.anonymous = true
s:option(Flag, "enabled", translate("Enable monitoring"))
s:option(Flag, "testmail", translate("Send test email"))

function m.on_commit(self,map)
	require("luci.sys").call('/sbin/reload_config')
end

return m
