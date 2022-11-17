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

/*
Weather Config Json format:
{
    "weather_provider": "openweathermap.org",
    "settings" : {
        "location": "bangalore,india",
        "api_key" : "abcd",         // Free api - updates in 1min interval
        "units" : "metric"          // Kelvin / Celsius / Fahrenheit
        "update_interval" : 60      // in seconds
    }
}
*/

#ifndef TUX_WEATHERCONFIG_H_
#define TUX_WEATHERCONFIG_H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <inttypes.h>
#include "cJSON.h"
#include "esp_log.h"
#include <fstream>
using namespace std;

typedef enum
{
    WEATHER_UNITS_KELVIN,
    WEATHER_UNITS_CELSIUS,
    WEATHER_UNITS_FAHRENHEIT
} weather_units_t;


class WeatherConfig
{
    public:
        string WeatherProvider;
        string Location;        
        string APIkey;           
        uint UpdateInterval;   // in seconds
        weather_units_t TemperatureUnits;

        WeatherConfig(string filename);
        void load_config();
        void save_config();

    private:
        void read_json_file();
        void write_json_file();

        string file_name;
        string jsonString;
        cJSON *root;
        cJSON *settings;
    protected:
};


#endif