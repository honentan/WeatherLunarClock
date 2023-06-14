
#include "api.h"
#include "key_secret.h"

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>

API::API()
{
  //client = std::unique_ptr<BearSSL::WiFiClientSecure>(new BearSSL::WiFiClientSecure);
  //client->setInsecure();
}

API::~API()
{
  //client = nullptr;
}


int8_t API::split(String* desc_str, String source_str, const String split_str)
{
  int pos;
  int i = 0;
  String temp_str;
 
  do
  {
    pos = source_str.indexOf(split_str);
    if (pos >= 0)
    {
      temp_str = source_str.substring(0, pos);
      source_str = source_str.substring(pos + split_str.length(), source_str.length());
      
        if (temp_str.length() > 0)
        {
        desc_str[i] = temp_str;
        i++;
        }
    }
    else
    {
      if (source_str.length() > 0)
      {
        desc_str[i] = source_str;
        i++;
      }
    }
    //Serial.printf("split [%d]: %d,%s\n", i, pos, source_str.c_str());
  }
  while (pos >= 0);

  return i;
}


// 简化版json字符串处理，+数字表示数组，-代表值为数字
// "/results/+0/now/temperature"
// "/results/+0/daily/+1/low"
// "/data/lunarCalendar"
// "/code/-"
String API::getJsonStringField(String payload, const String json_field_def)
{
  String json_fields[json_field_def.length()];
  String json_str, field_str;
  bool is_digit = false;
  
  int8_t field_number = split(json_fields, json_field_def, "/");
  int pos;
  
  /*
  for (int8_t i = 0; i < field_number; i++)
  {
    Serial.printf("getJsonStringField [%d]:%s\n", i, json_fields[i].c_str());
  }
  */
  
  for (int8_t i = 0; i < field_number; i++)
  {
    if (json_fields[i].length() == 0)
      continue;

    if (json_fields[i].startsWith("+")) // 数组
    {
      payload = payload.substring(payload.indexOf("[") + 1, payload.length());
      for (int j = 0; j < atoi(json_fields[i].c_str()); j++)
        payload = payload.substring(payload.indexOf("{") + 1, payload.length());
    }
    else if (json_fields[i].startsWith("-")) // 值为数字
    {
      is_digit = true;
      break;
    }
    else
    {
      field_str = "\"" + json_fields[i] + "\":";
      //Serial.printf("getJsonStringField payload[%d]: %s,%s\n", i, field_str.c_str(), payload.c_str());
      pos = payload.indexOf(field_str);
      //Serial.printf("getJsonStringField field_str[%d]: %d,%s\n", i, pos, field_str.c_str());
      if (pos >= 0)
      {
        payload = payload.substring(pos + field_str.length(), payload.length());
        if (i < field_number - 1) // 非最后一段
          payload.setCharAt(payload.lastIndexOf("}"), char(0));
      }
      else
        break;
    }
  }
  //Serial.printf("getJsonStringField substring payload:%d,%s\n", is_digit, payload.c_str());
  
  if (is_digit)
  {
    return payload.substring(0, payload.indexOf(","));
  }
  else
  {
    json_str = payload.substring(payload.indexOf("\"") + 1, payload.length());
    return json_str.substring(0, json_str.indexOf("\""));
  }
}

bool API::ifLeapMonth(String *lunar_date)
{
  if (lunar_date->indexOf("闰") == -1)
  {
    return false;
  }
  else
  {
    *lunar_date = lunar_date->substring(3, 15);

    return true;
  }
}

String API::toSolarTerm(String solar_term)
{
  if ((solar_term.indexOf("前") == -1) && (solar_term.indexOf("后") == -1))
    return solar_term;
  else
    return "";
}

String API::toWorkday(String workday)
{
  if (workday.indexOf("工作日") >= 0)
    return "班";
  else
    return "休";
}

// 获取天气
bool API::getWeather(WeatherData* weather_data, bool is_now)
{
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;

  bool is_success = true;

  String url_now_char;
  if (is_now)
    url_now_char = XINZHI_URL_NOW_PREFIX + String("&key=") + XINZHI_KEY + String("&location=") + XINZHI_LOCATION;
  else
    url_now_char = XINZHI_URL_3D_PREFIX + String("&key=") + XINZHI_KEY + String("&location=") + XINZHI_LOCATION;

  //Serial.printf("[HTTPS] getWeather %s begin... url:%s\n", (is_now ? "now" : "3d"), url_now_char.c_str());
  if (https.begin(*client, url_now_char))
  { // HTTPS
    // start connection and send HTTP header
    int httpCode = https.GET();
    // httpCode will be negative on error
    if (httpCode > 0)
    {
      // HTTP header has been send and Server response header has been handled
      //Serial.printf("[HTTPS] getWeatherNow GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        String payload = https.getString();
        //Serial.printf("[HTTPS] getLunarCalendar length:%d, payload:%s\n", payload.length(), payload.c_str());
        /*
        //StaticJsonDocument<XINZHI_MESSAGE_MAX_LENGTH> doc; // 小于1K时使用Static
        int buffer_len;
        if (is_now)
          buffer_len = XINZHI_MESSAGE_NOW_LENGTH; // 640
        else
          buffer_len = XINZHI_MESSAGE_3D_LENGTH; // 2048
        DynamicJsonDocument doc(buffer_len); // 640或2048
        DeserializationError error = deserializeJson(doc, payload);
        */
        Serial.printf("getWeather free heap: %d\n", ESP.getFreeHeap());
        /*
        if (error)
        {
          Serial.printf("\n!!!! deserializeJson() failed: %s\nfree heap: %d\n", error.f_str(), ESP.getFreeHeap());
          is_success = false;
        }
        */
        if (payload.length() < MIN_DATA_LENGTH)
        {
          Serial.printf("!!!! API::getWeather() failed: %d,%s\n", payload.length(), payload.c_str());
          is_success = false;
        }
        else
        {
          if (is_now == true)
          {
            //weather_data[0].air_temp_low = atoi(doc["results"][0]["now"]["temperature"]);
            //weather_data[0].air_temp_high = 0;
            //String icon_id = doc["results"][0]["now"]["code"];
            //weather_data[0].icon_id = icon_id.length() == 1 ? '0' + icon_id : icon_id;
            
            weather_data[0].air_temp_low = atoi(getJsonStringField(payload, "/results/+0/now/temperature").c_str());
            weather_data[0].air_temp_high = 0;
            String icon_id = getJsonStringField(payload, "/results/+0/now/code");
            weather_data[0].icon_id = icon_id.length() == 1 ? '0' + icon_id : icon_id;

            Serial.printf("[HTTPS] getWeather now air_temp_high:%d, air_temp_low:%d, icon_id:%s\n",
                          weather_data[0].air_temp_high, weather_data[0].air_temp_low, weather_data[0].icon_id);
          }
          else
          {
            /*
            weather_data[0].air_temp_low = atoi(doc["results"][0]["daily"][1]["low"]);
            weather_data[0].air_temp_high = atoi(doc["results"][0]["daily"][1]["high"]);
            String icon_id0 = doc["results"][0]["daily"][1]["code_day"];
            weather_data[0].icon_id = icon_id0.length() == 1 ? '0' + icon_id0 : icon_id0;

            weather_data[1].air_temp_low = atoi(doc["results"][0]["daily"][2]["low"]);
            weather_data[1].air_temp_high = atoi(doc["results"][0]["daily"][2]["high"]);
            String icon_id1 = doc["results"][0]["daily"][2]["code_day"];
            weather_data[1].icon_id = icon_id1.length() == 1 ? '0' + icon_id1 : icon_id1;
            */

            weather_data[0].air_temp_high = atoi(getJsonStringField(payload, "/results/+0/daily/+1/high").c_str());
            weather_data[0].air_temp_low = atoi(getJsonStringField(payload, "/results/+0/daily/+1/low").c_str());
            String icon_id0 = getJsonStringField(payload, "/results/+0/daily/+1/code_day");
            weather_data[0].icon_id = icon_id0.length() == 1 ? '0' + icon_id0 : icon_id0;

            weather_data[1].air_temp_high = atoi(getJsonStringField(payload, "/results/+0/daily/+2/high").c_str());
            weather_data[1].air_temp_low = atoi(getJsonStringField(payload, "/results/+0/daily/+2/low").c_str());
            String icon_id1 = getJsonStringField(payload, "/results/+0/daily/+2/code_day");
            weather_data[1].icon_id = icon_id1.length() == 1 ? '0' + icon_id1 : icon_id1;

            for (int i = 0; i < 2; i++)
              Serial.printf("[HTTPS] getWeather daily %d air_temp_high:%d, air_temp_low:%d, icon_id:%s\n",
                            i, weather_data[i].air_temp_high, weather_data[i].air_temp_low, weather_data[i].icon_id);
          }

        }

        //doc.clear();
      }
    }
    else
    {
      Serial.printf("[HTTPS] GET... failed, error: %d,%s\n",
                    httpCode, https.errorToString(httpCode).c_str());
      is_success = false;
    }

    https.end();
  }
  else
  {
    Serial.printf("[HTTPS] Unable to connect\n");

    is_success = false;
  }

  client = nullptr;

  return is_success;
}

// 获取农历及节气
bool API::getLunarCalendar(LunarCalendar *l_c)
{
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  
  bool is_success = true;

  String url_now_char = LUNAR_URL_PREFIX1 + l_c->date_str + LUNAR_URL_PREFIX2 + String("&app_id=") + LUNAR_APPID + String("&app_secret=") + LUNAR_APPSECRET;

  //Serial.printf("[HTTPS] getLunarCalendar begin... url:%s\n", url_now_char.c_str());
  if (https.begin(*client, url_now_char))
  { // HTTPS
    // start connection and send HTTP header
    int httpCode = https.GET();
    // httpCode will be negative on error
    if (httpCode > 0)
    {
      // HTTP header has been send and Server response header has been handled
      //Serial.printf("[HTTPS] getLunarCalendar GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        String payload = https.getString();
        //Serial.printf("[HTTPS] getLunarCalendar length:%d, payload:%s\n", payload.length(), payload.c_str());
        //StaticJsonDocument<LUNAR_MESSAGE_MAX_LENGTH> doc; // 小于1K时使用Static
        //DynamicJsonDocument doc(LUNAR_MESSAGE_MAX_LENGTH); // 大于1K时使用Dynamic
        //DeserializationError error = deserializeJson(doc, payload);

        /*
        if (error)
        {
          Serial.printf("\n!!!! deserializeJson() failed: %s\nfree heap: %d\n", error.f_str(), ESP.getFreeHeap());
          is_success = false;
        }
        */
        if (payload.length() < MIN_DATA_LENGTH)
        {
          Serial.printf("!!!! API::getLunarCalendar() failed: %d,%s\n", payload.length(), payload.c_str());
          is_success = false;
        }
        else
        {
          /*
          String lunar_date = doc["data"]["lunarCalendar"];
          l_c->lunar_date = lunar_date;
          String solar_terms = doc["data"]["solarTerms"];
          l_c->solar_terms = toSolarTerm(solar_terms);
          l_c->if_leap_month = ifLeapMonth(&(l_c->lunar_date));
          String workday = doc["data"]["typeDes"];
          l_c->workday = toWorkday(workday);
          Serial.printf(
              "[HTTPS] getLunarCalendar date_str:%s, lunar_date:%s, solar_terms:%s, leap_month:%d, workday:%s\n",
              l_c->date_str, l_c->lunar_date.c_str(), l_c->solar_terms.c_str(), l_c->if_leap_month, l_c->workday);
          */
          int ret_code = getJsonStringField(payload, "/code/-").toInt();
          
          if (ret_code == 1) // 成功
          {
            l_c->lunar_date = getJsonStringField(payload, "/data/lunarCalendar");
            String solar_terms = getJsonStringField(payload, "/data/solarTerms");
            l_c->solar_terms = toSolarTerm(solar_terms);
            String workday = getJsonStringField(payload, "/data/typeDes");
            l_c->workday = toWorkday(workday);
            Serial.printf("[HTTPS] getLunarCalendar date_str:%s, lunar_date:%s, solar_terms:%s, leap_month:%d, workday:%s\n",
                          l_c->date_str, l_c->lunar_date.c_str(), l_c->solar_terms.c_str(), l_c->if_leap_month, l_c->workday);
            
            Serial.printf("getLunarCalendar free heap: %d\n", ESP.getFreeHeap());
          }
          else
          {
            Serial.printf("API::getLunarCalendar() return_code: %d payload:%d,%s\n", ret_code, payload.length(), payload.c_str());
            //is_success = false;
            
            l_c->lunar_date = "正月初一";
            //l_c->solar_terms = "芒种";
            l_c->solar_terms = "";
            l_c->workday = "休";
            l_c->if_leap_month = false;
          }
        }

        //doc.clear();
      }
    }
    else
    {
      Serial.printf("[HTTPS] GET... failed, error: %s\n",
                    https.errorToString(httpCode).c_str());
      is_success = false;
    }

    https.end();
  }
  else
  {
    Serial.printf("[HTTPS] Unable to connect\n");
    is_success = false;
  }

  client = nullptr;

  /*
  l_c->lunar_date = "四月廿";
  l_c->solar_terms = "芒种";
  l_c->workday = "休";
  l_c->if_leap_month = true;
  */

  return is_success;
}
