/*
MIT License

Copyright (c) 2022 Sukesh Ashok Kumar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ota.h"
#include "widgets/tux_panel.h"
#include <fmt/core.h>
#include <fmt/format.h>
#include "OpenWeatherMap.hpp"
#include "apps/weather/weathericons.h"
#include "events/gui_events.hpp"
#include <esp_partition.h>
#include <string>
#include "esp_http_client.h"
#include "esp_wifi.h"
#include "esp_system.h"


LV_IMG_DECLARE(dev_bg)
//LV_IMG_DECLARE(tux_logo)

// LV_FONT_DECLARE(font_7seg_64)
// LV_FONT_DECLARE(font_7seg_60)
// LV_FONT_DECLARE(font_7seg_58)
LV_FONT_DECLARE(font_7seg_56)

//LV_FONT_DECLARE(font_robotomono_12)
LV_FONT_DECLARE(font_robotomono_13)

LV_FONT_DECLARE(font_fa_14)
#define FA_SYMBOL_BLE "\xEF\x8A\x94"      // 0xf294
#define FA_SYMBOL_SETTINGS "\xEF\x80\x93" // 0xf0ad

/*********************
 *      DEFINES
 *********************/
#define HEADER_HEIGHT 30
#define FOOTER_HEIGHT 30

/******************
 *  LV DEFINES
 ******************/
static const lv_font_t *font_large;
static const lv_font_t *font_normal;
static const lv_font_t *font_symbol;
static const lv_font_t *font_fa;
static const lv_font_t *font_xl;
static const lv_font_t *font_xxl;

static lv_obj_t *panel_header;
static lv_obj_t *panel_title;
static lv_obj_t *panel_status; // Status icons in the header
static lv_obj_t *content_container;
static lv_obj_t *settings_container;
static lv_obj_t *screen_container;
static lv_obj_t *qr_status_container;

// TUX Panels
static lv_obj_t *tux_clock_weather;
static lv_obj_t *tux_db;
static lv_obj_t *tux_stop_clock;
static lv_obj_t *island_wifi;
static lv_obj_t *island_ota;
static lv_obj_t *island_devinfo;
static lv_obj_t *prov_qr;

static lv_obj_t *label_title;
static lv_obj_t *label_message;
static lv_obj_t *lbl_version;
static lv_obj_t *lbl_update_status;
static lv_obj_t *lbl_scan_status;

static lv_obj_t *lbl_device_info;

static lv_obj_t *icon_storage;
static lv_obj_t *icon_wifi;
static lv_obj_t *icon_ble;
static lv_obj_t *icon_battery;

/* Date/Time */
static lv_obj_t *lbl_time;
static lv_obj_t *lbl_time2;
static lv_obj_t *lbl_ampm;
static lv_obj_t *lbl_date;

/* Weather */
static lv_obj_t *lbl_weathericon;
static lv_obj_t *lbl_temp;
static lv_obj_t *lbl_temp2;
static lv_obj_t *lbl_hl;

static lv_obj_t *stopwatch;

static lv_obj_t *lbl_wifi_status;

static lv_coord_t screen_h;
static lv_coord_t screen_w;

static lv_obj_t *ui_Chart2;
static lv_obj_t *ui_Chart_rot;
static lv_obj_t *ui_Chart_db;
static lv_obj_t *ui_Chart_gelb;
static lv_obj_t *ui_Label2;

/******************
 *  LVL STYLES
 ******************/
static lv_style_t style_content_bg;

static lv_style_t style_message;
static lv_style_t style_title;
static lv_style_t style_iconstatus;
static lv_style_t style_storage;
static lv_style_t style_wifi;
static lv_style_t style_ble;
static lv_style_t style_battery;

static lv_style_t style_ui_island;
static lv_style_t lv_style_plain;

static lv_style_t style_glow;

/******************
 *  LVL ANIMATION
 ******************/
static lv_anim_t anim_labelscroll;

void anim_move_left_x(lv_obj_t * TargetObject, int start_x, int end_x, int delay);
void tux_anim_callback_set_x(lv_anim_t * a, int32_t v);

void anim_move_left_y(lv_obj_t * TargetObject, int start_y, int end_y, int delay);
void tux_anim_callback_set_y(lv_anim_t * a, int32_t v);

void anim_fade_in(lv_obj_t * TargetObject, int delay);
void anim_fade_out(lv_obj_t * TargetObject, int delay);

void tux_anim_callback_set_opacity(lv_anim_t * a, int32_t v);
/******************
 *  LVL FUNCS & EVENTS
 ******************/
static void create_page_home(lv_obj_t *parent);
static void create_page_settings(lv_obj_t *parent);
static void create_page_updates(lv_obj_t *parent);
static void create_page_remote(lv_obj_t *parent);

// Home page islands
static void tux_panel_clock_weather(lv_obj_t *parent);
static void tux_panel_db(lv_obj_t *parent);
static void tux_panel_stop_clock(lv_obj_t *parent);
static void tux_panel_config(lv_obj_t *parent);

// Setting page islands
static void tux_panel_devinfo(lv_obj_t *parent);
static void tux_panel_ota(lv_obj_t *parent);
static void tux_panel_wifi(lv_obj_t *parent);

static void create_header(lv_obj_t *parent);
static void create_footer(lv_obj_t *parent);

static void footer_message(const char *fmt, ...);
static void create_splash_screen();
static void switch_theme(bool dark);
static void qrcode_ui(lv_obj_t *parent);
static void show_ui();

static const char* get_firmware_version();

static void test_event_handler(lv_event_t *e);
static void theme_switch_event_handler(lv_event_t *e);
static void espwifi_event_handler(lv_event_t* e);
//static void espble_event_handler(lv_event_t *e);
static void checkupdates_event_handler(lv_event_t *e);
static void home_clicked_eventhandler(lv_event_t *e);
static void status_clicked_eventhandler(lv_event_t *e);
static void footer_button_event_handler(lv_event_t *e);

// static void new_theme_apply_cb(lv_theme_t * th, lv_obj_t * obj);

/* MSG Events */
void datetime_event_cb(lv_event_t * e);
void weather_event_cb(lv_event_t * e);
//void temp_event_cb(lv_event_t * e);
void db_draw_cb(lv_event_t * e);
void stopwatch_cb(lv_event_t * e);

static void status_change_cb(void * s, lv_msg_t *m);
static void lv_update_battery(uint batval);
static void set_weather_icon(string weatherIcon);

static int current_page = 0;


lv_chart_series_t* ui_Chart2_series_1;
lv_chart_series_t* ui_Chart2_series_gruen;
lv_chart_series_t* ui_Chart2_series_gelb;
lv_chart_series_t* ui_Chart2_series_rot;
static int counter = 0;

/*
lv_coord_t db_data[60] ;
static lv_chart_series_t * ser2;
//static int new_x = 0;
static int counter = 0;
static int db_index = 30;
lv_chart_series_t* ui_Chart2_series_1;

struct SensorData {
    tm timestamp;
    lv_coord_t db_value;
    //string bb_id;
    // Add more fields as needed for your specific sensor data
};
const int bufferSize = 60; // Assuming one reading per second for simplicity
SensorData sensorBuffer[bufferSize];
int bufferIndex;
*/



// HTTP POST request body
const char* post_data = "This is the body of the POST request";

// HTTP POST request handler
esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG,"HTTP_EVENT_ERROR\n");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG,"HTTP_EVENT_ON_CONNECTED\n");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG,"HTTP_EVENT_HEADER_SENT\n");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG,"HTTP_EVENT_ON_HEADER, key=%s, value=%s\n", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG,"HTTP_EVENT_ON_DATA, len=%d, data:%s\n", evt->data_len, evt->data);
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(TAG,"HTTP_EVENT_REDIRECT\n");
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG,"HTTP_EVENT_ON_FINISH\n");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG,"HTTP_EVENT_DISCONNECTED\n");
            break;
    }
    return ESP_OK;
}

void lv_setup_styles()
{
    font_symbol = &lv_font_montserrat_14;
    font_normal = &lv_font_montserrat_14;
    font_large = &lv_font_montserrat_16;
    font_xl = &lv_font_montserrat_24;
    font_xxl = &lv_font_montserrat_32;
    font_fa = &font_fa_14;

    screen_h = lv_obj_get_height(lv_scr_act());
    screen_w = lv_obj_get_width(lv_scr_act());

    /* CONTENT CONTAINER BACKGROUND */
    lv_style_init(&style_content_bg);
    
    //style_content_bg.scroll.ver = false;

    lv_style_set_bg_opa(&style_content_bg, LV_OPA_50);
    lv_style_set_radius(&style_content_bg, 0);

// Enabling wallpaper image slows down scrolling perf etc...
//#if defined(CONFIG_WALLPAPER_IMAGE)
#if defined(false)
    // Image Background
    // CF_INDEXED_8_BIT for smaller size - resolution 480x480
    // NOTE: Dynamic loading bg from SPIFF makes screen perf bad
    if (lv_fs_is_ready('F')) { // NO SD CARD load default
        ESP_LOGW(TAG,"Loading - F:/bg/dev_bg9.bin");
        lv_style_set_bg_img_src(&style_content_bg, "F:/bg/dev_bg9.bin");    
    } else {
        ESP_LOGW(TAG,"Loading - from firmware");
        lv_style_set_bg_img_src(&style_content_bg, &dev_bg);
    }
    //lv_style_set_bg_img_src(&style_content_bg, &dev_bg);
    // lv_style_set_bg_img_opa(&style_content_bg,LV_OPA_50);
#else
    ESP_LOGW(TAG,"Using Gradient");
    // Gradient Background
    static lv_grad_dsc_t grad;
    grad.dir = LV_GRAD_DIR_VER;
    grad.stops_count = 2;
    grad.stops[0].color = lv_color_make(31,32,34) ;
    grad.stops[1].color = lv_palette_main(LV_PALETTE_BLUE);
    grad.stops[0].frac = 150;
    grad.stops[1].frac = 190;
    lv_style_set_bg_grad(&style_content_bg, &grad);
#endif

    // DASHBOARD TITLE
    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_large);
    lv_style_set_align(&style_title, LV_ALIGN_LEFT_MID);
    lv_style_set_pad_left(&style_title, 15);
    lv_style_set_border_width(&style_title, 0);
    lv_style_set_size(&style_title, LV_SIZE_CONTENT);

    // HEADER STATUS ICON PANEL
    lv_style_init(&style_iconstatus);
    lv_style_set_size(&style_iconstatus, LV_SIZE_CONTENT);
    lv_style_set_pad_all(&style_iconstatus, 0);
    lv_style_set_border_width(&style_iconstatus, 0);
    lv_style_set_align(&style_iconstatus, LV_ALIGN_RIGHT_MID);
    lv_style_set_pad_right(&style_iconstatus, 15);

    lv_style_set_layout(&style_iconstatus, LV_LAYOUT_FLEX);
    lv_style_set_flex_flow(&style_iconstatus, LV_FLEX_FLOW_ROW);
    lv_style_set_flex_main_place(&style_iconstatus, LV_FLEX_ALIGN_CENTER);
    lv_style_set_flex_track_place(&style_iconstatus, LV_FLEX_ALIGN_SPACE_BETWEEN);
    lv_style_set_pad_row(&style_iconstatus, 3);

    // BATTERY
    lv_style_init(&style_battery);
    lv_style_set_text_font(&style_battery, font_symbol);
    lv_style_set_align(&style_battery, LV_ALIGN_RIGHT_MID);
    lv_style_set_text_color(&style_battery, lv_palette_main(LV_PALETTE_RED));

    // SD CARD
    lv_style_init(&style_storage);
    lv_style_set_text_font(&style_storage, font_symbol);
    lv_style_set_align(&style_storage, LV_ALIGN_RIGHT_MID);

    // WIFI
    lv_style_init(&style_wifi);
    lv_style_set_text_font(&style_wifi, font_symbol);
    lv_style_set_align(&style_wifi, LV_ALIGN_RIGHT_MID);

    // BLE
    lv_style_init(&style_ble);
    lv_style_set_text_font(&style_ble, font_fa);
    lv_style_set_align(&style_ble, LV_ALIGN_RIGHT_MID);

    // FOOTER MESSAGE & ANIMATION
    lv_anim_init(&anim_labelscroll);
    lv_anim_set_delay(&anim_labelscroll, 1000);        // Wait 1 second to start the first scroll
    lv_anim_set_repeat_delay(&anim_labelscroll, 3000); // Repeat the scroll 3 seconds after the label scrolls back to the initial position

    lv_style_init(&style_message);
    lv_style_set_anim(&style_message, &anim_labelscroll); // Set animation for the style
    // lv_style_set_text_color(&style_message, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_opa(&style_message, LV_OPA_COVER);
    lv_style_set_text_font(&style_message, font_normal);
    lv_style_set_align(&style_message, LV_ALIGN_LEFT_MID);
    lv_style_set_pad_left(&style_message, 15);
    lv_style_set_pad_right(&style_message, 15);

    // UI ISLANDS
    lv_style_init(&style_ui_island);
    lv_style_set_bg_color(&style_ui_island, bg_theme_color);
    lv_style_set_bg_opa(&style_ui_island, LV_OPA_80);
    lv_style_set_border_color(&style_ui_island, bg_theme_color);
    //lv_style_set_border_opa(&style_ui_island, LV_OPA_80);
    lv_style_set_border_width(&style_ui_island, 1);
    lv_style_set_radius(&style_ui_island, 10);

    // FOOTER NAV BUTTONS
    lv_style_init(&style_glow);
    lv_style_set_bg_opa(&style_glow, LV_OPA_COVER);
    lv_style_set_border_width(&style_glow,0);
    lv_style_set_bg_color(&style_glow, lv_palette_main(LV_PALETTE_RED));

    /*Add a shadow*/
    // lv_style_set_shadow_width(&style_glow, 10);
    // lv_style_set_shadow_color(&style_glow, lv_palette_main(LV_PALETTE_RED));
    // lv_style_set_shadow_ofs_x(&style_glow, 5);
    // lv_style_set_shadow_ofs_y(&style_glow, 5);    
}

static void create_header(lv_obj_t *parent)
{
    // HEADER PANEL
    panel_header = lv_obj_create(parent);
    lv_obj_set_size(panel_header, LV_PCT(100), HEADER_HEIGHT);
    lv_obj_set_style_pad_all(panel_header, 0, 0);
    lv_obj_set_style_radius(panel_header, 0, 0);
    lv_obj_set_align(panel_header, LV_ALIGN_TOP_MID);
    lv_obj_set_scrollbar_mode(panel_header, LV_SCROLLBAR_MODE_OFF);
    //lv_obj_set_style_bg_opa(panel_header, LV_OPA_TRANSP, 0);

    // HEADER TITLE PANEL
    panel_title = lv_obj_create(panel_header);
    lv_obj_add_style(panel_title, &style_title, 0);
    lv_obj_set_scrollbar_mode(panel_title, LV_SCROLLBAR_MODE_OFF);

    // HEADER TITLE
    label_title = lv_label_create(panel_title);
    lv_label_set_text(label_title, LV_SYMBOL_HOME " BoothBuddy");

    // HEADER STATUS ICON PANEL
    panel_status = lv_obj_create(panel_header);
    lv_obj_add_style(panel_status, &style_iconstatus, 0);
    lv_obj_set_scrollbar_mode(panel_status, LV_SCROLLBAR_MODE_OFF);

    // BLE
    icon_ble = lv_label_create(panel_status);
    lv_label_set_text(icon_ble, FA_SYMBOL_BLE);
    lv_obj_add_style(icon_ble, &style_ble, 0);

    // WIFI
    icon_wifi = lv_label_create(panel_status);
    lv_label_set_text(icon_wifi, LV_SYMBOL_WIFI);
    lv_obj_add_style(icon_wifi, &style_wifi, 0);

    // SD CARD
    icon_storage = lv_label_create(panel_status);
    lv_label_set_text(icon_storage, LV_SYMBOL_SD_CARD);
    lv_obj_add_style(icon_storage, &style_storage, 0);

    // BATTERY
    icon_battery = lv_label_create(panel_status);
    lv_label_set_text(icon_battery, LV_SYMBOL_CHARGE);
    lv_obj_add_style(icon_battery, &style_battery, 0);

    // lv_obj_add_event_cb(panel_title, home_clicked_eventhandler, LV_EVENT_CLICKED, NULL);
    // lv_obj_add_event_cb(panel_status, status_clicked_eventhandler, LV_EVENT_CLICKED, NULL);
}

static void create_footer(lv_obj_t *parent)
{
    lv_obj_t *panel_footer = lv_obj_create(parent);
    lv_obj_set_size(panel_footer, LV_PCT(100), FOOTER_HEIGHT);
    // lv_obj_set_style_bg_color(panel_footer, bg_theme_color, 0);
    lv_obj_set_style_pad_all(panel_footer, 0, 0);
    lv_obj_set_style_radius(panel_footer, 0, 0);
    lv_obj_set_align(panel_footer, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_scrollbar_mode(panel_footer, LV_SCROLLBAR_MODE_OFF);

/*
    // Create Footer label and animate if text is longer
    label_message = lv_label_create(panel_footer); // full screen as the parent
    lv_obj_set_width(label_message, LV_PCT(100));
    lv_label_set_long_mode(label_message, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_add_style(label_message, &style_message, LV_STATE_DEFAULT);
    lv_obj_set_style_align(label_message,LV_ALIGN_BOTTOM_LEFT,0);

    // Show LVGL version in the footer
    footer_message("A Touch UX Template using LVGL v%d.%d.%d", lv_version_major(), lv_version_minor(), lv_version_patch());
*/

    /* REPLACE STATUS BAR WITH BUTTON PANEL FOR NAVIGATION */
    //static const char * btnm_map[] = {LV_SYMBOL_HOME " HOME", LV_SYMBOL_KEYBOARD " REMOTE", FA_SYMBOL_SETTINGS " SETTINGS", LV_SYMBOL_DOWNLOAD " UPDATE", NULL};
    static const char * btnm_map[] = {LV_SYMBOL_HOME, LV_SYMBOL_KEYBOARD,FA_SYMBOL_SETTINGS, LV_SYMBOL_DOWNLOAD,  NULL};
    lv_obj_t * footerButtons = lv_btnmatrix_create(panel_footer);
    lv_btnmatrix_set_map(footerButtons, btnm_map);
    lv_obj_set_style_text_font(footerButtons,&lv_font_montserrat_16,LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(footerButtons,LV_OPA_TRANSP,0);
    lv_obj_set_size(footerButtons,LV_PCT(100),LV_PCT(100));
    lv_obj_set_style_border_width(footerButtons,0,LV_PART_MAIN | LV_PART_ITEMS);
    lv_btnmatrix_set_btn_ctrl_all(footerButtons, LV_BTNMATRIX_CTRL_CHECKABLE);
    
    //lv_obj_set_style_align(footerButtons,LV_ALIGN_TOP_MID,0);
    lv_btnmatrix_set_one_checked(footerButtons, true);   // only 1 button can be checked
    lv_btnmatrix_set_btn_ctrl(footerButtons,0,LV_BTNMATRIX_CTRL_CHECKED);

    // Very important but weird behavior
    lv_obj_set_height(footerButtons,FOOTER_HEIGHT+20);    
    lv_obj_set_style_radius(footerButtons,0,LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(footerButtons,LV_OPA_TRANSP,LV_PART_ITEMS);
    lv_obj_add_style(footerButtons, &style_glow,LV_PART_ITEMS | LV_BTNMATRIX_CTRL_CHECKED); // selected

    lv_obj_align(footerButtons, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(footerButtons, footer_button_event_handler, LV_EVENT_ALL, NULL); 
    
}

static void tux_panel_clock_weather(lv_obj_t *parent)
{
    // Create a new panel.
    tux_clock_weather = tux_panel_create(parent, "", 130);
    lv_obj_add_style(tux_clock_weather, &style_ui_island, 0);
    // Get the content of the panel.
    lv_obj_t *cont_panel = tux_panel_get_content(tux_clock_weather);
    lv_obj_set_flex_flow(tux_clock_weather, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tux_clock_weather, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // ************ Date/Time panel

    // Create a new panel for the date and time.
    lv_obj_t *cont_datetime = lv_obj_create(cont_panel);
    // Set the size of the date and time panel.
    lv_obj_set_size(cont_datetime, 180, 120);
    // Set the flex flow of the date and time panel to row wrap.
    lv_obj_set_flex_flow(cont_datetime, LV_FLEX_FLOW_ROW_WRAP);
    // Set the scrollbar mode of the date and time panel to off.
    lv_obj_set_scrollbar_mode(cont_datetime, LV_SCROLLBAR_MODE_OFF);
    // Align the date and time panel to the left and middle of the parent panel.
    lv_obj_align(cont_datetime, LV_ALIGN_LEFT_MID,0,0);
    // Set the background opacity of the date and time panel to transparent.
    lv_obj_set_style_bg_opa(cont_datetime,LV_OPA_TRANSP,0);
    // Set the border opacity of the date and time panel to transparent.
    lv_obj_set_style_border_opa(cont_datetime,LV_OPA_TRANSP,0);
    //lv_obj_set_style_pad_gap(cont_datetime,10,0);
    lv_obj_set_style_pad_top(cont_datetime,20,0);
    // MSG - MSG_TIME_CHANGED - EVENT
    // Create a new event callback for the date and time panel.
    lv_obj_add_event_cb(cont_datetime, datetime_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    // Subscribe the date and time panel to the MSG_TIME_CHANGED message.
    lv_msg_subsribe_obj(MSG_TIME_CHANGED, cont_datetime, NULL);
    

    // Create a new label for the time.
    lbl_time = lv_label_create(cont_datetime);
    // Set the alignment of the time label to top left.
    lv_obj_set_style_align(lbl_time, LV_ALIGN_TOP_LEFT, 0);
    // Set the font of the time label to the 7seg_56 font.
    lv_obj_set_style_text_font(lbl_time, &font_7seg_56, 0);
    // Set the text of the time label to "00:00".
    lv_label_set_text(lbl_time, "00:00");
    // Create a new label for the AM/PM.
    lbl_ampm = lv_label_create(cont_datetime);
    // Set the alignment of the AM/PM label to top left.
    lv_obj_set_style_align(lbl_ampm, LV_ALIGN_TOP_LEFT, 0);
    // Set the text of the AM/PM label to "AM".
    lv_label_set_text(lbl_ampm, "AM");
    // Create a new label for the date.
    lbl_date = lv_label_create(cont_datetime);
    // Set the alignment of the date label to bottom middle.
    lv_obj_set_style_align(lbl_date, LV_ALIGN_BOTTOM_MID, 0);
    // Set the font of the date label to the large font.
    lv_obj_set_style_text_font(lbl_date, font_large, 0);
    // Set the height of the date label to 30 pixels.
    lv_obj_set_height(lbl_date, 30);
    // Set the text of the date label to "waiting for update".
    lv_label_set_text(lbl_date, "waiting for update");
    // ************ Weather panel (panel widen with weekly forecast in landscape)

    // Create a new panel for the weather.
    lv_obj_t *cont_weather = lv_obj_create(cont_panel);
    lv_obj_set_size(cont_weather,100,115);
    lv_obj_set_flex_flow(cont_weather, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont_weather, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cont_weather, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align_to(cont_weather,cont_datetime,LV_ALIGN_OUT_RIGHT_MID,0,0);
    lv_obj_set_style_bg_opa(cont_weather,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_opa(cont_weather,LV_OPA_TRANSP,0);

    // MSG - MSG_WEATHER_CHANGED - EVENT
    lv_obj_add_event_cb(cont_weather, weather_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(MSG_WEATHER_CHANGED, cont_weather, NULL);

    // This only for landscape
    // lv_obj_t *lbl_unit = lv_label_create(cont_weather);
    // lv_obj_set_style_text_font(lbl_unit, font_normal, 0);
    // lv_label_set_text(lbl_unit, "Light rain");

    // Weather icons
    lbl_weathericon = lv_label_create(cont_weather);
    lv_obj_set_style_text_font(lbl_weathericon, &font_fa_weather_42, 0);
    // "F:/weather/cloud-sun-rain.bin");//10d@2x.bin"
    lv_label_set_text(lbl_weathericon, FA_WEATHER_SUN);
    lv_obj_set_style_text_color(lbl_weathericon,lv_palette_main(LV_PALETTE_ORANGE),0);

    // Temperature
    lbl_temp = lv_label_create(cont_weather);
    //lv_obj_set_style_text_font(lbl_temp, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_font(lbl_temp, font_xl, 0);
    lv_obj_set_style_align(lbl_temp, LV_ALIGN_BOTTOM_MID, 0);
    lv_label_set_text(lbl_temp, "0°C");

    lbl_hl = lv_label_create(cont_weather);
    lv_obj_set_style_text_font(lbl_hl, font_normal, 0);
    lv_obj_set_style_align(lbl_hl, LV_ALIGN_BOTTOM_MID, 0);
    lv_label_set_text(lbl_hl, "H:0° L:0°");
}
void tux_panel_stop_clock(lv_obj_t *parent) {
  // Create a new panel
  tux_stop_clock = tux_panel_create(parent, "", 130);
  lv_obj_add_style(tux_stop_clock, &style_ui_island, 0);

   lv_obj_t *stopwatch_panel = tux_panel_get_content(tux_stop_clock);
    lv_obj_set_flex_flow(tux_stop_clock, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tux_stop_clock, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  // Set the panel's title
  //tux_panel_set_title(panel, "Stop Clock");

  // Add a stop clock to the panel
  //lv_obj_t *stopwatch = lv_stopwatch_create(stopwatch_panel, NULL);
// Add the stopwatch object to the stopwatch panel.
 // lv_obj_add_to_parent(stopwatch, stopwatch_panel);
  // Start the stopwatch.
 // lv_stopwatch_start(stopwatch);
  // Set the stop clock's properties
  //tux_panel_stop_clock_set_format(stop_clock, "%H:%M:%S");

  // Add the panel to the screen
  //tux_panel_add_to_screen(tux_stop_clock);
}




static lv_obj_t * slider_label;
static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    lv_label_set_text_fmt(slider_label,"Brightness : %d",(int)lv_slider_get_value(slider));
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
    lcd.setBrightness((int)lv_slider_get_value(slider));
}

static void tux_panel_config(lv_obj_t *parent)
{
    /******** CONFIG & TESTING ********/
    lv_obj_t *island_2 = tux_panel_create(parent, LV_SYMBOL_EDIT " CONFIG", 200);
    lv_obj_add_style(island_2, &style_ui_island, 0);

    // Get Content Area to add UI elements
    lv_obj_t *cont_2 = tux_panel_get_content(island_2);

    lv_obj_set_flex_flow(cont_2, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_row(cont_2, 10, 0);
    //lv_obj_set_style_pad_column(cont_2, 5, 0);
    lv_obj_set_flex_align(cont_2, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END);

    // Screen Brightness
    /*Create a label below the slider*/
    slider_label = lv_label_create(cont_2);
    lv_label_set_text_fmt(slider_label, "Brightness : %d", lcd.getBrightness());   

    lv_obj_t * slider = lv_slider_create(cont_2);
    lv_obj_center(slider);
    lv_obj_set_size(slider, LV_PCT(90), 20);
    lv_slider_set_range(slider, 50 , 255);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_TOP_MID, 0, 30);
    lv_bar_set_value(slider,lcd.getBrightness(),LV_ANIM_ON);

    // THEME Selection
    lv_obj_t *label = lv_label_create(cont_2);
    lv_label_set_text(label, "Theme : Dark");
    //lv_obj_set_size(label, LV_PCT(90), 20);
    lv_obj_align_to(label,slider,LV_ALIGN_OUT_TOP_MID,0,15);
    //lv_obj_center(slider);
    
    lv_obj_t *sw = lv_switch_create(cont_2);
    lv_obj_add_event_cb(sw, theme_switch_event_handler, LV_EVENT_ALL, label);
    lv_obj_align_to(label, sw, LV_ALIGN_OUT_TOP_MID, 0, 20);
    //lv_obj_align(sw,LV_ALIGN_RIGHT_MID,0,0);

    // Rotate to Portait/Landscape
    lv_obj_t *btn2 = lv_btn_create(cont_2);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(btn2, LV_SIZE_CONTENT, 30);
    lv_obj_add_event_cb(btn2, test_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_t *lbl2 = lv_label_create(btn2);
    lv_label_set_text(lbl2, "try POST message");
    //lv_obj_center(lbl2);
    lv_obj_align_to(btn2, sw, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
}

// Provision WIFI
static void tux_panel_wifi(lv_obj_t *parent)
{
    /******** PROVISION WIFI ********/
    island_wifi = tux_panel_create(parent, LV_SYMBOL_WIFI " WIFI STATUS", 270);
    lv_obj_add_style(island_wifi, &style_ui_island, 0);
    // tux_panel_set_title_color(island_wifi, lv_palette_main(LV_PALETTE_BLUE));

    // Get Content Area to add UI elements
    lv_obj_t *cont_1 = tux_panel_get_content(island_wifi);
    lv_obj_set_flex_flow(cont_1, LV_FLEX_FLOW_COLUMN_WRAP);
    lv_obj_set_flex_align(cont_1, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lbl_wifi_status = lv_label_create(cont_1);
    lv_obj_set_size(lbl_wifi_status, LV_SIZE_CONTENT, 30);
    lv_obj_align(lbl_wifi_status, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_text(lbl_wifi_status, "Waiting for IP");

    // Check for Updates button
    lv_obj_t *btn_unprov = lv_btn_create(cont_1);
    lv_obj_set_size(btn_unprov, LV_SIZE_CONTENT, 40);
    lv_obj_align(btn_unprov, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *lbl2 = lv_label_create(btn_unprov);
    lv_label_set_text(lbl2, "Reset Wi-Fi Settings");
    lv_obj_center(lbl2);    

    /* ESP QR CODE inserted here */
    qr_status_container = lv_obj_create(cont_1);
    lv_obj_set_size(qr_status_container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(qr_status_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_ver(qr_status_container, 3, 0);
    lv_obj_set_style_border_width(qr_status_container, 0, 0);
    lv_obj_set_flex_flow(qr_status_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(qr_status_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_add_event_cb(btn_unprov, espwifi_event_handler, LV_EVENT_CLICKED, NULL);

    /* QR CODE */
    lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_GREY, 4);
    lv_color_t fg_color = lv_palette_darken(LV_PALETTE_BLUE, 4);

    prov_qr = lv_qrcode_create(qr_status_container, 100, fg_color, bg_color);

    /* Set data - format of BLE provisioning data */
    // {"ver":"v1","name":"TUX_4AA440","pop":"abcd1234","transport":"ble"}
    const char *qrdata = "https://github.com/sukesh-ak/ESP32-TUX";
    lv_qrcode_update(prov_qr, qrdata, strlen(qrdata));

    /*Add a border with bg_color*/
    lv_obj_set_style_border_color(prov_qr, bg_color, 0);
    lv_obj_set_style_border_width(prov_qr, 5, 0);

    lbl_scan_status = lv_label_create(qr_status_container);
    lv_obj_set_size(lbl_scan_status, LV_SIZE_CONTENT, 30);
    lv_label_set_text(lbl_scan_status, "Scan to learn about ESP32-TUX");

}

static void tux_panel_ota(lv_obj_t *parent)
{
    /******** OTA UPDATES ********/
    island_ota = tux_panel_create(parent, LV_SYMBOL_DOWNLOAD " OTA UPDATES", 180);
    lv_obj_add_style(island_ota, &style_ui_island, 0);

    // Get Content Area to add UI elements
    lv_obj_t *cont_ota = tux_panel_get_content(island_ota);
    lv_obj_set_flex_flow(cont_ota, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont_ota, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Current Firmware version
    lbl_version = lv_label_create(cont_ota);
    lv_obj_set_size(lbl_version, LV_SIZE_CONTENT, 30);
    lv_obj_align(lbl_version, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text_fmt(lbl_version, "Firmware Version %s",get_firmware_version());

    // Check for Updates button
    lv_obj_t *btn_checkupdates = lv_btn_create(cont_ota);
    lv_obj_set_size(btn_checkupdates, LV_SIZE_CONTENT, 40);
    lv_obj_align(btn_checkupdates, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *lbl2 = lv_label_create(btn_checkupdates);
    lv_label_set_text(lbl2, "Check for Updates");
    lv_obj_center(lbl2);
    lv_obj_add_event_cb(btn_checkupdates, checkupdates_event_handler, LV_EVENT_ALL, NULL);

    lv_obj_t *esp_updatestatus = lv_obj_create(cont_ota);
    lv_obj_set_size(esp_updatestatus, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(esp_updatestatus, LV_OPA_10, 0);
    lv_obj_set_style_border_width(esp_updatestatus, 0, 0);

    lbl_update_status = lv_label_create(esp_updatestatus);
    lv_obj_set_style_text_color(lbl_update_status, lv_palette_main(LV_PALETTE_YELLOW), 0);
    lv_obj_align(lbl_update_status, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(lbl_update_status, "Click to check for updates");
}

static void tux_panel_devinfo(lv_obj_t *parent)
{
    island_devinfo = tux_panel_create(parent, LV_SYMBOL_TINT " DEVICE INFO", 200);
    lv_obj_add_style(island_devinfo, &style_ui_island, 0);

    // Get Content Area to add UI elements
    lv_obj_t *cont_devinfo = tux_panel_get_content(island_devinfo);

    lbl_device_info = lv_label_create(cont_devinfo);
    // Monoaspace font for alignment
    lv_obj_set_style_text_font(lbl_device_info,&font_robotomono_13,0); 
}

static void create_page_remote(lv_obj_t *parent)
{

    static lv_style_t style;
    lv_style_init(&style);

    /*Set a background color and a radius*/
    lv_style_set_radius(&style, 10);
    lv_style_set_bg_opa(&style, LV_OPA_80);
    // lv_style_set_bg_color(&style, lv_palette_lighten(LV_PALETTE_GREY, 1));

    /*Add a shadow*/
    lv_style_set_shadow_width(&style, 55);
    lv_style_set_shadow_color(&style, lv_palette_main(LV_PALETTE_BLUE));

    lv_obj_t * island_remote = tux_panel_create(parent, LV_SYMBOL_KEYBOARD " REMOTE", LV_PCT(100));
    lv_obj_add_style(island_remote, &style_ui_island, 0);

    // Get Content Area to add UI elements
    lv_obj_t *cont_remote = tux_panel_get_content(island_remote);

    lv_obj_set_flex_flow(cont_remote, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont_remote, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(cont_remote, 10, 0);
    lv_obj_set_style_pad_row(cont_remote, 10, 0);

    uint32_t i;
    for(i = 0; i <12; i++) {
        lv_obj_t * obj = lv_btn_create(cont_remote);
        lv_obj_add_style(obj, &style, LV_STATE_PRESSED);
        lv_obj_set_size(obj, 80, 80);
        
        lv_obj_t * label = lv_label_create(obj);
        lv_label_set_text_fmt(label, "%" LV_PRIu32, i);
        lv_obj_center(label);
    }

}

static void create_page_home(lv_obj_t *parent)
{
    /* HOME PAGE PANELS */
    //tux_panel_stop_clock(parent);
    //tux_panel_db(parent);
    //tux_panel_clock_weather(parent);
    //tux_panel_devinfo(parent);  

}
static void create_page_chart(lv_obj_t *parent)
{
 



    ui_Chart_db = lv_chart_create(parent);
    lv_obj_set_width( ui_Chart_db, 500);   //10px rechts und links sind rand? display ist 480px breit
    lv_obj_set_height( ui_Chart_db, 300);
    lv_obj_set_style_border_width(ui_Chart_db, 0, 0);

    lv_obj_set_x( ui_Chart_db, 0 );
    lv_obj_set_y( ui_Chart_db, 0 );
    lv_obj_set_align( ui_Chart_db, LV_ALIGN_CENTER );
    lv_chart_set_type( ui_Chart_db, LV_CHART_TYPE_BAR);
    lv_chart_set_update_mode(ui_Chart_db, LV_CHART_UPDATE_MODE_SHIFT);
    //lv_obj_set_style_size(ui_Chart_db, 20, 40);

    lv_chart_set_point_count( ui_Chart_db, 480);                                                           //  160 240 480 TODO: automatisch berechnen
    lv_chart_set_div_line_count( ui_Chart_db, 0, 0);
    lv_obj_set_style_pad_gap(ui_Chart_db, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(ui_Chart_db,-1, LV_PART_ITEMS); //this can set the spacing between the series // -3  -2 -1
    lv_obj_set_style_pad_column(ui_Chart_db, 0, 0);
    lv_chart_set_axis_tick( ui_Chart_db, LV_CHART_AXIS_PRIMARY_X, 10, 5, 0, 0, false, 50);
    lv_chart_set_axis_tick( ui_Chart_db, LV_CHART_AXIS_PRIMARY_Y, 0, 5, 0, 0, false, 50);
    lv_chart_set_axis_tick( ui_Chart_db, LV_CHART_AXIS_SECONDARY_Y, 10, 5, 0, 0, false, 25);
    lv_chart_set_range(ui_Chart_db,LV_CHART_AXIS_PRIMARY_Y,0,1200);



    //ui_Chart2_series_1 = lv_chart_add_series(ui_Chart_db, lv_color_hex(0xff8080), LV_CHART_AXIS_PRIMARY_Y);
    ui_Chart2_series_gruen = lv_chart_add_series(ui_Chart_db, lv_color_hex(0x587b0d), LV_CHART_AXIS_PRIMARY_Y);
    ui_Chart2_series_gelb = lv_chart_add_series(ui_Chart_db, lv_color_hex(0xee9b2a), LV_CHART_AXIS_PRIMARY_Y);
    ui_Chart2_series_rot = lv_chart_add_series(ui_Chart_db, lv_color_hex(0xec270c), LV_CHART_AXIS_PRIMARY_Y);

    ui_Label2 = lv_label_create(parent);
    lv_obj_set_width( ui_Label2, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height( ui_Label2, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_align( ui_Label2, LV_ALIGN_CENTER );
    lv_label_set_text(ui_Label2,"no mic");
    lv_obj_set_style_text_font(ui_Label2, &lv_font_montserrat_48, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Label2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(ui_Label2, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label2, LV_TEXT_ALIGN_AUTO, LV_PART_MAIN| LV_STATE_DEFAULT);
    //lv_obj_set_style_text_font(ui_Label2, &lv_font_montserrat_48, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_Label2, 10, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Label2, lv_color_hex(0x434343), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_Label2, 150, LV_PART_MAIN| LV_STATE_DEFAULT);

    //static lv_coord_t db_data[] = { 0,10,20,20,20,20,20,20,20,0 };
    //lv_chart_set_ext_y_array(ui_Chart2, ui_Chart2_series_1, db_data );
    //get data
    //lv_obj_add_event_cb(parent, temp_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(MSG_TEMP_UPDATE, parent, NULL); 

    lv_obj_add_event_cb(parent, db_draw_cb, LV_EVENT_MSG_RECEIVED, NULL);
    //lv_msg_subsribe_obj(MSG_DRAW_UPDATE, parent, NULL); 
    
    /*ser2 = lv_chart_add_series(ui_Chart2, lv_palette_lighten(LV_PALETTE_GREY, 1), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_next_value(ui_Chart2, ser2, lv_rand(10, 80));
    lv_chart_set_next_value(ui_Chart2, ser2, lv_rand(10, 80));
    lv_chart_set_next_value(ui_Chart2, ser2, lv_rand(10, 80));
*/

    // Adjust the width and spacing of the series to overlap
    // lv_chart_set_series_width(ui_Chart2_series_1, 8);  // Set the width of each bar in the series
    // lv_chart_set_series_gap(ui_Chart2_series_1, 2);    // Set the spacing between each bar in the series    // Adjust the width and spacing of the series to overlap
    // lv_chart_set_series_width(ui_Chart2_series_gruen, 8);  // Set the width of each bar in the series
    // lv_chart_set_series_gap(ui_Chart2_series_gruen, 2);    // Set the spacing between each bar in the series

   // lv_obj_set_style_line_width(ui_Chart_db, 1, LV_PART_INDICATOR);
// Adjust the width and spacing of the bars within each series
    //lv_obj_set_style_pad_right();  // Set negative right padding
    //lv_obj_set_style_local_pad_inner(ui_Chart_db, LV_CHART_PART_BAR, LV_STATE_DEFAULT, -2);  // Set negative inner padding

}

static void create_page_settings(lv_obj_t *parent)
{
    /* SETTINGS PAGE PANELS */
    tux_panel_wifi(parent);
    tux_panel_config(parent);
}

static void create_page_updates(lv_obj_t *parent)
{
    /* OTA UPDATES PAGE PANELS */
    tux_panel_ota(parent);
    tux_panel_devinfo(parent);    
}

static void create_splash_screen()
{
    lv_obj_t * splash_screen = lv_scr_act();
    lv_obj_set_style_bg_color(splash_screen, lv_color_black(),0);
    lv_obj_t * splash_img = lv_img_create(splash_screen);
    lv_img_set_src(splash_img, "F:/bg/tux-logo.bin"); //&tux_logo);
    lv_obj_align(splash_img, LV_ALIGN_CENTER, 0, 0);

    //lv_scr_load_anim(splash_screen, LV_SCR_LOAD_ANIM_FADE_IN, 5000, 10, true);
    lv_scr_load(splash_screen);
}

static void show_ui()
{
    // screen_container is the root of the UX
    screen_container = lv_obj_create(NULL);

    lv_obj_set_size(screen_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(screen_container, 0, 0);
    lv_obj_align(screen_container, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_border_width(screen_container, 0, 0);
    lv_obj_set_scrollbar_mode(screen_container, LV_SCROLLBAR_MODE_OFF);

    // Gradient / Image Background for screen container
    //lv_obj_add_style(screen_container, &style_content_bg, 0);

    // HEADER & FOOTER
    create_header(screen_container);
    create_footer(screen_container);

    // CONTENT CONTAINER 
    content_container = lv_obj_create(screen_container);
    lv_obj_set_size(content_container, screen_w, screen_h - HEADER_HEIGHT - FOOTER_HEIGHT);
    lv_obj_align(content_container, LV_ALIGN_TOP_MID, 0, HEADER_HEIGHT);
    lv_obj_set_style_border_width(content_container, 0, 0);
    lv_obj_set_style_bg_opa(content_container, LV_OPA_TRANSP, 0);
    //lv_obj_set_scrollbar_mode(content_container, LV_SCROLLBAR_MODE_OFF);
    //lv_obj_clear_flag(content_container, LV_OBJ_FLAG_SCROLLABLE);
    //lv_obj_set_flex_flow(content_container, LV_FLEX_FLOW_COLUMN);


    create_page_chart(content_container);
    // Settings CONTAINER 
    settings_container = lv_obj_create(screen_container);
    lv_obj_set_size(settings_container, screen_w, screen_h - HEADER_HEIGHT - FOOTER_HEIGHT);
    lv_obj_align(settings_container, LV_ALIGN_TOP_MID, 0, HEADER_HEIGHT);
    lv_obj_set_style_border_width(settings_container, 0, 0);
    lv_obj_set_style_bg_opa(settings_container, LV_OPA_TRANSP, 0);
    //lv_obj_set_scrollbar_mode(content_container, LV_SCROLLBAR_MODE_OFF);
    //lv_obj_clear_flag(content_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(settings_container, LV_FLEX_FLOW_COLUMN);

    // Show Home Page
    //create_page_home(content_container);
    
    //create_header(content_container);
    //lv_obj_clean(content_container);

    //create_page_settings(settings_container);
    
    //create_page_remote(content_container);

    // Load main screen with animation
    //lv_scr_load(screen_container);
    lv_scr_load_anim(screen_container, LV_SCR_LOAD_ANIM_FADE_IN, 1000,100, true);

    // Status subscribers
    lv_msg_subsribe(MSG_WIFI_PROV_MODE, status_change_cb, NULL);    
    lv_msg_subsribe(MSG_WIFI_CONNECTED, status_change_cb, NULL);    
    lv_msg_subsribe(MSG_WIFI_DISCONNECTED, status_change_cb, NULL);    
    lv_msg_subsribe(MSG_OTA_STATUS, status_change_cb, NULL);    
    lv_msg_subsribe(MSG_SDCARD_STATUS, status_change_cb, NULL);  
    lv_msg_subsribe(MSG_BATTERY_STATUS, status_change_cb, NULL);  
    lv_msg_subsribe(MSG_DEVICE_INFO, status_change_cb, NULL);      

    // Send default page load notification => HOME
    lv_msg_send(MSG_PAGE_HOME,NULL);
}
void get_mac_address() {
    uint8_t mac[6];  // Buffer to store the MAC address

    esp_err_t err = esp_wifi_get_mac(WIFI_IF_AP, mac);

    if (err == ESP_OK) {
        ESP_LOGI(TAG,"MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } else {
        ESP_LOGI(TAG,"Failed to read MAC address: %s\n", esp_err_to_name(err));
    }
}
static void test_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);

    if (code == LV_EVENT_CLICKED)
    {
    // Get the MAC address
    get_mac_address();
       
    esp_http_client_config_t config = {
        .url = "http://192.168.188.34:3000/user/",
        .method = HTTP_METHOD_POST,
        .event_handler = http_event_handler,
        //.user_data = NULL
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Set POST data
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    // Perform the HTTP POST request
    esp_err_t err3 = esp_http_client_perform(client);
    if (err3 == ESP_OK) {
        ESP_LOGI(TAG,"HTTP POST request sent successfully\n");
    } else {
        ESP_LOGI(TAG,"HTTP POST request failed: %s\n", esp_err_to_name(err3));
    }

    // Cleanup
    esp_http_client_cleanup(client);
        // footer_message("%d,%d",screen_h,screen_w);
    }
}

static void theme_switch_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *udata = (lv_obj_t *)lv_event_get_user_data(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        LV_LOG_USER("State: %s\n", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
        if (lv_obj_has_state(obj, LV_STATE_CHECKED))
        {
            switch_theme(false);
            lv_label_set_text(udata, "Theme : Light");

            // Pass the new theme info
            // ESP_ERROR_CHECK(esp_event_post(TUX_EVENTS, TUX_EVENT_THEME_CHANGED, NULL,NULL, portMAX_DELAY));
        }
        else
        {
            switch_theme(true);
            lv_label_set_text(udata, "Theme : Dark");
            
            // Pass the new theme info
            // ESP_ERROR_CHECK(esp_event_post(TUX_EVENTS, TUX_EVENT_THEME_CHANGED, NULL,NULL, portMAX_DELAY));
        }
    }
}

static void footer_message(const char *fmt, ...)
{
    char buffer[200];
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    lv_label_set_text(label_message, buffer);
    va_end(args);
}

static void home_clicked_eventhandler(lv_event_t *e)
{
    // footer_message("Home clicked!");
    //  Clean the content container first
    lv_obj_clean(content_container);
    create_page_home(content_container);
}

static void status_clicked_eventhandler(lv_event_t *e)
{
    // footer_message("Status icons touched but this is a very long message to show scroll animation!");
    //  Clean the content container first
    //lv_obj_clean(content_container);
    //create_page_settings(settings_container);
    //create_page_remote(content_container);
}

void switch_theme(bool dark)
{
    if (dark)
    {
        theme_current = lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE),
                                              lv_palette_main(LV_PALETTE_GREEN),
                                              1, /*Light or dark mode*/
                                              &lv_font_montserrat_14);
        bg_theme_color = lv_palette_darken(LV_PALETTE_GREY, 5);
        lv_disp_set_theme(disp, theme_current);
        //bg_theme_color = theme_current->flags & LV_USE_THEME_DEFAULT ? lv_palette_darken(LV_PALETTE_GREY, 5) : lv_palette_lighten(LV_PALETTE_GREY, 2);
        // lv_theme_set_apply_cb(theme_current, new_theme_apply_cb);

        lv_style_set_bg_color(&style_ui_island, bg_theme_color);
        //lv_style_set_bg_opa(&style_ui_island, LV_OPA_80);

        ESP_LOGI(TAG,"Dark theme set");
    }
    else
    {
        theme_current = lv_theme_default_init(disp,
                                              lv_palette_main(LV_PALETTE_BLUE),
                                              lv_palette_main(LV_PALETTE_RED),
                                              0, /*Light or dark mode*/
                                              &lv_font_montserrat_14);
        //bg_theme_color = lv_palette_lighten(LV_PALETTE_GREY, 5);    // #BFBFBD
        // bg_theme_color = lv_color_make(0,0,255); 
        bg_theme_color = lv_color_hex(0xBFBFBD); //383837


        lv_disp_set_theme(disp, theme_current);
        // lv_theme_set_apply_cb(theme_current, new_theme_apply_cb);
        lv_style_set_bg_color(&style_ui_island, bg_theme_color);
        ESP_LOGI(TAG,"Light theme set");        

    }
}

// /*Will be called when the styles of the base theme are already added
//   to add new styles*/
// static void new_theme_apply_cb(lv_theme_t * th, lv_obj_t * obj)
// {
//     LV_UNUSED(th);

//     if(lv_obj_check_type(obj, &tux_panel_class)) {
//         lv_obj_add_style(obj, &style_ui_island, 0);
//         //lv_style_set_bg_color(&style_ui_island,theme_current->color_primary);
//     }

// }

static void espwifi_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        bool provisioned = false;
        ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));
        if (provisioned) {
            wifi_prov_mgr_reset_provisioning();     // reset wifi
            
            // Reset device to start provisioning
            lv_label_set_text(lbl_wifi_status, "Wi-Fi Disconnected!");
            lv_obj_set_style_text_color(lbl_wifi_status, lv_palette_main(LV_PALETTE_YELLOW), 0);
            lv_label_set_text(lbl_scan_status, "Restart device to provision WiFi.");
            lv_obj_add_state( btn, LV_STATE_DISABLED );  /* Disable */
        }
    }
}

inline void checkupdates_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        /*Get the first child of the button which is the label and change its text*/
        // Maybe disable the button and enable once completed
        //lv_obj_t *label = lv_obj_get_child(btn, 0);
        //lv_label_set_text_fmt(label, "Checking for updates...");
        LV_LOG_USER("Clicked");
        lv_msg_send(MSG_OTA_INITIATE,NULL);
    }
}

static const char* get_firmware_version()
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        return fmt::format("{}",running_app_info.version).c_str();
    }
    return "0.0.0";
}

void datetime_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    //lv_event_get_target(e) => cont_datetime
    lv_msg_t * m = lv_event_get_msg(e);
    
    // Not necessary but if event target was button or so, then required
    if (code == LV_EVENT_MSG_RECEIVED)  
    {
        struct tm *dtinfo = (tm*)lv_msg_get_payload(m);
        // Date & Time formatted
        char strftime_buf[64];
        // strftime(strftime_buf, sizeof(strftime_buf), "%c %z", dtinfo);
        // ESP_LOGW(TAG,"Triggered:datetime_event_cb %s",strftime_buf);

        // Date formatted
        strftime(strftime_buf, sizeof(strftime_buf), "%a, %e %b %Y", dtinfo);
        lv_label_set_text_fmt(lbl_date,"%s",strftime_buf);

        // Time in 12hrs 
        strftime(strftime_buf, sizeof(strftime_buf), "%I:%M", dtinfo);
        lv_label_set_text_fmt(lbl_time, "%s", strftime_buf);        

        // 12hr clock AM/PM
        strftime(strftime_buf, sizeof(strftime_buf), "%p", dtinfo);
        lv_label_set_text_fmt(lbl_ampm, "%s", strftime_buf);        
    }
}

void stopwatch_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_msg_t * m = lv_event_get_msg(e);
    
    // Not necessary but if event target was button or so, then required
    if (code == LV_EVENT_CLICKED)  
    {
        //OpenWeatherMap *e_owm = NULL;
        //e_owm = (OpenWeatherMap*)lv_msg_get_payload(m);
        //ESP_LOGW(TAG,"weather_event_cb %s",e_owm->LocationName.c_str());

        // set this according to e_owm->WeatherIcon 
        //set_weather_icon(e_owm->WeatherIcon);      

        lv_label_set_text(lbl_time2,"1234");
        ESP_LOGI(TAG, "clicked");

        //lv_label_set_text(lbl_hl,fmt::format("H:{:.1f}° L:{:.1f}°",e_owm->TemperatureHigh,e_owm->TemperatureLow).c_str());
    }
}

void weather_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_msg_t * m = lv_event_get_msg(e);
    
    // Not necessary but if event target was button or so, then required
    if (code == LV_EVENT_MSG_RECEIVED)  
    {
        OpenWeatherMap *e_owm = NULL;
        e_owm = (OpenWeatherMap*)lv_msg_get_payload(m);
        //ESP_LOGW(TAG,"weather_event_cb %s",e_owm->LocationName.c_str());

        // set this according to e_owm->WeatherIcon 
        set_weather_icon(e_owm->WeatherIcon);      

        lv_label_set_text(lbl_temp,fmt::format("{:.1f}°{}",e_owm->Temperature,e_owm->TemperatureUnit).c_str());
        lv_label_set_text(lbl_hl,fmt::format("H:{:.1f}° L:{:.1f}°",e_owm->TemperatureHigh,e_owm->TemperatureLow).c_str());
    }
}

// void temp_event_cb(lv_event_t * e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     lv_msg_t * m = lv_event_get_msg(e);

//       //  ESP_LOGW(TAG,"jdij");
//     // Not necessary but if event target was button or so, then required
//     if (code == LV_EVENT_MSG_RECEIVED)  
//     {
//         float tsens_value = *((float*)lv_msg_get_payload(m));
//         char temp_chip[20];
//         //lv_label_set_text(lbl_temp2,fmt::format("{:.1f}°C",tsens_value).c_str());
//         db_data[0] = (((static_cast<lv_coord_t>(tsens_value))-39)*10)+20;
//         lv_obj_invalidate(ui_Chart2);
//         lv_task_handler();
//     }
// }
void db_draw_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_msg_t * m = lv_event_get_msg(e);    
    unsigned int msg_id = lv_msg_get_id(m);

    
    // Not necessary but if event target was button or so, then required
    if (code == LV_EVENT_MSG_RECEIVED)  
    {
        if (msg_id == MSG_TEMP_UPDATE) {

            lv_coord_t tsens_value = *((lv_coord_t*)lv_msg_get_payload(m));
            char temp_chip[20];
            
            //db_data[db_index] = tsens_value;
            //db_index++;
            if (tsens_value<590) // 600
            {
                lv_chart_set_next_value(ui_Chart_db, ui_Chart2_series_gruen,tsens_value);
                lv_chart_set_next_value(ui_Chart_db, ui_Chart2_series_gelb,0);
                lv_chart_set_next_value(ui_Chart_db, ui_Chart2_series_rot,0);

            }
            else if (tsens_value<680)
            {
                lv_chart_set_next_value(ui_Chart_db, ui_Chart2_series_gruen,0);
                lv_chart_set_next_value(ui_Chart_db, ui_Chart2_series_gelb,tsens_value);
                lv_chart_set_next_value(ui_Chart_db, ui_Chart2_series_rot,0);
            }
            else{
                lv_chart_set_next_value(ui_Chart_db, ui_Chart2_series_gruen,0);
                lv_chart_set_next_value(ui_Chart_db, ui_Chart2_series_gelb,0);
                lv_chart_set_next_value(ui_Chart_db, ui_Chart2_series_rot,tsens_value);
            }
            
            
            std::string str_number = std::to_string(tsens_value);
            str_number.insert(str_number.length() - 1, "."); // add a dot 462 -> 46.2
            lv_label_set_text(ui_Label2,str_number.c_str());
            //sensorBuffer[17].db_value = tsens_value;
            //ESP_LOGI(TAG,"db: %d",tsens_value);

        }
        //TODO: use vertival scroll function of st7796s
        //new_x = new_x-1;
        //lv_obj_set_x( ui_Chart2, new_x );  //scrolling chart
        //counter++; 
        //if (counter == 300)   {
         //   counter=0;
        //    new_x = 0;

        //}
        //ESP_LOGI(TAG,"counter: %d",counter);

        // lv_chart_refresh(ui_Chart2);
        // ESP_LOGI(TAG,"counter2: %d",counter);
        // if (counter == 96)
        // {
        // ESP_LOGI(TAG,"abwerfen: %d",counter);
        // }
        
        //lv_obj_invalidate(ui_Chart2);
        //lv_task_handler();
        

    }
}

static void footer_button_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        uint32_t page_id = lv_btnmatrix_get_selected_btn(obj);
        const char * txt = lv_btnmatrix_get_btn_text(obj, page_id);
        printf("[%ld] %s was pressed\n", page_id,txt);

        // Do not refresh the page if its not changed
        if (current_page != page_id) current_page = page_id;
        else return;    // Skip if no page change

        // HOME
        if (page_id==MSG_PAGE_HOME)  {
            //lv_timer_pause(timer_temp);
            lv_obj_clean(content_container);
            lv_obj_clean(settings_container);
            create_page_chart(content_container);
            //lv_timer_resume(timer_temp);
            //anim_move_left_x(content_container,screen_w,0,200);
            lv_msg_send(MSG_PAGE_HOME,NULL);
        } 
        // REMOTE
        else if (page_id == MSG_PAGE_REMOTE) {
            //lv_timer_pause(timer_temp);
            lv_obj_clean(content_container);
            lv_obj_clean(settings_container);
            create_page_remote(content_container);
            //anim_move_left_x(content_container,screen_w,0,200);
            lv_msg_send(MSG_PAGE_REMOTE,NULL);
        }
        // SETTINGS
        else if (page_id == MSG_PAGE_SETTINGS) {
            //lv_timer_pause(timer_temp);
            lv_obj_clean(content_container);
            lv_obj_clean(settings_container);
            create_page_settings(settings_container);
            //anim_move_left_x(settings_container,screen_w,0,200);
            lv_msg_send(MSG_PAGE_SETTINGS,NULL);
        }
        // OTA UPDATES
        else if (page_id == MSG_PAGE_OTA) {
            //lv_timer_pause(timer_temp);
            lv_obj_clean(content_container);
            lv_obj_clean(settings_container);
            create_page_updates(content_container);
            //anim_move_left_x(content_container,screen_w,0,200);
            lv_msg_send(MSG_PAGE_OTA,NULL);
        }
    }
}

static void status_change_cb(void * s, lv_msg_t *m)
{
    LV_UNUSED(s);
    unsigned int msg_id = lv_msg_get_id(m);
    const char * msg_payload = (const char *)lv_msg_get_payload(m);
    const char * msg_data = (const char *)lv_msg_get_user_data(m);

    switch (msg_id)
    {
        case MSG_WIFI_PROV_MODE:
        {
            ESP_LOGW(TAG,"[%d] MSG_WIFI_PROV_MODE",msg_id);
            // Update QR code for PROV and wifi symbol to red?
            lv_style_set_text_color(&style_wifi, lv_palette_main(LV_PALETTE_GREY));
            lv_label_set_text(icon_wifi, LV_SYMBOL_WIFI);

            char qr_data[150] = {0};
            snprintf(qr_data,sizeof(qr_data),"%s",(const char*)lv_msg_get_payload(m));
            lv_qrcode_update(prov_qr, qr_data, strlen(qr_data));
            lv_label_set_text(lbl_scan_status, "Install 'ESP SoftAP Prov' App & Scan");
        }
        break;
        case MSG_WIFI_CONNECTED:
        {
            ESP_LOGW(TAG,"[%d] MSG_WIFI_CONNECTED",msg_id);
            lv_style_set_text_color(&style_wifi, lv_palette_main(LV_PALETTE_BLUE));
            lv_label_set_text(icon_wifi, LV_SYMBOL_WIFI);

            if (lv_msg_get_payload(m) != NULL) {
                char ip_data[20]={0};
                // IP address in the payload so display
                snprintf(ip_data,sizeof(ip_data),"%s",(const char*)lv_msg_get_payload(m));
                lv_label_set_text_fmt(lbl_wifi_status, "IP Address: %s",ip_data);
            }
        }
        break;
        case MSG_WIFI_DISCONNECTED:
        {
            ESP_LOGW(TAG,"[%d] MSG_WIFI_DISCONNECTED",msg_id);
            lv_style_set_text_color(&style_wifi, lv_palette_main(LV_PALETTE_GREY));
            lv_label_set_text(icon_wifi, LV_SYMBOL_WIFI);
        }
        break;
        case MSG_OTA_STATUS:
        {
            ESP_LOGW(TAG,"[%d] MSG_OTA_STATUS",msg_id);
            // Shows different status during OTA update
            char ota_data[150] = {0};
            snprintf(ota_data,sizeof(ota_data),"%s",(const char*)lv_msg_get_payload(m));
            lv_label_set_text(lbl_update_status, ota_data);
        }
        break;
        case MSG_SDCARD_STATUS:
        {
            bool sd_status = *(bool *)lv_msg_get_payload(m);
            ESP_LOGW(TAG,"[%d] MSG_SDCARD_STATUS %d",msg_id,sd_status);
            if (sd_status) {
                lv_style_set_text_color(&style_storage, lv_palette_main(LV_PALETTE_GREEN));
            } else {
                lv_style_set_text_color(&style_storage, lv_palette_main(LV_PALETTE_RED));
            }
        }
        break;
        /*case MSG_BATTERY_STATUS:
        {
            int battery_val = *(int *)lv_msg_get_payload(m);
            //ESP_LOGW(TAG,"[%d] MSG_BATTERY_STATUS %d",msg_id,battery_val);
            lv_update_battery(battery_val);
        }
        break;*/
        case MSG_DEVICE_INFO:
        {
            ESP_LOGW(TAG,"[%d] MSG_DEVICE_INFO",msg_id);
            char devinfo_data[300] = {0};
            snprintf(devinfo_data,sizeof(devinfo_data),"%s",(const char*)lv_msg_get_payload(m));
            lv_label_set_text(lbl_device_info,devinfo_data);
        }
        break;
    }
}

static void lv_update_battery(uint batval)
{
    if (batval < 20)
    {
        lv_style_set_text_color(&style_battery, lv_palette_main(LV_PALETTE_RED));
        lv_label_set_text(icon_battery, LV_SYMBOL_BATTERY_EMPTY);
    }
    else if (batval < 50)
    {
        lv_style_set_text_color(&style_battery, lv_palette_main(LV_PALETTE_RED));
        lv_label_set_text(icon_battery, LV_SYMBOL_BATTERY_1);
    }
    else if (batval < 70)
    {
        lv_style_set_text_color(&style_battery, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
        lv_label_set_text(icon_battery, LV_SYMBOL_BATTERY_2);
    }
    else if (batval < 90)
    {
        lv_style_set_text_color(&style_battery, lv_palette_main(LV_PALETTE_GREEN));
        lv_label_set_text(icon_battery, LV_SYMBOL_BATTERY_3);
    }
    else
    {
        lv_style_set_text_color(&style_battery, lv_palette_main(LV_PALETTE_GREEN));
        lv_label_set_text(icon_battery, LV_SYMBOL_BATTERY_FULL);
    }
}

/********************** ANIMATIONS *********************/
void anim_move_left_x(lv_obj_t * TargetObject, int start_x, int end_x, int delay)
{
    lv_anim_t property_anim;
    lv_anim_init(&property_anim);
    lv_anim_set_time(&property_anim, 200);
    lv_anim_set_user_data(&property_anim, TargetObject);
    lv_anim_set_custom_exec_cb(&property_anim, tux_anim_callback_set_x);
    lv_anim_set_values(&property_anim, start_x, end_x);
    lv_anim_set_path_cb(&property_anim, lv_anim_path_overshoot);
    lv_anim_set_delay(&property_anim, delay + 0);
    lv_anim_set_playback_time(&property_anim, 0);
    lv_anim_set_playback_delay(&property_anim, 0);
    lv_anim_set_repeat_count(&property_anim, 0);
    lv_anim_set_repeat_delay(&property_anim, 0);
    lv_anim_set_early_apply(&property_anim, true);
    lv_anim_start(&property_anim);
}

void tux_anim_callback_set_x(lv_anim_t * a, int32_t v)
{
    lv_obj_set_x((lv_obj_t *)a->user_data, v);
}

void anim_move_left_y(lv_obj_t * TargetObject, int start_y, int end_y, int delay)
{
    lv_anim_t property_anim;
    lv_anim_init(&property_anim);
    lv_anim_set_time(&property_anim, 300);
    lv_anim_set_user_data(&property_anim, TargetObject);
    lv_anim_set_custom_exec_cb(&property_anim, tux_anim_callback_set_y);
    lv_anim_set_values(&property_anim, start_y, end_y);
    lv_anim_set_path_cb(&property_anim, lv_anim_path_overshoot);
    lv_anim_set_delay(&property_anim, delay + 0);
    lv_anim_set_playback_time(&property_anim, 0);
    lv_anim_set_playback_delay(&property_anim, 0);
    lv_anim_set_repeat_count(&property_anim, 0);
    lv_anim_set_repeat_delay(&property_anim, 0);
    lv_anim_set_early_apply(&property_anim, true);
    lv_anim_start(&property_anim);
}

void tux_anim_callback_set_y(lv_anim_t * a, int32_t v)
{
    lv_obj_set_y((lv_obj_t *)a->user_data, v);
}

void anim_fade_in(lv_obj_t * TargetObject, int delay)
{
    lv_anim_t property_anim;
    lv_anim_init(&property_anim);
    lv_anim_set_time(&property_anim, 3000);
    lv_anim_set_user_data(&property_anim, TargetObject);
    lv_anim_set_custom_exec_cb(&property_anim, tux_anim_callback_set_opacity);
    lv_anim_set_values(&property_anim, 0, 255);
    lv_anim_set_path_cb(&property_anim, lv_anim_path_ease_out);
    lv_anim_set_delay(&property_anim, delay + 0);
    lv_anim_set_playback_time(&property_anim, 0);
    lv_anim_set_playback_delay(&property_anim, 0);
    lv_anim_set_repeat_count(&property_anim, 0);
    lv_anim_set_repeat_delay(&property_anim, 0);
    lv_anim_set_early_apply(&property_anim, false);
    lv_anim_start(&property_anim);

}
void anim_fade_out(lv_obj_t * TargetObject, int delay)
{
    lv_anim_t property_anim;
    lv_anim_init(&property_anim);
    lv_anim_set_time(&property_anim, 1000);
    lv_anim_set_user_data(&property_anim, TargetObject);
    lv_anim_set_custom_exec_cb(&property_anim, tux_anim_callback_set_opacity);
    lv_anim_set_values(&property_anim, 255, 0);
    lv_anim_set_path_cb(&property_anim, lv_anim_path_ease_in_out);
    lv_anim_set_delay(&property_anim, delay + 0);
    lv_anim_set_playback_time(&property_anim, 0);
    lv_anim_set_playback_delay(&property_anim, 0);
    lv_anim_set_repeat_count(&property_anim, 0);
    lv_anim_set_repeat_delay(&property_anim, 0);
    lv_anim_set_early_apply(&property_anim, false);
    lv_anim_start(&property_anim);

}
void tux_anim_callback_set_opacity(lv_anim_t * a, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)a->user_data, v, 0);
}

static void set_weather_icon(string weatherIcon)
{
    /* 
        https://openweathermap.org/weather-conditions#Weather-Condition-Codes-2
        d = day / n = night
        01 - clear sky
        02 - few clouds
        03 - scattered clouds
        04 - broken clouds
        09 - shower rain
        10 - rain
        11 - thunderstorm
        13 - snow
        50 - mist
    */
    // lv_color_make(red, green, blue);

    if (weatherIcon.find('d') != std::string::npos) {
        // set daytime color
        // color = whitesmoke = lv_color_make(245, 245, 245)
        // Ideally it should change for each weather - light blue for rain etc...
        lv_obj_set_style_text_color(lbl_weathericon,lv_color_make(241, 235, 156),0); 
    } else {
        // set night time color
        lv_obj_set_style_text_color(lbl_weathericon,lv_palette_main(LV_PALETTE_BLUE_GREY),0);
    }

    if (weatherIcon.find("50") != std::string::npos) {     // mist - need icon
        lv_label_set_text(lbl_weathericon,FA_WEATHER_DROPLET);
        return;
    }
    
    if (weatherIcon.find("13") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_SNOWFLAKES);
        return;
    }    

    if (weatherIcon.find("11") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_CLOUD_SHOWERS_HEAVY);
        return;
    }    

    if (weatherIcon.find("10") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_CLOUD_RAIN);
        return;
    }    

    if (weatherIcon.find("09") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_CLOUD_RAIN);
        return;
    }    

    if (weatherIcon.find("04") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_CLOUD);
        return;
    }   

    if (weatherIcon.find("03") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_CLOUD);
        return;
    }

    if (weatherIcon.find("02") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_CLOUD);
        return;
    }

    if (weatherIcon.find("01") != std::string::npos) {     
        lv_label_set_text(lbl_weathericon,FA_WEATHER_SUN);
        return;
    }


    // default
    lv_label_set_text(lbl_weathericon,FA_WEATHER_CLOUD_SHOWERS_HEAVY);
    lv_obj_set_style_text_color(lbl_weathericon,lv_palette_main(LV_PALETTE_BLUE_GREY),0); 


}