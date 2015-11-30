include $(TOPDIR)/rules.mk

PKG_NAME:=cathpkt
PKG_VERSION:=1.0
PKG_RELEASE:=1

#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libuci
  TITLE:=Used for collecting user data
endef

define Package/$(PKG_NAME)/description
  This package is used for user data collecting.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

#define Package/cathpkt/postinst
##!/bin/sh
#rm -rf /tmp/luci-*
#endef

define Build/Compile
	$(call Build/Compile/Default)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/cathpkt  $(1)/usr/bin

	#$(INSTALL_DATA) ./files/etc/config/cathpkt $(1)/etc/config
	#$(INSTALL_BIN) ./files/etc/init.d/cathpkt $(1)/etc/init.d
	#$(CP)  ./files/etc/rc.d/* $(1)/etc/rc.d
	$(CP)  ./files/* $(1)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
