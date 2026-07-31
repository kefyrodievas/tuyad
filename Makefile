include $(TOPDIR)/rules.mk

PKG_NAME:=tuyad
PKG_RELEASE:=1
PKG_VERSION:=1.0.1

CMAKE_INSTALL:=1


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/tuyad
	CATEGORY:=Base system
	TITLE:=tuyad
	DEPENDS:=+libtuyasdk +libblobmsg-json +libubus +libubox
endef

define Package/tuyad/description
	Connects the router to the Tuya platform 
endef

define Package/tuyad/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tuyad $(1)/usr/bin
	$(INSTALL_BIN) ./files/tuyad.init $(1)/etc/init.d/tuyad
	$(INSTALL_CONF) ./files/tuyad.config $(1)/etc/config/tuyad
endef

$(eval $(call BuildPackage,tuyad))
