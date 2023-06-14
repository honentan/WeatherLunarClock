#include <Arduino.h>
#include <ESP8266HTTPClient.h>
// #include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
//#include <coredecls.h>
// #include <sys/time.h>
#include <time.h>

#define TNHMETER


/* 使用0.96寸的OLED屏幕需要使用包含这个头文件 */
#include "SSD1306Wire.h"
/* OLED屏幕ui界面需要使用的头文件 */
#include "OLEDDisplayUi.h"

#ifdef TNHMETER // 温湿度模块
  #include "Adafruit_SHT4x.h"
#endif

#include "WeatherLunar.h"
#include "wl_frame.h"
#include "api.h"
#include "key_secret.h"

/* 设置oled屏幕的相关信息 */
const int I2C_ADDR = 0x3c; // oled屏幕的I2c地址
#define SDA_PIN 4          // SDA引脚，默认gpio4(D2)
#define SCL_PIN 5          // SCL引脚，默认gpio5(D1)

/* 新建一个oled屏幕对象，需要输入IIC地址，SDA和SCL引脚号 */
SSD1306Wire oled(I2C_ADDR, SDA_PIN, SCL_PIN);
/* 新建一个olde屏幕ui控制对象，参数为oled屏幕对象指针 */
OLEDDisplayUi ui(&oled);

#ifdef TNHMETER // 温湿度模块
  // 温湿度模块初始化
  Adafruit_SHT4x sht4 = Adafruit_SHT4x();
#endif

// 数据全局变量
time_t now;

int NTP_sync_hour = -1;
int weather_now_sync_min = -1;
int weather_3d_sync_hour = -1;
int lunar_sync_mday = -1;

#ifdef TNHMETER // 温湿度模块
  // 温湿度
  sensors_event_t humidity, temp;
#endif

/* 4个屏幕 */
WL_FRAME wl_frame;
// 画屏幕函数数组 - 0.农历日历
void frameLunarCalendar(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  wl_frame.drawLunarCalendar(display, x, y);
}
// 画屏幕函数数组 - 1.数字时钟
void frameClock(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  struct tm *p = nowTimeStruct();
  wl_frame.digitalClock(p, display, x, y);
}
// 画屏幕函数数组 - 2.实时天气
void frameWeatherNow(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  wl_frame.drawWeatherNow(display, x, y);
}
// 画屏幕函数数组 - 3.明后天天气预报
void frameWeather3d(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  wl_frame.drawWeather3d(display, x, y);
}
#ifdef TNHMETER // 温湿度模块
  // 画屏幕函数数组 - 4.温湿度计
  void frameTnHMeter(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
  {
    wl_frame.TnHMeter(int(temp.temperature), int(humidity.relative_humidity), display, x, y);
  }
#endif

/* 将要显示的每一帧图像函数定义在一个数组中 */
#ifdef TNHMETER // 温湿度模块
  FrameCallback frames[] = { frameLunarCalendar, frameClock, frameTnHMeter, frameWeatherNow, frameWeather3d, };
  /* 图像数量 */
  int frameCount = 5;
#else
  FrameCallback frames[] = { frameLunarCalendar, frameClock, frameWeatherNow, frameWeather3d, };
  /* 图像数量 */
  int frameCount = 4;
#endif

void setup()
{
  /* 1. 初始化串口通讯波特率为115200 */
  Serial.begin(115200);

  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  //delay(2000);
  Serial.printf("setup0 free heap: %d\n", ESP.getFreeHeap());

#ifdef TNHMETER // 温湿度模块
  sht4_setup();
#endif

  /* 2. oled屏幕初始化，oled屏幕ui显示控制设置 */
  oled_ui_setup();

  Serial.printf("setup1 free heap: %d\n", ESP.getFreeHeap());

  NTPSync(0); // NTP时间同步
  Serial.printf("setup2 free heap: %d\n", ESP.getFreeHeap());

  updateAll();

  Serial.printf("setup3 free heap: %d\n", ESP.getFreeHeap());
}

//int8_t loop_count = 0;
void loop()
{
  /* 刷新OLED屏幕，返回值为在当前设置的刷新率下显示完当前图像后所剩余的时间 */
  int8_t remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0)
  {
    // 我们可以在这剩余的时间做一些事情，但是如果时间不够，就不要做任何事情了
    updateAll();
  }

  /* LED状态取反 */
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

  //delay(500);
}

// 获取当前时间
struct tm *nowTimeStruct()
{
  now = time(nullptr);
  static struct tm *p;
  p = localtime(&now);

  //Serial.printf("\nnow: %d:%d:%d", p->tm_hour, p->tm_min, p->tm_sec);

  return p;
}

// 更新全部数据
// NTP同步：1小时(20秒后)
// 实时天气：20分钟(1/21/41分)
// 预报天气：4小时(1分)
// 农历：一天(30秒后)
void updateAll()
{
  struct tm *p = nowTimeStruct();

#ifdef TNHMETER // 温湿度模块
  sht4.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
#endif

  // NTP同步：1小时
  if ((p->tm_hour != NTP_sync_hour) && (p->tm_sec > 20))
  {
    Serial.printf("%d-%d %d:%d:%d Start NTPSync...\n", p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    wifiConnect();

    NTPSync(p->tm_hour);
    
    if (p->tm_hour >= 23 || p->tm_hour < 6) // 23点-6点关闭屏幕（降低亮度至1仍比较亮）
      oled.setContrast(0);       // 降低屏幕亮度，0为关闭
    else  // 6点-23点恢复亮度
      oled.setContrast(200);     // 恢复屏幕亮度，最高255
    Serial.printf("update1 free heap: %d\n", ESP.getFreeHeap());
  }

  // 实时天气：20分钟
  if ((p->tm_min % 20 == 1) && (p->tm_min != weather_now_sync_min) || weather_now_sync_min < 0)
  {
    Serial.printf("%d-%d %d:%d:%d Start WeatherNow...\n", p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    wifiConnect();

    WeatherData w_data_now;
    API api;
    if (api.getWeather(&w_data_now, true))
    { // now
      weather_now_sync_min = p->tm_min;
      wl_frame.updateWeatherNow(w_data_now);
    }
    Serial.printf("update2 free heap: %d\n", ESP.getFreeHeap());
  }

  // 预报天气：4小时
  if ((int(p->tm_hour / 4) == 0) && (p->tm_hour != weather_3d_sync_hour) || weather_3d_sync_hour < 0)
  {
    Serial.printf("%d-%d %d:%d:%d Start Weather3d...\n", p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    wifiConnect();

    WeatherData w_data_3d[2];
    API api;
    if (api.getWeather(w_data_3d, false)) // 3d
    {
      weather_3d_sync_hour = p->tm_hour;
      wl_frame.updateWeather3d(w_data_3d);
    }
    Serial.printf("update3 free heap: %d\n", ESP.getFreeHeap());
  }

  // 农历日历
  if (p->tm_mday != lunar_sync_mday || lunar_sync_mday < 0)
  {
    Serial.printf("%d-%d %d:%d:%d Start LunarCalendar...\n", p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    wifiConnect();

    char date_str[32];
    strftime(date_str, sizeof(date_str), "%Y%m%d", p);
    LunarCalendar l_c = {"", false, String(date_str), "", ""};
    API api;
    if(api.getLunarCalendar(&l_c))
    {
      wl_frame.updateLunar(l_c);
      lunar_sync_mday = p->tm_mday;
    }
    Serial.printf("update4 free heap: %d\n", ESP.getFreeHeap());
  }
}

// WIFI连接
void wifiConnect()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    //Serial.print("Already connected!\n");

    return;
  }

  Serial.printf("\nWIFI connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(80);
  }
  Serial.printf("done!\n");
  //delay(500);
}

// NTP时间同步
void NTPSync(int sync_hour)
{
  wifiConnect();

  configTime(8 * 60 * 60, 0, "ntp.ntsc.ac.cn", "ntp1.aliyun.com"); // ntp获取时间

  Serial.print("Waiting for NTP time sync: ");
  now = time(nullptr);
  while (now < 8 * 3600 * 2)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.print("done!\n");
  
  NTP_sync_hour = sync_hour;
}

/* oled屏幕ui显示控制设置 */
void oled_ui_setup(void)
{
  /* oled屏幕初始化 */
  oled.init();
  oled.flipScreenVertically(); // 设置屏幕翻转
  oled.setContrast(255);       // 设置屏幕亮度
  oled.clear();
  oled.display(); // 清除屏幕

  /* 1.设置帧率，当设置为60fps时，会几乎占满cpu，没有太多时间做其他事 */
  ui.setTargetFPS(10);

  /* 2. 设置指示栏活动和非活动符号外形 */
  // ui.setActiveSymbol(activeSymbol);
  // ui.setInactiveSymbol(inactiveSymbol);

  /* 3. 指示栏位置设置：TOP, LEFT, BOTTOM, RIGHT */
  // ui.setIndicatorPosition(BOTTOM);

  /* 4. 设置指示栏指示条移动的方向 */
  // ui.setIndicatorDirection(LEFT_RIGHT);

  /* 隐藏指示栏 */
  ui.disableAllIndicators();

  /* 5. 设置从上一帧过渡到下一帧的动画效果：SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN */
  ui.setFrameAnimation(SLIDE_LEFT);

  /* 6. 添加我们需要显示的图像，第一个参数为添加的图像数组，第二个参数为图像数量 */
  ui.setFrames(frames, frameCount);

  /* 7. 添加一个Overlays，Overlay是一个总是显示在相同位置的控件，第一个参数为添加的Overlays数组，第二个参数为Overlays数量 */
  // ui.setOverlays(overlays, overlaysCount);
}

#ifdef TNHMETER // 温湿度模块
  // 温湿度模块初始化
  void sht4_setup(void)
  {
    Serial.println("Adafruit SHT4x test ");
  
    sht4.begin();
  
    Serial.println("Found SHT4x sensor");
    Serial.print("Serial number 0x");
    Serial.println(sht4.readSerial(), HEX);
  
    // You can have 3 different precisions, higher precision takes longer
    sht4.setPrecision(SHT4X_HIGH_PRECISION);
    switch (sht4.getPrecision()) {
      case SHT4X_HIGH_PRECISION: 
        Serial.println("High precision");
        break;
      case SHT4X_MED_PRECISION: 
        Serial.println("Med precision");
        break;
      case SHT4X_LOW_PRECISION: 
        Serial.println("Low precision");
        break;
    }
  
    // You can have 6 different heater settings
    // higher heat and longer times uses more power
    // and reads will take longer too!
    sht4.setHeater(SHT4X_NO_HEATER);
    switch (sht4.getHeater()) {
      case SHT4X_NO_HEATER: 
        Serial.println("No heater");
        break;
      case SHT4X_HIGH_HEATER_1S: 
        Serial.println("High heat for 1 second");
        break;
      case SHT4X_HIGH_HEATER_100MS: 
        Serial.println("High heat for 0.1 second");
        break;
      case SHT4X_MED_HEATER_1S: 
        Serial.println("Medium heat for 1 second");
        break;
      case SHT4X_MED_HEATER_100MS: 
        Serial.println("Medium heat for 0.1 second");
        break;
      case SHT4X_LOW_HEATER_1S: 
        Serial.println("Low heat for 1 second");
        break;
      case SHT4X_LOW_HEATER_100MS: 
        Serial.println("Low heat for 0.1 second");
        break;
    }
  }
#endif
