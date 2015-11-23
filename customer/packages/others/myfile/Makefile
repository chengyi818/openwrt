include $(TOPDIR)/rules.mk

PKG_NAME:=myfile
PKG_VERSION=1.0
PKG_RELEASE:=2

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/myfile
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=Web Based File Server For Myfile
  DEPENDS:=+php5 +php5-cgi +php5-mod-json
endef

define Package/myfile/description
  This package for Web Based File Server For Myfile
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/myfile/postinst
#!/bin/sh
uci set uhttpd.web=uhttpd
uci add_list uhttpd.web.listen_http=0.0.0.0:88
uci set uhttpd.web.home=/www
uci set uhttpd.web.index_page=index.php
uci add_list uhttpd.web.interpreter=".php=/usr/bin/php-cgi"
uci commit
if [ -f /etc/init.d/uhttpd ]; then
/etc/init.d/uhttpd restart
fi
rm -rf /tmp/luci-*
endef

define Package/myfile/postrm
#!/bin/sh
uci delete uhttpd.web
uci commit
if [ -f /etc/init.d/uhttpd ]; then
/etc/init.d/uhttpd restart
fi
rm -rf /tmp/luci-*
endef

define Package/myfile/install
	$(INSTALL_DIR) $(1)/www/myfile
	$(INSTALL_DIR) $(1)/etc/uhttpd/conf.d
	$(CP) ./files/elfinder-2.0-rc1/* $(1)/www/myfile/
	$(INSTALL_BIN) ./files/root/etc/uhttpd/conf.d/10-myfile.conf $(1)/etc/uhttpd/conf.d/10-myfile.conf
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller/admin
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/admin_app
	$(INSTALL_DIR) $(1)/www/luci-static/win8/images/app	
	$(INSTALL_DATA) ./files/luasrc/controller/myfile.lua $(1)/usr/lib/lua/luci/controller/admin/myfile.lua
	$(INSTALL_DATA) ./files/luasrc/view/myfile.htm $(1)/usr/lib/lua/luci/view/admin_app/myfile.htm
endef

$(eval $(call BuildPackage,myfile))
