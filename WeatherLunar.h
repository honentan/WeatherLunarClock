#ifndef WEATHER_LUNAR_H_
#define WEATHER_LUNAR_H_

#include <Arduino.h>
#include <time.h>

/* OLED屏幕ui界面需要使用的头文件 */
#include "OLEDDisplayUi.h"


// 画屏幕函数数组 - 0.农历日历
void frameLunarCalendar(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
// 画屏幕函数数组 - 1.数字时钟
void frameClock(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
// 画屏幕函数数组 - 2.实时天气
void frameWeatherNow(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
// 画屏幕函数数组 - 3.明后天天气预报
void frameWeather3d(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);

// 获取当前时间
struct tm *nowTimeStruct();

// 更新全部数据
// NTP同步：1小时(20秒后)
// 实时天气：20分钟(1/21/41分)
// 预报天气：4小时(1分)
// 农历：一天(30秒后)
void updateAll();

// WIFI连接
void wifiConnect();

// NTP时间同步
void NTPSync(int sync_hour);

/* oled屏幕ui显示控制设置 */
void oled_ui_setup(void);

#endif
