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

static const char *TAG = "ESP32-TUX";
#include "main.hpp"
#include <iostream>
#include <string>
#include <helper_modbus.hpp>
#include "driver/gpio.h"
#include <cmath>
#include <cstdlib>
//#define GPIO_PIN  GPIO_NUM_11
#define BUFFER_SIZE 25


int count_read_db =0;
lv_coord_t db_value = 0;

// Sensor reading structure
typedef struct {
    // Define your sensor reading data structure here
    // For example:
    lv_coord_t value;
    int square_value;
    // Add any additional sensor data you need
} SensorReading;

// Ring buffer structure
typedef struct {
    SensorReading buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    int avg5s;
} RingBuffer;

RingBuffer ringBuffer ;

// Initialize the ring buffer
void initRingBuffer(RingBuffer* ringBuffer) {
    ringBuffer->head = 0;
    ringBuffer->tail = 0;
    ringBuffer->count = 0;
    ringBuffer->avg5s = 0;
}

double calculateSquareAverage(RingBuffer* ringBuffer) {
    double sum = 0;
    int count = ringBuffer->count;

    for (int i = 0; i < count; ++i) {
        sum += ringBuffer->buffer[i].square_value;
    }

    return (count > 0) ? (sum / count) : 0.0;
}

void update_5s_avg(RingBuffer* ringBuffer) {
    //calculateSquareAverage(ringBuffer)
    double avg=calculateSquareAverage(ringBuffer);
    ringBuffer->avg5s = sqrt(avg);
}

// Add a sensor reading to the ring buffer
void addToRingBuffer(RingBuffer* ringBuffer, SensorReading reading) {
    ringBuffer->buffer[ringBuffer->head] = reading;
    ringBuffer->head = (ringBuffer->head + 1) % BUFFER_SIZE;
    if (ringBuffer->count < BUFFER_SIZE) {
        ringBuffer->count++;
    } else {
        ringBuffer->tail = (ringBuffer->tail + 1) % BUFFER_SIZE;
    }
}

static void set_timezone()
{
    // Update local timezone
    //setenv("TZ", cfg->TimeZone, 1);
    setenv("TZ", CONFIG_TIMEZONE_STRING, 1);
    tzset();    
}

// GEt time from internal RTC and update date/time of the clock
static void update_datetime_ui()
{
    // If we are on another screen where lbl_time is not valid
    //if (!lv_obj_is_valid(lbl_time)) return;

    // date/time format reference => https://cplusplus.com/reference/ctime/strftime/
    time_t now;
    struct tm datetimeinfo;
    time(&now);
    localtime_r(&now, &datetimeinfo);

    // tm_year will be (1970 - 1900) = 70, if not set
    if (datetimeinfo.tm_year < 100) // Time travel not supported :P
    {
        // ESP_LOGD(TAG, "Date/time not set yet [%d]",datetimeinfo.tm_year);    
        return;
    }

    // Send update time to UI with payload using lv_msg
    lv_msg_send(MSG_TIME_CHANGED, &datetimeinfo);
}
// GEt time from internal RTC and update date/time of the clock
static void update_temp_ui(SensorReading db_value_sensor)
{
    // If we are on another screen where lbl_time is not valid
    //if (!lv_obj_is_valid(lbl_time)) return;

    // date/time format reference => https://cplusplus.com/reference/ctime/strftime/
    time_t now;
    struct tm datetimeinfo;
    time(&now);
    localtime_r(&now, &datetimeinfo);

    // // tm_year will be (1970 - 1900) = 70, if not set
    // if (datetimeinfo.tm_year < 100) // Time travel not supported :P
    // {
    //     // ESP_LOGD(TAG, "Date/time not set yet [%d]",datetimeinfo.tm_year);    
    //     return;
    // }
    // ESP_LOGW(TAG,"TEMP_TIMER_2222222222");
    //  ESP_LOGI(TAG, "Read temperature");
    // int cnt = 20;
    // float tsens_value;
    // while (cnt--) {
    //     temperature_sensor_get_celsius(temp_sensor, &tsens_value);
    //     ESP_LOGI(TAG, "Temperature value %.02f ℃", tsens_value);
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }

    // Send update time to UI with payload using lv_msg
    //lv_msg_send(MSG_TIME_CHANGED, &datetimeinfo);
    //ESP_LOGI(TAG, "Read temperature");
    //float tsens_value;
    
    lv_coord_t db_value = db_value_sensor.value;
    //string tsens_string;
        //ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_sensor, &tsens_value));
        
        //MODBUS get db
        //ESP_ERROR_CHECK(master_init()); //
        //db_value = master_operation_func(NULL);

        //ESP_LOGI(TAG, "db(A) value %.02d db(A)", db_value);
        //ESP_LOGI(TAG, "Temperature value %.02f ℃", tsens_value);
        // Store the current reading in the buffer
        //sensorBuffer[bufferIndex].timestamp = datetimeinfo;
        //sensorBuffer[bufferIndex].db_value = db_value;
        // for (int i = 0;i<bufferIndex;i++) {
        //     ESP_LOGI(TAG, "db %.02f ",  sensorBuffer[i].db_value);

        // }
        //db_value = db_value + 1;
        //addToRingBuffer(ringBuffer, db_value);
        //ESP_LOGI(TAG, "ringbuffer %.02f", ringBuffer[1]);
        // Increment the buffer index and handle wraparound
        //bufferIndex = (bufferIndex + 1) % bufferSize;
        lv_msg_send(MSG_TEMP_UPDATE, &db_value);
}

static const char* get_id_string(esp_event_base_t base, int32_t id) {
    //if (base == TUX_EVENTS) {
        switch(id) {
            case TUX_EVENT_DATETIME_SET:
                return "TUX_EVENT_DATETIME_SET";
            case TUX_EVENT_OTA_STARTED:
                return "TUX_EVENT_OTA_STARTED";
            case TUX_EVENT_OTA_IN_PROGRESS:
                return "TUX_EVENT_OTA_IN_PROGRESS";
            case TUX_EVENT_OTA_ROLLBACK:
                return "TUX_EVENT_OTA_ROLLBACK";
            case TUX_EVENT_OTA_COMPLETED:
                return "TUX_EVENT_OTA_COMPLETED";
            case TUX_EVENT_OTA_FAILED:
                return "TUX_EVENT_OTA_FAILED";
            case TUX_EVENT_OTA_ABORTED:
                return "TUX_EVENT_OTA_ABORTED";
            case TUX_EVENT_WEATHER_UPDATED:
                return "TUX_EVENT_WEATHER_UPDATED";
            case TUX_EVENT_THEME_CHANGED:
                return "TUX_EVENT_THEME_CHANGED";
            default:
                return "TUX_EVENT_UNKNOWN";        
        }
    //} 
}

// tux event handler
static void tux_event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    ESP_LOGD(TAG, "tux_event_handler => %s:%s", event_base, get_id_string(event_base, event_id));
    if (event_base != TUX_EVENTS) return;   // bye bye - me not invited :(

    // Handle TUX_EVENTS
    if (event_id == TUX_EVENT_DATETIME_SET) {

        set_timezone();

        // update clock
        update_datetime_ui();

        // Enable timer after the date/time is set.
        lv_timer_ready(timer_weather); 
        //lv_timer_ready(timer_temp); 

    } else if (event_id == TUX_EVENT_OTA_STARTED) {
        // OTA Started
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s",(char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_OTA_IN_PROGRESS) {
        // OTA In Progress - progressbar?
        char buffer[150] = {0};
        int bytes_read = (*(int *)event_data)/1024;
        snprintf(buffer,sizeof(buffer),"OTA: Data read : %dkb", bytes_read);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_OTA_ROLLBACK) {
        // OTA Rollback - god knows why!
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s", (char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_OTA_COMPLETED) {
        // OTA Completed - YAY! Success
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s", (char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

        // wait before reboot
        vTaskDelay(3000 / portTICK_PERIOD_MS);

    } else if (event_id == TUX_EVENT_OTA_ABORTED) {
        // OTA Aborted - Not a good day for updates
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s", (char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_OTA_FAILED) {
        // OTA Failed - huh! - maybe in red color?
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s", (char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_WEATHER_UPDATED) {
        // Weather updates - summer?

    } else if (event_id == TUX_EVENT_THEME_CHANGED) {
        // Theme changes - time to play with dark & light
    }
}                          

// Wifi & IP related event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    //ESP_LOGD(TAG, "%s:%s: wifi_event_handler", event_base, get_id_string(event_base, event_id));
    if (event_base == WIFI_EVENT  && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        is_wifi_connected = true;
        lv_timer_ready(timer_datetime);   // start timer

        // After OTA device restart, RTC will have time but not timezone
        set_timezone();

        // Not a warning but just for highlight
        ESP_LOGW(TAG,"WIFI_EVENT_STA_CONNECTED");
        lv_msg_send(MSG_WIFI_CONNECTED,NULL);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        is_wifi_connected = false;        
        lv_timer_pause(timer_datetime);   // stop/pause timer

        ESP_LOGW(TAG,"WIFI_EVENT_STA_DISCONNECTED");
        lv_msg_send(MSG_WIFI_DISCONNECTED,NULL);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        is_wifi_connected = true;
        lv_timer_ready(timer_datetime);   // start timer

        ESP_LOGW(TAG,"IP_EVENT_STA_GOT_IP");
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;

        snprintf(ip_payload,sizeof(ip_payload),"%d.%d.%d.%d", IP2STR(&event->ip_info.ip));
        
        // We got IP, lets update time from SNTP. RTC keeps time unless powered off
        xTaskCreate(configure_time, "config_time", 1024*4, NULL, 3, NULL);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP)
    {
        is_wifi_connected = false;
        ESP_LOGW(TAG,"IP_EVENT_STA_LOST_IP");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_START) {
        ESP_LOGW(TAG,"WIFI_PROV_START");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_CRED_RECV) {
        ESP_LOGW(TAG,"WIFI_PROV_CRED_RECV");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_CRED_FAIL) {
        ESP_LOGW(TAG,"WIFI_PROV_CRED_FAIL");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_CRED_SUCCESS) {
        ESP_LOGW(TAG,"WIFI_PROV_CRED_SUCCESS");
        // FIXME Refresh IP details once provision is successfull
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_END) {
        ESP_LOGW(TAG,"WIFI_PROV_END");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_SHOWQR) {
        ESP_LOGW(TAG,"WIFI_PROV_SHOWQR");
        strcpy(qr_payload,(char*)event_data);   // Add qr payload to the variable
    }
}
// Function to initialize the GPIO
// void init_gpio() {
//     //gpio_pad_select_gpio(GPIO_PIN);
//     gpio_set_direction(GPIO_PIN, GPIO_MODE_OUTPUT);
// }
extern "C" void app_main(void)
{
    //init_gpio();
    // Create and initialize the ring buffer
    //RingBuffer ringBuffer;
    initRingBuffer(&ringBuffer);

    esp_log_level_set(TAG, ESP_LOG_DEBUG);      // enable DEBUG logs for this App
    //esp_log_level_set("SettingsConfig", ESP_LOG_DEBUG);    
    esp_log_level_set("wifi", ESP_LOG_WARN);    // enable WARN logs from WiFi stack

    // Print device info
    ESP_LOGE(TAG,"\n%s",device_info().c_str());

    //Initialize NVS
    esp_err_t err = nvs_flash_init();

    // NVS partition contains new data format or unable to access
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err); 

    // Init SPIFF - needed for lvgl images
    init_spiff();

#ifdef SD_SUPPORTED
    // Initializing SDSPI 
    if (init_sdspi() == ESP_OK) // SD SPI
    {
        is_sdcard_enabled = true;
    }
#endif   
//********************** CONFIG HELPER TESTING STARTS

     //cfg = new SettingsConfig("/sdcard/settings.json");    // yet to test
    cfg = new SettingsConfig("/spiffs/settings.json");
    // Save settings
    cfg->save_config();   // save default loaded settings
    // Load values
    cfg->load_config();
    // Change device name
    cfg->DeviceName = "ESP32-TUX";
    cfg->WeatherAPIkey = CONFIG_WEATHER_API_KEY;
    cfg->WeatherLocation = CONFIG_WEATHER_LOCATION;
    cfg->WeatherProvider = CONFIG_WEATHER_OWM_URL;
    // Change brightness
    cfg->Brightness=250;
    // Save settings again
    cfg->save_config();
    cfg->load_config();

//******************************************** 
    owm = new OpenWeatherMap();
//********************** CONFIG HELPER TESTING ENDS

    lcd.init();         // Initialize LovyanGFX
    lcd.initDMA();      // Init DMA
    lv_init();          // Initialize lvgl

    if (lv_display_init() != ESP_OK) // Configure LVGL
    {
        ESP_LOGE(TAG, "LVGL setup failed!!!");
    }

    // /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Register for event handler for Wi-Fi, IP related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    
    /* Events related to provisioning */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    // TUX EVENTS
    ESP_ERROR_CHECK(esp_event_handler_instance_register(TUX_EVENTS, ESP_EVENT_ANY_ID, tux_event_handler, NULL, NULL));

    // LV_FS integration & print readme.txt from the root for testing
    lv_print_readme_txt("F:/readme.txt");   // SPIFF / FAT
    lv_print_readme_txt("S:/readme.txt");   // SDCARD

    //db stuff
    //bufferIndex = 0;



/* Push LVGL/UI to its own UI task later*/
    // Splash screen
    lvgl_acquire();
    create_splash_screen();
    lvgl_release();

    // Main UI
    lvgl_acquire();
    lv_setup_styles();    
    show_ui();
    lvgl_release();

    

/* Push these to its own UI task later*/

#ifdef SD_SUPPORTED
    // Icon status color update
    lv_msg_send(MSG_SDCARD_STATUS,&is_sdcard_enabled);
#endif

    // Wifi Provision and connection.
    // Use idf.py menuconfig to configure 
    // Use SoftAP only / BLE has some issues
    // Tuning PSRAM options visible only in IDF5, so will wait till then for BLE.
    xTaskCreate(provision_wifi, "wifi_prov", 1024*8, NULL, 3, NULL);

    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());

    // Date/Time update timer - once per sec
    timer_datetime = lv_timer_create(timer_datetime_callback, 1000,  NULL);
    timer_temp = lv_timer_create(timer_temp_callback, 200,  NULL);
    //start temp timer
    lv_timer_ready(timer_temp);   // start timer
    //lv_timer_pause(timer_datetime); // enable only when wifi is connected

    // Weather update timer - Once per min (60*1000) or maybe once in 10 mins (10*60*1000)
    timer_weather = lv_timer_create(timer_weather_callback, WEATHER_UPDATE_INTERVAL,  NULL);

    //lv_timer_set_repeat_count(timer_weather,1);
    //lv_timer_pause(timer_weather);  // enable after wifi is connected

    // Subscribe to page change events in the UI
    /* SPELLING MISTAKE IN API BUG => https://github.com/lvgl/lvgl/issues/3822 */
    lv_msg_subsribe(MSG_PAGE_HOME, tux_ui_change_cb, NULL);
    lv_msg_subsribe(MSG_PAGE_REMOTE, tux_ui_change_cb, NULL);
    lv_msg_subsribe(MSG_PAGE_SETTINGS, tux_ui_change_cb, NULL);
    lv_msg_subsribe(MSG_PAGE_OTA, tux_ui_change_cb, NULL);
    lv_msg_subsribe(MSG_OTA_INITIATE, tux_ui_change_cb, NULL);    // Initiate OTA
    // TEMP     SENSOR
    /*
    ESP_LOGI(TAG, "Install temperature sensor, expected temp ranger range: 10~50 ℃");
    temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
    ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor_config, &temp_sensor));

    ESP_LOGI(TAG, "Enable temperature sensor");
    ESP_ERROR_CHECK(temperature_sensor_enable(temp_sensor));
    ESP_LOGI(TAG, "Read temperature");
    float tsens_value;
    
        ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_sensor, &tsens_value));
        ESP_LOGI(TAG, "Temperature value %.02f ℃", tsens_value);
        //vTaskDelay(pdMS_TO_TICKS(1000));
    */
    
}

static void timer_datetime_callback(lv_timer_t * timer)
{
    // Battery icon animation
    if (battery_value>100) battery_value=0;
    battery_value+=10;
    
    lv_msg_send(MSG_BATTERY_STATUS,&battery_value);
    update_datetime_ui();
}

static void timer_temp_callback(lv_timer_t * timer)
{
    
    // Read the sensor
    SensorReading db_value;
    lv_coord_t db_chart;
    
    #ifdef NO_MIC
    db_value.value =  std::rand() % 510 + 500; //500 = 50db

    #else
    //MODBUS get db
    ESP_ERROR_CHECK(master_init()); //
    db_value.value = master_operation_func(NULL);

    #endif

    db_value.square_value = db_value.value * db_value.value;
    //ESP_LOGI(TAG, "db value %d", db_value.value);
    
    if (db_value.value > 100)
    {
        /* code */
    
    
        addToRingBuffer(&ringBuffer, db_value);
        update_5s_avg(&ringBuffer);
        db_chart =  ringBuffer.avg5s;
        //ESP_LOGI(TAG, "db ringBuffer %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ", ringBuffer.buffer[0].value,ringBuffer.buffer[1].value,ringBuffer.buffer[2].value,ringBuffer.buffer[3].value,ringBuffer.buffer[4].value,ringBuffer.buffer[5].value,ringBuffer.buffer[6].value,ringBuffer.buffer[7].value,ringBuffer.buffer[8].value,ringBuffer.buffer[9].value,ringBuffer.buffer[10].value,ringBuffer.buffer[11].value,ringBuffer.buffer[12].value,ringBuffer.buffer[13].value,ringBuffer.buffer[14].value,ringBuffer.buffer[15].value,ringBuffer.buffer[16].value,ringBuffer.buffer[17].value,ringBuffer.buffer[18].value,ringBuffer.buffer[19].value,ringBuffer.buffer[20].value,ringBuffer.buffer[21].value,ringBuffer.buffer[22].value,ringBuffer.buffer[23].value,ringBuffer.buffer[24].value);
        //ESP_LOGI(TAG, "db count: %d average-25: %d ", ringBuffer.count,ringBuffer.avg5s);
        lv_msg_send(MSG_TEMP_UPDATE, &db_chart);
    }
}

static void timer_weather_callback(lv_timer_t * timer)
{
    if (cfg->WeatherAPIkey.empty()) {   // If API key not defined skip weather update
        ESP_LOGW(TAG,"Weather API Key not set");
        return;
    }

    // Update weather and trigger UI update
    owm->request_weather_update();
    lv_msg_send(MSG_WEATHER_CHANGED, owm);
}

// Callback to notify App UI change
static void tux_ui_change_cb(void * s, lv_msg_t *m)
{
    LV_UNUSED(s);
    unsigned int page_id = lv_msg_get_id(m);
    const char * msg_payload = (const char *)lv_msg_get_payload(m);
    const char * msg_data = (const char *)lv_msg_get_user_data(m);

    //ESP_LOGW(TAG,"[%" PRIu32 "] page event triggered",page_id);

    switch (page_id)
    {
        case MSG_PAGE_HOME:
            // Update date/time and current weather
            // Date/time is updated every second anyway
            lv_timer_resume(timer_temp);
            lv_msg_send(MSG_WEATHER_CHANGED, owm);
            break;
        case MSG_PAGE_REMOTE:
            // Trigger loading buttons data
            lv_timer_pause(timer_temp);
            break;
        case MSG_PAGE_SETTINGS:
            lv_timer_pause(timer_temp);
            if (!is_wifi_connected)  {// Provisioning mode
                lv_msg_send(MSG_WIFI_PROV_MODE, qr_payload);
                //lv_msg_send(MSG_QRCODE_CHANGED, qr_payload);
            } else {
                lv_msg_send(MSG_WIFI_CONNECTED, ip_payload);
            }
            break;
        case MSG_PAGE_OTA:
            lv_timer_pause(timer_temp);
            // Update firmware current version info
            lv_msg_send(MSG_DEVICE_INFO,device_info().c_str());
            break;
        case MSG_OTA_INITIATE:
            lv_timer_pause(timer_temp);
            // OTA update from button trigger
            xTaskCreate(run_ota_task, "run_ota_task", 1024 * 8, NULL, 5, NULL);
            break;
    }
}

static string device_info()
{
    std::string s_chip_info = "";

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);

    // CPU Speed - 80Mhz / 160 Mhz / 240Mhz
    rtc_cpu_freq_config_t conf;
    rtc_clk_cpu_freq_get_config(&conf);

    multi_heap_info_t info;    
	heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    float psramsize = (info.total_free_bytes + info.total_allocated_bytes) / (1024.0 * 1024.0);

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        s_chip_info += fmt::format("Firmware Ver : {}\n",running_app_info.version);
        s_chip_info += fmt::format("Project Name : {}\n",running_app_info.project_name);
        // running_app_info.time
        // running_app_info.date
    }
    s_chip_info += fmt::format("IDF Version  : {}\n\n",esp_get_idf_version());

    s_chip_info += fmt::format("Controller   : {} Rev.{}\n",CONFIG_IDF_TARGET,chip_info.revision);  
    //s_chip_info += fmt::format("\nModel         : {}",chip_info.model); // esp_chip_model_t type
    s_chip_info += fmt::format("CPU Cores    : {}\n", (chip_info.cores==2)? "Dual Core" : "Single Core");
    s_chip_info += fmt::format("CPU Speed    : {}Mhz\n",conf.freq_mhz);
    if(esp_flash_get_size(NULL, &flash_size) == ESP_OK) {
    s_chip_info += fmt::format("Flash Size   : {}MB {}\n",flash_size / (1024 * 1024),
                                            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "[embedded]" : "[external]");
    }
    s_chip_info += fmt::format("PSRAM Size   : {}MB {}\n",static_cast<int>(round(psramsize)),
                                            (chip_info.features & CHIP_FEATURE_EMB_PSRAM) ? "[embedded]" : "[external]");

    s_chip_info += fmt::format("Connectivity : {}{}{}\n",(chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "2.4GHz WIFI" : "NA",
                                                    (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
                                                    (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    //s_chip_info += fmt::format("\nIEEE 802.15.4 : {}",string((chip_info.features & CHIP_FEATURE_IEEE802154) ? "YES" : "NA"));

    //ESP_LOGE(TAG,"\n%s",device_info().c_str());
    return s_chip_info;
}