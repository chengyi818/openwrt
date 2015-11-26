--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008-2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

require("luci.sys")
 
m = Map("antiportscan", translate("Anti Port Scaning Configuration"), translate("Anti Port Scaning Configuration."))
 
s = m:section(TypedSection, "antiportscan", "")
s.addremove = false
s.anonymous = true
 
enable = s:option(Flag, "enable", translate("Enable"))

local apply = luci.http.formvalue("cbi.apply")
if apply then
    io.popen("/etc/init.d/antiportscan restart")
end

return m
