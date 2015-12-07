# All rights reserved.
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=cathpkt
P4REV:=1
PKG_VERSION:=$(P4REV)

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

TAR_CMD=$(HOST_TAR) -C $(1)/ $(TAR_OPTIONS)

define KernelPackage/$(PKG_NAME)
  CATEGORY:=Kernel modules
  TITLE:=Kernel support for cathpkt
  FILES:=$(PKG_BUILD_DIR)/cathpkt.ko
  AUTOLOAD:=$(call AutoLoad,91,cathpkt)
  SUBMENU:=Other modules
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" V=1 \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)/" \
		modules
endef

define KernelPackage/cathpkt/install
	echo ----------------------------------------------------------------cathpkt
endef

$(eval $(call KernelPackage,cathpkt))
