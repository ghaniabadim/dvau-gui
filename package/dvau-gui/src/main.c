#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lv_conf.h"
#include "lvgl.h"
#include "src/drivers/display/fb/lv_linux_fbdev.h"

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

static volatile sig_atomic_t keep_running = 1;
static lv_obj_t *temperature_label;
static lv_obj_t *clock_label;
static bool relay_enabled;

static void stop_handler(int signal)
{
	(void)signal;
	keep_running = 0;
}

static void set_label_text(lv_obj_t *label, const char *text)
{
	if (label != NULL)
		lv_label_set_text(label, text);
}

static void relay_event(lv_event_t *event)
{
	lv_obj_t *button = lv_event_get_target(event);

	relay_enabled = !relay_enabled;
	lv_obj_set_style_bg_color(button,
		lv_color_hex(relay_enabled ? 0x21c55d : 0x334155), LV_PART_MAIN);
	lv_obj_t *label = lv_obj_get_child(button, 0);
	set_label_text(label, relay_enabled ? "RELAY  ON" : "RELAY  OFF");
}

static void update_clock(lv_timer_t *timer)
{
	static unsigned int seconds;
	char time_text[16];

	(void)timer;
	seconds++;
	snprintf(time_text, sizeof(time_text), "%02u:%02u", (seconds / 60) % 24,
		 seconds % 60);
	set_label_text(clock_label, time_text);
}

static lv_obj_t *create_card(lv_obj_t *parent, int x, int y, int width, int height)
{
	lv_obj_t *card = lv_obj_create(parent);
	lv_obj_set_pos(card, x, y);
	lv_obj_set_size(card, width, height);
	lv_obj_set_style_bg_color(card, lv_color_hex(0x182235), LV_PART_MAIN);
	lv_obj_set_style_border_width(card, 1, LV_PART_MAIN);
	lv_obj_set_style_border_color(card, lv_color_hex(0x26364f), LV_PART_MAIN);
	lv_obj_set_style_radius(card, 12, LV_PART_MAIN);
	lv_obj_set_style_pad_all(card, 12, LV_PART_MAIN);
	lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
	return card;
}

static void create_dashboard(void)
{
	lv_obj_t *screen = lv_screen_active();
	lv_obj_set_style_bg_color(screen, lv_color_hex(0x0b1220), LV_PART_MAIN);
	lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_t *title = lv_label_create(screen);
	lv_label_set_text(title, "DVAU  |  CONTROL PANEL");
	lv_obj_set_pos(title, 18, 14);
	lv_obj_set_style_text_color(title, lv_color_hex(0xe2e8f0), LV_PART_MAIN);
	lv_obj_set_style_text_font(title, &lv_font_montserrat_20, LV_PART_MAIN);

	clock_label = lv_label_create(screen);
	lv_label_set_text(clock_label, "00:00");
	lv_obj_align(clock_label, LV_ALIGN_TOP_RIGHT, -18, 16);
	lv_obj_set_style_text_color(clock_label, lv_color_hex(0x60a5fa), LV_PART_MAIN);

	lv_obj_t *sensor = create_card(screen, 18, 55, 212, 106);
	lv_obj_t *caption = lv_label_create(sensor);
	lv_label_set_text(caption, "TEMPERATURE");
	lv_obj_set_style_text_color(caption, lv_color_hex(0x94a3b8), LV_PART_MAIN);
	temperature_label = lv_label_create(sensor);
	lv_label_set_text(temperature_label, "24.6 C");
	lv_obj_align(temperature_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);
	lv_obj_set_style_text_color(temperature_label, lv_color_hex(0x38bdf8), LV_PART_MAIN);
	lv_obj_set_style_text_font(temperature_label, &lv_font_montserrat_28, LV_PART_MAIN);

	lv_obj_t *network = create_card(screen, 250, 55, 212, 106);
	lv_obj_t *network_title = lv_label_create(network);
	lv_label_set_text(network_title, "NETWORK");
	lv_obj_set_style_text_color(network_title, lv_color_hex(0x94a3b8), LV_PART_MAIN);
	lv_obj_t *network_value = lv_label_create(network);
	lv_label_set_text(network_value, "ONLINE");
	lv_obj_align(network_value, LV_ALIGN_BOTTOM_LEFT, 0, 0);
	lv_obj_set_style_text_color(network_value, lv_color_hex(0x4ade80), LV_PART_MAIN);
	lv_obj_set_style_text_font(network_value, &lv_font_montserrat_28, LV_PART_MAIN);

	lv_obj_t *relay = lv_button_create(screen);
	lv_obj_set_pos(relay, 18, 181);
	lv_obj_set_size(relay, 212, 66);
	lv_obj_set_style_bg_color(relay, lv_color_hex(0x334155), LV_PART_MAIN);
	lv_obj_set_style_radius(relay, 12, LV_PART_MAIN);
	lv_obj_add_event_cb(relay, relay_event, LV_EVENT_CLICKED, NULL);
	lv_obj_t *relay_label = lv_label_create(relay);
	lv_label_set_text(relay_label, "RELAY  OFF");
	lv_obj_center(relay_label);

	lv_obj_t *status = create_card(screen, 250, 181, 212, 66);
	lv_obj_t *status_label = lv_label_create(status);
	lv_label_set_text(status_label, "DISPLAY: 480 x 272\nRGB666 framebuffer ready");
	lv_obj_set_style_text_color(status_label, lv_color_hex(0xcbd5e1), LV_PART_MAIN);
	lv_obj_center(status_label);
}

int main(int argc, char *argv[])
{
	const char *fbdev = "/dev/fb0";
	lv_display_t *display;

	if (argc == 3 && strcmp(argv[1], "--fbdev") == 0)
		fbdev = argv[2];
	else if (argc != 1) {
		fprintf(stderr, "Usage: %s [--fbdev /dev/fb0]\n", argv[0]);
		return 2;
	}

	signal(SIGINT, stop_handler);
	signal(SIGTERM, stop_handler);
	lv_init();
	display = lv_linux_fbdev_create();
	if (display == NULL) {
		fprintf(stderr, "Cannot create the LVGL framebuffer display\n");
		return 1;
	}
	lv_linux_fbdev_set_file(display, fbdev);
	lv_display_set_resolution(display, SCREEN_WIDTH, SCREEN_HEIGHT);
	create_dashboard();
	lv_timer_create(update_clock, 1000, NULL);

	while (keep_running) {
		lv_timer_handler();
		usleep(5000);
	}
	return 0;
}
