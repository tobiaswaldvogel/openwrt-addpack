--[[

LuCI hdmon

]]--

m = Map("transmission", translate("trnsm_transmission"), translate("trnsm_desc"))
s = m:section(TypedSection, "transmission", "transmission")
s.anonymous = true

s:tab("general", translate("trnsm_general_settings"))
s:tab("network", translate("trnsm_network"))
s:tab("speedlimit", translate("trnsm_speedlimits"))
s:tab("limits", translate("trnsm_limits"))
s:tab("advanced", translate("trnsm_advanced_settings"))

s:taboption("general", Flag, "enabled", translate("trnsm_enabled"))
s:taboption("general", Value, "cache_size_mb", translate("trnsm_cache_size_mb"))
s:taboption("general", Value, "download_dir", translate("trnsm_download_dir"))

o = s:taboption("general",Flag, "watch_dir_enabled", translate("trnsm_watch_dir_enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("general", Value, "watch_dir", translate("trnsm_watch_dir"))
o.default = "/mnt/md0/torrent"
o:depends("watch_dir_enabled", "true")

s:taboption("general", Value, "config_dir", translate("trnsm_config_dir"))

o = s:taboption("general", Flag, "blocklist_enabled", translate("trnsm_blocklist_enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("general", Value, "blocklist_url", translate("trnsm_blocklist_url"))
o:depends("blocklist_enabled", "true")


o = s:taboption("general", ListValue, "encryption", translate("trnsm_encryption"))
o:value("0", translate("trnsm_off"))
o:value("1", translate("trnsm_preferred"))
o:value("2", translate("trnsm_forced"))
o.rmempty = false

o = s:taboption("general", Flag, "incomplete_dir_enabled", translate("trnsm_incomplete_dir_enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("general", Value, "incomplete_dir", translate("trnsm_incomplete_dir"))
o:depends("incomplete_dir_enabled", "true")
o.default = "/tmp/transmission/incomplete"

o = s:taboption("general", Flag, "prefetch_enabled", translate("trnsm_prefetch_enabled"))
o.enabled = "true"
o.disabled = "false"

s:taboption("general", Flag, "script_torrent_done_enabled", translate("trnsm_script_torrent_done_enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("general", Value, "script_torrent_done_filename", translate("trnsm_script_torrent_done_filename"))
o:depends("script_torrent_done_enabled", "true")

o = s:taboption("general", Flag, "lazy_bitfield_enabled", translate("trnsm_lazy_bitfield_enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("general", Flag, "lpd_enabled", translate("trnsm_lpd_enabled"))
o.enabled = "true"
o.disabled = "false"

o = s:taboption("general", ListValue, "message_level", translate("trnsm_message_level"))
o:value("0", translate("trnsm_none"))
o:value("1", translate("trnsm_error"))
o:value("2", translate("trnsm_info"))
o:value("3", translate("trnsm_debug"))
o.rmempty = false

o = s:taboption("general", ListValue, "preallocation", translate("trnsm_preallocation"))
o:value("0", translate("trnsm_off"))
o:value("1", translate("trnsm_fast"))
o:value("2", translate("trnsm_full"))
o.rmempty = false

o = s:taboption("general", Flag, "rename_partial_files", translate("trnsm_rename_partial_files"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("general", Flag, "start_added_torrents", translate("trnsm_start_added_torrents"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("general", Flag, "trash_original_torrent_files", translate("trnsm_trash_original_torrent_files"))
o.enabled = "true"
o.disabled = "false"
s:taboption("general", Value, "umask", translate("trnsm_umask"))


s:taboption("network", Value, "bind_address_ipv4", translate("trnsm_bind_address_ipv4"))
s:taboption("network", Value, "bind_address_ipv6", translate("trnsm_bind_address_ipv6"))
s:taboption("network", Value, "peer_port", translate("trnsm_peer_port"))
o = s:taboption("network", Flag, "peer_port_random_on_start", translate("trnsm_peer_port_random_on_start"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("network", Value, "peer_port_random_low", translate("trnsm_peer_port_random_low"))
o:depends("peer_port_random_on_start", "true")
o.default = 49152
o = s:taboption("network", Value, "peer_port_random_high", translate("trnsm_peer_port_random_high"))
o:depends("peer_port_random_on_start", "true")
o.default = 65535
o = s:taboption("network", Flag, "port_forwarding_enabled", translate("trnsm_port_forwarding_enabled"))
o.enabled = "true"
o.disabled = "false"

s:taboption("network", Value, "peer_socket_tos", translate("trnsm_peer_socket_tos"))
o = s:taboption("network", Flag, "pex_enabled", translate("trnsm_pex_enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("network", Flag, "dht_enabled", translate("trnsm_dht_enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("network", Flag, "utp_enabled", translate("trnsm_utp_enabled"))
o.enabled = "true"
o.disabled = "false"




o = s:taboption("speedlimit", Flag, "speed_limit_down_enabled", translate("trnsm_speed_limit_down_enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("speedlimit", Value, "speed_limit_down", translate("trnsm_speed_limit_down"))
o.default = "100"
o:depends("speed_limit_down_enabled", "true")
o = s:taboption("speedlimit", Flag, "speed_limit_up_enabled", translate("trnsm_speed_limit_up_enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("speedlimit", Value, "speed_limit_up", translate("trnsm_speed_limit_up"))
o.default = "20"
o:depends("speed_limit_up_enabled", "true")



o = s:taboption("speedlimit", Flag, "alt_speed_enabled", translate("trnsm_alt_speed_enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("speedlimit", Value, "alt_speed_down", translate("trnsm_alt_speed_down"))
o:depends("alt_speed_enabled", "true")
o.default = "50"
o = s:taboption("speedlimit", Value, "alt_speed_up", translate("trnsm_alt_speed_up"))
o:depends("alt_speed_enabled", "true")
o.default = "50"

o = s:taboption("speedlimit", Flag, "alt_speed_time_enabled", translate("trnsm_alt_speed_time_enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("speedlimit", Value, "alt_speed_time_begin", translate("trnsm_alt_speed_time_begin"))
o:depends("alt_speed_time_enabled", "true")
o = s:taboption("speedlimit", Value, "alt_speed_time_day", translate("trnsm_alt_speed_time_day"))
o:depends("alt_speed_time_enabled", "true")
o = s:taboption("speedlimit", Value, "alt_speed_time_end", translate("trnsm_alt_speed_time_end"))
o:depends("alt_speed_time_enabled", "true")

o = s:taboption("speedlimit", Flag, "idle_seeding_limit_enabled", translate("trnsm_idle_seeding_limit_enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("speedlimit", Value, "idle_seeding_limit", translate("trnsm_idle_seeding_limit"))
o:depends("idle_seeding_limit_enabled", "true")
o.default = "30"


o = s:taboption("limits", Flag, "ratio_limit_enabled", translate("trnsm_ratio_limit_enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("limits", Value, "ratio_limit", translate("trnsm_ratio_limit"))
o:depends("ratio_limit_enabled", "true")
o.default = "2.0000"
s:taboption("limits", Value, "open_file_limit", translate("trnsm_open_file_limit"))
s:taboption("limits", Value, "peer_congestion_algorithm", translate("trnsm_peer_congestion_algorithm"))
s:taboption("limits", Value, "peer_limit_global", translate("trnsm_peer_limit_global"))
s:taboption("limits", Value, "peer_limit_per_torrent", translate("trnsm_peer_limit_per_torrent"))
s:taboption("limits", Value, "upload_slots_per_torrent", translate("trnsm_upload_slots_per_torrent"))




s:taboption("advanced", Value, "run_daemon_as_user", translate("trnsm_run_daemon_as_user"))
o = s:taboption("advanced", Flag, "rpc_enabled", translate("trnsm_rpc_enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("advanced", Value, "rpc_bind_address", translate("trnsm_rpc_bind_address"))
o:depends("rpc_enabled", "true")
o = s:taboption("advanced", Value, "rpc_port", translate("trnsm_rpc_port"))
o:depends("rpc_enabled", "true")
o = s:taboption("advanced", Value, "rpc_url", translate("trnsm_rpc_url"))
o:depends("rpc_enabled", "true")
o = s:taboption("advanced", Flag, "rpc_authentication_required", translate("trnsm_rpc_authentication_required"))
o.enabled = "true"
o.disabled = "false"
o:depends("rpc_enabled", "true")
o = s:taboption("advanced", Value, "rpc_username", translate("trnsm_rpc_username"))
o:depends("rpc_authentication_required", "true")
o = s:taboption("advanced", Value, "rpc_password", translate("trnsm_rpc_password"))
o:depends("rpc_authentication_required", "true")
o = s:taboption("advanced", Flag, "rpc_whitelist_enabled", translate("trnsm_RPC whitelist enabled"))
o.enabled = "true"
o.disabled = "false"
o = s:taboption("advanced", Value, "rpc_whitelist", translate("trnsm_rpc_whitelist"))
o.default = "127.0.0.1"
o:depends("rpc_whitelist_enabled", "true")



return m
