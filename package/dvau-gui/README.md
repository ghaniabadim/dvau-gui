# DVAU LVGL panel

`dvau-gui` is a full-screen LVGL dashboard for the LicheePi Zero framebuffer.
It targets a 480×272 panel and uses `/dev/fb0` by default. Its SysV init script
starts the app automatically after the kernel has registered that device.

## Build

```sh
make dvau_licheepi_zero_lvgl_defconfig
make
```

The generated SD-card image is `output/images/sdcard.img`.

## LCD integration

The V3s display engine must be enabled by the board device tree before the
application can start. `board/sipeed/licheepi_zero/lcd-4.3-rgb666.dtsi` is a
reference simple-panel fragment for the common 9 MHz, 480×272 RGB666 timing.
Include it in the active LicheePi Zero DTS, connect the panel to the V3s DE
RGB output, and set panel power/backlight GPIOs for the exact 40-pin module.
Do not assume pinout, supply voltage, or timing from this sample: confirm them
against the panel's datasheet before connecting hardware.

At runtime, override the framebuffer node if necessary:

```sh
/usr/bin/dvau-gui --fbdev /dev/fb1
```
