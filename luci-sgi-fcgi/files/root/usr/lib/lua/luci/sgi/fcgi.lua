local ltn12 = require("luci.ltn12")
require("nixio.util")
require("luci.http")
require("luci.sys")
require("luci.dispatcher")
require("luci.cacheloader")

luci.dispatcher.indexcache = "/tmp/luci-indexcache"

-- os.exit would exit the fastcgi server
-- therefore we replace it with a dummy function
function os_exit_dummy(code)
end
_G["os"]["exit"] = os_exit_dummy

os.exit(0)

function main(env, req)
	exectime = os.clock()
        local r = luci.http.Request(env, limitsource(req, tonumber(env.CONTENT_LENGTH)), ltn12.sink.file(io.stderr))
        local x = coroutine.create(luci.dispatcher.httpdispatch)
        local hcache = {}
        local active = true

        while coroutine.status(x) ~= "dead" do
                local res, id, data1, data2 = coroutine.resume(x, r)

                if not res then
			req:header({['Status'] = '500 Internal Server Error', ['Content-Type'] = 'text/plain'})
			req:puts(id)
                        break;
                end

                if active then
                        if id == 1 then
				hcache['Status'] =  tostring(data1) .. " " .. data2
                        elseif id == 2 then
				hcache[data1] = data2
                        elseif id == 3 then
				req:header(hcache)
                        elseif id == 4 then
                                req:puts(tostring(data1 or ""))
                        elseif id == 5 then
                                active = false
                        elseif id == 6 then
                                data1:copyz(nixio.stdout, data2)
                                data1:close()
                        end
                end
        end
end

function limitsource(req, limit)
	limit = limit or 0

	return function()
		if limit < 1 then
			return nil
		end

		local chunk = req:gets()
		local len = chunk:len()

		if len > limit then
			len = limit
			chunk = chunk:sub(1, len)
		end

		limit = limit - len
		return chunk
	end
end
