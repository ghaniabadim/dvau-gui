################################################################################
#
# lvgl
#
################################################################################

LVGL_VERSION = 9.2.2
LVGL_SITE = $(call github,lvgl,lvgl,v$(LVGL_VERSION))
LVGL_LICENSE = MIT
LVGL_LICENSE_FILES = LICENCE.txt
LVGL_INSTALL_STAGING = YES

# The Linux framebuffer backend is enabled by applications through lv_conf.h.
# Build a static library so every application can supply its own LVGL config.
define LVGL_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CC) $(TARGET_CFLAGS) -DLV_CONF_SKIP -DLV_USE_LINUX_FBDEV=1 \
		-I$(@D) -c $$(find $(@D)/src -name '*.c')
	$(TARGET_AR) rcs $(@D)/liblvgl.a *.o
	$(RM) *.o
endef

define LVGL_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 644 $(@D)/liblvgl.a $(STAGING_DIR)/usr/lib/liblvgl.a
	cp -a $(@D)/lvgl.h $(@D)/src $(STAGING_DIR)/usr/include/
endef

$(eval $(generic-package))
