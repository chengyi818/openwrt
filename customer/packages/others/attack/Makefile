include $(TOPDIR)/rules.mk

PKG_NAME:=attack
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/attack
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=attack-- Anti dos attack
endef

define Package/attack/description
	use to anti dos attack
endef

define Package/attack/postinst
#!/bin/sh
rm -rf /tmp/luci*
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/attack $(1)/etc/init.d/attack
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
