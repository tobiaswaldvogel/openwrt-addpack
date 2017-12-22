local uci = luci.model.uci.cursor_state()

m = Map("xpl")

sched = m:section(TypedSection, "schedule", translate("Scheduler"), translate("Scheduled xPL commands"))
sched.addremove = true
sched.anonymous = true

task = sched:option(DynamicList, "target", translate("Target"))
m.uci:foreach("xpld", "alias", function(s)
        task:value(s.alias_addr, s.alias_addr)
end)



hour = sched:option(ListValue, "hour", translate("Hour"))
hour:value("*","*")
for i=0,23,1 do
  hour:value(i,i)
end

minute = sched:option(ListValue, "minute", translate("Minute"))
minute:value("*","*")
for i=0,59,1 do
  minute:value(i,i)
end

return m
