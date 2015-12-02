module("luci.controller.admin.app", package.seeall)

function index()
	entry({"admin", "app"}, alias("admin", "system", "packages"), _("Applications"), 80).index = true
end