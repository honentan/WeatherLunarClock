
#ifndef API_H_
#define API_H_

#pragma once
//#include <ArduinoJson.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#define XINZHI_URL_NOW_PREFIX "https://api.seniverse.com/v3/weather/now.json?language=zh-Hans&unit=c"
#define XINZHI_URL_3D_PREFIX "https://api.seniverse.com/v3/weather/daily.json?language=zh-Hans&unit=c"
#define XINZHI_MESSAGE_3D_LENGTH 2048
#define XINZHI_MESSAGE_NOW_LENGTH 640
#define LUNAR_URL_PREFIX1 "https://www.mxnzp.com/api/holiday/single/"
#define LUNAR_URL_PREFIX2 "?ignoreHoliday=false"
#define LUNAR_MESSAGE_MAX_LENGTH 768
#define MIN_DATA_LENGTH 60

typedef struct XinZhiWeatherData
{
  String icon_id;
  int air_temp_low;
  int air_temp_high;
} WeatherData;

typedef struct LunarData
{
  String lunar_date;
  bool if_leap_month;
  String date_str;
  String solar_terms;
  String workday;
} LunarCalendar;

class API
{
private:
  //std::unique_ptr<BearSSL::WiFiClientSecure> client;

  String toSolarTerm(String solar_term);
  bool ifLeapMonth(String *lunar_date);
  String toWorkday(String workday);
  
  int8_t split(String* desc_str, String source_str, const String split_str);
  String getJsonStringField(String payload, const String json_field_def);

public:
  API();
  ~API();
  bool getWeather(WeatherData* weather_data, bool is_now); // 获取天气数据，成功返回true
  bool getLunarCalendar(LunarCalendar *l_c);
};

#endif
