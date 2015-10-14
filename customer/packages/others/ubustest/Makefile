include $(TOPDIR)/rules.mk

PKG_NAME:=ubustest
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/ubustest
    SECTION:=utils
    CATEGORY:=Utilities
    TITLE:=ubustest -- prints a snarky message
endef

define Package/ubustest/description
    It's chengyi ubus test.
endef

define Build/Prepare
    echo "Here is Package/Prepare"
    mkdir -p $(PKG_BUILD_DIR)
    $(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/ubustest/install
    echo "Here is Package/install"
    $(INSTALL_DIR) $(1)/bin
    $(INSTALL_BIN) $(PKG_BUILD_DIR)/server $(1)/bin/
    $(INSTALL_BIN) $(PKG_BUILD_DIR)/client $(1)/bin/
endef

$(eval $(call BuildPackage,ubustest))
