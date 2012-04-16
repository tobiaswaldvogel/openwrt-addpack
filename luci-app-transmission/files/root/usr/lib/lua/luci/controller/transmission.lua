--[[

LuCI transmission

]]--

module("luci.controller.transmission", package.seeall)

local session_id

function index()
	if not nixio.fs.access("/etc/config/transmission") then
		return
	end

	local e
	
	e = entry({"admin", "services", "transmission"}, cbi("transmission"), _("Transmission"), 120)
	e.i18n = "transmission"
	e.dependent = true

--[[
	e = entry({"admin", "status", "transmission"}, call("transmission_web"), _("Transmission"), 120)
	e.i18n = "transmission"
	e.dependent = true

	e = entry({"admin", "status", "transmission", "rpc" }, call("transmission_rpc"), "", 120)
	e.i18n = "transmission"
	e.leaf = true
	e.dependent = true
]]--
end

function transmission_web()
	local http =require("luci.http")
	local rpcroot = http.protocol.urlencode(require("luci.dispatcher").build_url("admin/status/transmission/rpc"))
	http.header("Set-Cookie", "rpcroot=" .. rpcroot .. "; path=/luci-static/transmission")
	luci.template.render("header")
	http.write("<iframe src=\"/luci-static/transmission/index.html\" width=\"100%\" height=\"800px\" name=\"transmission\">")
	luci.template.render("footer")
end

function transmission_rpc()
	local http = require("luci.http")
	http.setfilehandler(nil)
	local req, reqlen = http.content()
	local session_id = http.getcookie("TransmissionSession")

	local httpc = require("luci.httpclient")
	local uri = "http://localhost:9091/transmission/rpc/"
	local options

	if (session_id) then

	options = {
		headers = {
			["X-Transmission-Session-Id"] = session_id,
		},
		body = req
	}

	else

	options = { body = req }

	end

	local response, code, msg = httpc.request_to_buffer(uri, options)

--[[
	local response, code, msg = httpc.request_to_buffer(uri, options)
	if (req) then
		http.write("<br>request was=" .. req)
	end
	if (msg) then
		http.write("<br>msg=" .. msg)
	end

--]]

	if (msg) then
		local session_id = string.sub(msg, string.find(msg, "<code>") + 33, string.find(msg, "</code>") - 1)
		if (session_id) then
			local rpcroot = require("luci.dispatcher").build_url("admin/status/transmission/rpc")
			http.header("Set-Cookie", "TransmissionSession=" .. session_id .. "; path=" .. rpcroot)
		end
		http.write(msg)
	elseif (response) then
		http.prepare_content("application/json")
		http.write(response)
	end

--[[
	if (req) then
		http.write("<br>request was=" .. req)
	end

	t = http.getenv()
	for k,v in pairs(t) do
		http.write(k .. "=" .. v .. "\r\n")
	end
--]]

end

function torrent_startup()
	local http = require("luci.http")
	local disc = http.formvalue("disc")
	local fmg = require("luci.fmg")
	fmg.start("torrent", disc.."/")
	os.execute("sleep 2")
	return require("luci.http").redirect(require("luci.dispatcher").build_url("admin/services/transmission"))
end

function torrent_download()
	local http = require("luci.http")
	local disc = http.formvalue("disc")
	local fmg = require("luci.fmg")
	fmg.install("torrent", disc)
	return require("luci.http").redirect(require("luci.dispatcher").build_url("admin/services/transmission"))
end

function torrent_shutdown()
	local fmg = require("luci.fmg")
	fmg.stop("torrent")
	os.execute("sleep 1")
	return require("luci.http").redirect(require("luci.dispatcher").build_url("admin/services/transmission"))
end

function torrent_cmd(cmd)
	local http = require("luci.http")
	local id = http.formvalue("id")
	local t = require("luci.transmission")
	t.issue_cmd(cmd, id)
	return require("luci.http").redirect(require("luci.dispatcher").build_url("admin/services/transmission"))
end

function torrent_cmd(cmd)
	local http = require("luci.http")
	local id = http.formvalue("id")
	local t = require("luci.transmission")
	t.issue_cmd(cmd, id)
	return require("luci.http").redirect(require("luci.dispatcher").build_url("admin/services/transmission"))
end

function torrent_add()
	local http = require("luci.http")
	local link = http.formvalue("torrent_link")
	local up = http.formvalue("torrent_up")
	local down = http.formvalue("torrent_down")
	local t = require("luci.transmission")
	if link then
		t.add_torrent(link)
	end
	if up then
		t.speed_limit("up", (up ~= "0") and up or nil)
	end
	if down then
		t.speed_limit("down", (down ~= "0") and down or nil)
	end
	return require("luci.http").redirect(require("luci.dispatcher").build_url("admin/services/transmission"))
end

function torrent_start()
	return torrent_cmd("start")
end

function torrent_pause()
	return torrent_cmd("stop")
end

function torrent_del()
	return torrent_cmd("remove")
end
