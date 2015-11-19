m = Map("attack", translate("DOSATTACK CONTROL"), translate("choose your configure"))

s = m:section(TypedSection, "arguments", translate("You can change the configure if you like.But if you select this option,you can prevent dos attack.Otherwise,you may face dos attack."))
s.addremove = false
s.anonymous = false

s:option(Flag, "enable", translate("DOSATTACK_STATUS:"))

local apply = luci.http.formvalue("cbi.apply")
if apply then
    io.popen("/etc/init.d/attack start")
end

return m
