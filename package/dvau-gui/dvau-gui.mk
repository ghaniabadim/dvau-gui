################################################################################
#
# dvau-gui
#
################################################################################

DVAU_GUI_VERSION = 1.0.0
DVAU_GUI_SITE = $(TOPDIR)/package/dvau-gui
DVAU_GUI_SITE_METHOD = local
DVAU_GUI_LICENSE = MIT
DVAU_GUI_LICENSE_FILES = LICENSE
DVAU_GUI_DEPENDENCIES = lvgl

# lv_conf.h enables the framebuffer backend; LVGL itself is built with its
# default configuration so the app's configuration applies to app sources.
define DVAU_GUI_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CC) $(TARGET_CFLAGS) -DLV_CONF_INCLUDE_SIMPLE \
		-I$(@D)/src -I$(STAGING_DIR)/usr/include \
		-o $(@D)/dvau-gui $(@D)/src/main.c \
		$(STAGING_DIR)/usr/lib/liblvgl.a -lm -lpthread
endef

define DVAU_GUI_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/dvau-gui $(TARGET_DIR)/usr/bin/dvau-gui
	$(INSTALL) -D -m 755 $(@D)/rootfs-overlay/etc/init.d/S99dvau-gui \
		$(TARGET_DIR)/etc/init.d/S99dvau-gui
endef

$(eval $(generic-package))
