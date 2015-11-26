include $(TOPDIR)/rules.mk

PKG_NAME:=antiportscan
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/antiportscan
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE:=Anti Port Scan
endef

define Package/antiportscan/description
  This package for anti port scan.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/antiportscan/postinst
#!/bin/sh
rm -rf /tmp/luci-*
endef

define Package/antiportscan/postrm
#!/bin/sh
rm -rf /tmp/luci-*
endef

define Package/antiportscan/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/www/luci-static/win8/images/app
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller/admin
	$(INSTALL_DIR) $(1)/etc/rc.d

#	$(INSTALL_CONF) ./files/root/etc/config/antiportscan $(1)/etc/config/antiportscan
	$(CP) ./files/root/etc/* $(1)/etc/
#	$(INSTALL_BIN) ./files/root/etc/init.d/antiportscan $(1)/etc/init.d/antiportscan
	$(INSTALL_DATA) ./files/luasrc/model/cbi/antiportscan.lua $(1)/usr/lib/lua/luci/model/cbi/antiportscan.lua
	$(INSTALL_DATA) ./files/luasrc/controller/admin/antiportscan.lua $(1)/usr/lib/lua/luci/controller/admin/antiportscan.lua
	$(INSTALL_BIN) ./files/root/www/luci-static/win8/images/app/* $(1)/www/luci-static/win8/images/app/
#	$(INSTALL_BIN) ./files/root/etc/rc.d/S50antiportscan $(1)/etc/rc.d/S50antiportscan
endef

$(eval $(call BuildPackage,antiportscan))
