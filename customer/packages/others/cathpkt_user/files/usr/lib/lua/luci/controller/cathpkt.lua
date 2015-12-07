module("luci.controller.cathpkt", package.seeall)

function index()
	entry({"admin", "app", "cathpkt"}, cbi("cathpktconfig"), _("Cathpkt Config"), 100)
end