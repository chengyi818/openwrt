module("luci.controller.admin.attack", package.seeall)

function index()
    entry({"admin", "attack"}, firstchild(), "App", 80).dependent = false
    entry({"admin", "attack", "attack"}, cbi("dosapp/attack"), _("Dosattack"), 1)
end
