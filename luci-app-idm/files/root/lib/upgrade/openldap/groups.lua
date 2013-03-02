require "lualdap"

function contains(t, s)
        local i,j

        for i,j in pairs(t) do
                if j == s then return true end
        end
        return false
end

function get_sambaSID(ld)
	local dn, attrs

	for dn, attrs in ld:search { attrs = { "sambaSID", "sambaDomainName" }, base = basedn, filter="objectClass=sambaDomain", scope="s" } do
		return attrs["sambaSID"],attrs["sambaDomain"]
	end
end


local rc,msg,ld = lualdap.open("ldapi:///", "EXTERNAL")
local gid, desc, rid

for dn, attrs in ld:search { filter="objectClass=posixGroup", scope="s" } do
	if not contains(attrs["objectClass"], "sambaGroupMapping") then
		gid = attrs["gidNumber"]
		desc = attrs["description"]

		if gid == "100" then
			rid = 513
			desc = "Domain users"
		elseif gid == "102" then
			rid = 515
			desc = "Domain computers"
		else
			rid = gid
		end

		table.insert(attrs["objectClass"], "sambaGroupMapping")
		attrs["sambaGroupType"] = 2
		attrs["sambaSID"] = get_sambaSID(ld) .. "-" .. rid
		attrs["displayName"] = desc
		attrs["description"] = desc
		attrs[1] = "="
		ld:modify(dn, attrs)
	end
end
