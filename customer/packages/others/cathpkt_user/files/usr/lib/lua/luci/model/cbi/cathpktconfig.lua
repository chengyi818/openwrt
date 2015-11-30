require("luci.http")

m3 = Map("cathpkt", translate("Cathpkt"),
	translate("Configurations for user data collecting"))

s = m3:section(TypedSection, "CollectUsrData", "")
s.addremove = false
s.anonymous = true

switch = s:option(Flag, "Enable", translate("Enable"))
switch.datatype = "bool"
switch.disabled = "0"
switch.enabled = "1"
switch.default = switch.disabled
switch.rmempty  = false

key = s:option(Value, "Key", translate("Key"))

ftpurl = s:option(Value, "FtpUrl", translate("Server"))
--ftpurl.default = "NULL"
ftpurl.notnull = true
switch.rmempty  = false

user = s:option(Value, "FtpUser", translate("User"))
--user.default = "NULL"
switch.rmempty  = false

passwd = s:option(Value, "FtpPassword", translate("Password"))
--passwd.default = "NULL"
switch.rmempty  = false

time = s:option(Value, "PeriodTime", translate("Period Time"))
time.datatype = "uinteger"
--time.default = 500
switch.rmempty  = false

local apply = luci.http.formvalue("cbi.apply")
if apply then
	io.popen("/etc/init.d/cathpktd restart")
end

return m3