include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=dosattack
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/dosattack
  SUBMENU:=Other modules
  DEPENDS:=@!LINUX_3_3
  TITLE:=Simple GPIO Button Hotplug driver
  FILES:=$(PKG_BUILD_DIR)/dosattack.ko
  KCONFIG:=
endef

define KernelPackage/dosattack/description
 This is a replacement for the following in-kernel drivers:
endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,dosattack))
