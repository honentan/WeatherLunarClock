
#include "OLEDDisplayUi.h"
#include "lishu_fonts.h"
#include "weather_fonts.h"
#include "number_fonts.h"
#include "wl_frame.h"
#include "api.h"

WL_FRAME::WL_FRAME() {
  lunar_calendar = {"正月初一", false, "20230101", "春节", "休"};
  weather_3d[0] = {"xx", 0, 0};
  weather_3d[1] = {"xx", 0, 0}; // 0明天，1后天
  weather_now = {"xx", 0, 0}; // 当天实时
  
  strcpy(w_icon48.icon_id, "");

  strcpy(w_icon30[0].icon_id, "");
  strcpy(w_icon30[1].icon_id, "");

  memset(solar_font[0].solar_term, 0, UTF8_HZ_LENGTH + 1);
  memset(solar_font[1].solar_term, 0, UTF8_HZ_LENGTH + 1);

  for (int8_t i = 0; i < 4; i++)
    memset(lunar_font[i].fzyt_str, 0, UTF8_HZ_LENGTH + 1);

  memset(workday_icon.fzyt_str, 0, UTF8_HZ_LENGTH + 1);
  memcpy(humidity_icon.fzyt_str, "湿", UTF8_HZ_LENGTH + 1);
  memcpy(humidity_icon.bits, fzyt_font_3030[fzytFontIndex(humidity_icon.fzyt_str)], FZYT_FONT_LENGTH);
}

// 更新节气字体数据
void WL_FRAME::updateSolarFontBits() {
  if (memcmp(solar_font[0].solar_term, lunar_calendar.solar_terms.c_str(), UTF8_HZ_LENGTH) != 0) {
    memcpy(solar_font[0].solar_term, lunar_calendar.solar_terms.c_str(), UTF8_HZ_LENGTH);
    memcpy(solar_font[0].bits, lishu_font_3030[solarFontIndex(solar_font[0].solar_term)], LISHU_FONT_LENGTH);
  }

  if (memcmp(solar_font[1].solar_term, lunar_calendar.solar_terms.c_str() + UTF8_HZ_LENGTH, UTF8_HZ_LENGTH) != 0) {
    memcpy(solar_font[1].solar_term, lunar_calendar.solar_terms.c_str() + UTF8_HZ_LENGTH, UTF8_HZ_LENGTH);
    memcpy(solar_font[1].bits, lishu_font_3030[solarFontIndex(solar_font[1].solar_term)], LISHU_FONT_LENGTH);
  }
}

// 更新农历日期和班/休字体数据
void WL_FRAME::updateLunarFontBits() {
  for (int8_t i = 0; i < 4; i++) {
    if (memcmp(lunar_font[i].fzyt_str, lunar_calendar.lunar_date.c_str() + i * UTF8_HZ_LENGTH, UTF8_HZ_LENGTH) != 0) {
      memcpy(lunar_font[i].fzyt_str, lunar_calendar.lunar_date.c_str() + i * UTF8_HZ_LENGTH, UTF8_HZ_LENGTH);
      memcpy(lunar_font[i].bits, fzyt_font_3030[fzytFontIndex(lunar_font[i].fzyt_str)], FZYT_FONT_LENGTH);
    }
  }

  if (memcmp(workday_icon.fzyt_str, lunar_calendar.workday.c_str(), UTF8_HZ_LENGTH) != 0) {
    memcpy(workday_icon.fzyt_str, lunar_calendar.workday.c_str(), UTF8_HZ_LENGTH);
    memcpy(workday_icon.bits, fzyt_font_3030[fzytFontIndex(workday_icon.fzyt_str)], FZYT_FONT_LENGTH);
  }
}

int8_t WL_FRAME::fzytFontIndex(const char* fzyt_str) {
  if (strlen(fzyt_str) != UTF8_HZ_LENGTH)
    return 0;

  int index = 0;

  for (int i = 0; i < FZYT_FONT_NUMBER; i++) {
    if (strcmp(fzyt_font_ids[i], fzyt_str) == 0) {
      index = i;
      break;
    }
  }

  return index;
}

int8_t WL_FRAME::solarFontIndex(const char* solar_term) {
  if (strlen(solar_term) != UTF8_HZ_LENGTH)
    return 0;

  int index = 0;

  for (int i = 0; i < LISHU_FONT_NUMBER; i++) {
    if (strcmp(lishu_font_ids[i], solar_term) == 0) {
      index = i;
      break;
    }
  }

  return index;
}

void WL_FRAME::updateWeatherIconBits(FontSize font_size, const char *icon_id[]) {
  if (font_size == LARGE) {
	  if (strcmp(icon_id[0], w_icon48.icon_id) == 0)
	    return;
  
    strcpy(w_icon48.icon_id, icon_id[0]);

	  int8_t index = weatherIconIndex(icon_id[0]);
	  if (index == w_icon48.index)
	    return;
  
    w_icon48.index = index;
    memcpy(w_icon48.bits, weather_icon_4848[index], WEATHER_ICON_L_LENGTH);
  } else {
    for (int8_t i = 0; i < 2; i++) {
      if (strcmp(icon_id[i], w_icon30[i].icon_id) != 0) {
        strcpy(w_icon30[i].icon_id, icon_id[i]);
        
        int8_t index = weatherIconIndex(icon_id[i]);
        if (index != w_icon30[i].index) {
          w_icon30[i].index = index;
          memcpy(w_icon30[i].bits, weather_icon_3030[index], WEATHER_ICON_S_LENGTH);
        }
      }
    }
  }
}

int8_t WL_FRAME::weatherIconIndex(const char* icon_id)
{
  if (strlen(icon_id) != 2)
    return 0;

  int index = 0;

  for (int i = 0; i < WEATHER_FONT_NUMBER; i++)
  {
    if (strstr(weather_icon_ids[i], icon_id) != NULL)
    {
      index = i;
      break;
    }
  }

  return index;
}

void WL_FRAME::updateWeather3d(WeatherData* weather_data)
{
  weather_3d[0] = weather_data[0];
  weather_3d[1] = weather_data[1];
  
  const char* icon_id[] = {weather_3d[0].icon_id.c_str(), weather_3d[1].icon_id.c_str()};
  updateWeatherIconBits(SMALL, icon_id);
}
  
void WL_FRAME::drawWeather3d(OLEDDisplay *display, int16_t x, int16_t y)
{
  if ((strcmp(w_icon30[0].icon_id, "xx") == 0) ||
    (strcmp(w_icon30[1].icon_id, "xx") == 0))
    return;

  display->setFont(ArialMT_Plain_24);

  display->drawXbm(x, y + 1, WEATHER_S_WEIGHT, WEATHER_S_HEIGHT, w_icon30[0].bits);
  show2Digits(weather_3d[0].air_temp_low, SMALL, display, x + 35, y + 1);
  display->drawString(x + 77, y + 1, "-");
  show2Digits(weather_3d[0].air_temp_high, SMALL, display, x + 90, y + 1);

  display->drawXbm(x, y + 33, WEATHER_S_WEIGHT, WEATHER_S_HEIGHT, w_icon30[1].bits);
  show2Digits(weather_3d[1].air_temp_low, SMALL, display, x + 35, y + 33);
  display->drawString(x + 77, y + 33, "-");
  show2Digits(weather_3d[1].air_temp_high, SMALL, display, x + 90, y + 33);
}

void WL_FRAME::updateLunar(LunarCalendar l_lunar) {
  lunar_calendar = l_lunar;
  updateLunarFontBits();
  if (lunar_calendar.solar_terms.length() > 0)
	  updateSolarFontBits();
}

void WL_FRAME::updateWeatherNow(WeatherData weather_data)
{
  weather_now = weather_data;
  const char* icon_id[] = {weather_now.icon_id.c_str(), ""};
  updateWeatherIconBits(LARGE, icon_id);
}
  
void WL_FRAME::drawWeatherNow(OLEDDisplay *display, int16_t x, int16_t y)
{
  //Serial.printf("WL_FRAME::drawWeatherNow %s,%d\n", weather_now.icon_id, weather_now.air_temp_low);

  if (strlen(w_icon48.icon_id) != 2)
    return;

  display->drawXbm(x, y + 7, WEATHER_L_WEIGHT, WEATHER_L_HEIGHT, w_icon48.bits);
  show2Digits(weather_now.air_temp_low, LARGE, display, x + 54, y + 2);
}


void WL_FRAME::showSolarTerm(OLEDDisplay *display, int16_t x, int16_t y) {
  display->drawXbm(x + 1, y + 32, LISHU_FONT_WEIGHT, LISHU_FONT_HEIGHT, solar_font[0].bits);
  display->drawXbm(x + 31, y + 32, LISHU_FONT_WEIGHT, LISHU_FONT_HEIGHT, solar_font[1].bits);
}


void WL_FRAME::drawLunarCalendar(OLEDDisplay *display, int16_t x, int16_t y)
{
  if (lunar_calendar.date_str.length() == 0)
    return;

  //Serial.printf("WL_FRAME::lunarCalendar date_str:%s, lunar_date:%s, solar_terms:%s, leap_month:%d, workday:%s\n",
  //              lunar_calendar.date_str, lunar_calendar.lunar_date.c_str(), lunar_calendar.solar_terms.c_str(), lunar_calendar.if_leap_month, lunar_calendar.workday);

  // 农历月日，闰月反白
  for (int8_t i = 0; i < 4; i++) {
    if (strlen(lunar_font[i].fzyt_str) == 0) // 四月廿
      continue;

    if (lunar_calendar.if_leap_month)
      display->drawXbm(x + i * FZYT_FONT_WEIGHT, y, FZYT_FONT_WEIGHT, FZYT_FONT_HEIGHT, reverseHzBits(lunar_font[i].bits));
    else
      display->drawXbm(x + i * FZYT_FONT_WEIGHT, y, FZYT_FONT_WEIGHT, FZYT_FONT_HEIGHT, lunar_font[i].bits);
  }
  //showHzStr(lunar_calendar.lunar_date, display, x, y, lunar_calendar.if_leap_month);

  if (lunar_calendar.solar_terms.length() > 0)
    showSolarTerm(display, x, y);
  else
    showMMDD(lunar_calendar.date_str, display, x, y);
	
  //showHzStr(lunar_calendar.workday, display, x + 91, y + 32, false);
  display->drawXbm(x + 94, y + 32, FZYT_FONT_WEIGHT, FZYT_FONT_HEIGHT, workday_icon.bits);
}

void WL_FRAME::digitalClock(struct tm *p,
                            OLEDDisplay *display, int16_t x, int16_t y)
{
  show2Digits(p->tm_hour, SMALL, display, x + 2, y);
  show2Digits(p->tm_min, SMALL, display, x + 2, y + 31);
  show2Digits(0 - p->tm_sec, LARGE, display, x + 54, y + 2);
}


void WL_FRAME::TnHMeter(int8_t temperature, int8_t humidity,
                         OLEDDisplay *display, int16_t x, int16_t y)
{
  show2Digits(temperature, LARGE, display, x, y + 2);

  //showHzStr("湿", display, x + 94, y, false);
  display->drawXbm(x + 94, y, FZYT_FONT_WEIGHT, FZYT_FONT_HEIGHT, humidity_icon.bits);
  show2Digits(humidity, SMALL, display, x + 89, y + 34);
}


// 日期MM/DD
void WL_FRAME::showMMDD(String date_str, // YYYYMMDD
                        OLEDDisplay *display, int16_t x, int16_t y)
{
  if (date_str.length() != String("YYYYMMDD").length())
    return;

  display->setFont(ArialMT_Plain_24);
  display->drawString(x + 40, y + 32, "/");
  
  int month = atoi(date_str.substring(4, 6).c_str());
  int day = atoi(date_str.substring(6, 8).c_str());

  //Serial.printf("WL_FRAME::showMMDD %s,%d,%d\n", date_str.c_str(), month, day);

  show2Digits(month, SMALL, display, x + 0, y + 32);
  show2Digits(day, SMALL, display, x + 51, y + 32);
}
				
const unsigned char *WL_FRAME::digitBits(int number, FontSize font_size, bool if_reverse)
{
  static const unsigned char *source_bits;

  if (font_size == LARGE)
    source_bits = n_font3660.bits[number];
  else
    source_bits = n_font1830.bits[number];

  if (if_reverse)
    return reverseDigitBits(source_bits, font_size);
  else
    return source_bits;
}

void WL_FRAME::show2Digits(int number, FontSize font_size,
                           OLEDDisplay *display, int16_t x, int16_t y)
{
  bool if_reverse = number < 0 ? true : false;
  int abs_number = number < 0 ? 0 - number : number;
  int first_digit = int(abs_number / 10);
  int second_digit = abs_number % 10;

  //if (number < 10)
  //  Serial.printf("WL_FRAME::show2Digits %d,%d,%d\n", number, first_digit, second_digit);

  if (font_size == LARGE)
  {
    display->drawXbm(x, y, NUMBER_L_WEIGHT, NUMBER_L_HEIGHT, digitBits(first_digit, font_size, if_reverse));
    display->drawXbm(x + NUMBER_L_WEIGHT + 1, y, NUMBER_L_WEIGHT, NUMBER_L_HEIGHT, digitBits(second_digit, font_size, if_reverse));
  }
  else
  {
    display->drawXbm(x, y, NUMBER_S_WEIGHT, NUMBER_S_HEIGHT, digitBits(first_digit, font_size, if_reverse));
    display->drawXbm(x + NUMBER_S_WEIGHT + 1, y, NUMBER_S_WEIGHT, NUMBER_S_HEIGHT, digitBits(second_digit, font_size, if_reverse));
  }
}


const unsigned char *WL_FRAME::reverseDigitBits(const unsigned char *source_bits, FontSize font_size)
{
  if (font_size == LARGE)
  {
    static unsigned char desc_bits[(int(NUMBER_L_WEIGHT / 8) + 1) * NUMBER_L_HEIGHT];
    reverseAllBits(desc_bits, source_bits, NUMBER_L_WEIGHT, NUMBER_L_HEIGHT);

    return desc_bits;
  }
  else
  {
    static unsigned char desc_bits[(int(NUMBER_S_WEIGHT / 8) + 1) * NUMBER_S_HEIGHT];
    reverseAllBits(desc_bits, source_bits, NUMBER_S_WEIGHT, NUMBER_S_HEIGHT);

    return desc_bits;
  }
}

void WL_FRAME::reverseAllBits(unsigned char *desc_bits, const unsigned char *source_bits, int weight, int height) {
  int weight_bytes = weight % 8 == 0 ? int(weight / 8) : int(weight / 8) + 1;

  for (int b = 0; b < weight_bytes * height; b++) {
    desc_bits[b] = ~source_bits[b];
    // Serial.printf("source:%02x, desc:%02x\n", source_bits[b], desc_bits[b]);
  }

  // 第一行
  for (int fl = 0; fl < weight_bytes; fl++)
    desc_bits[fl] = 0xff;

  // 第一列，高低位二进制字符串镜像
  for (int fc = 0; fc < weight_bytes * height; fc += weight_bytes)
    desc_bits[fc] |= 0x01;

  // 最后一行
  for (int l = weight_bytes * (height - 1); l < weight_bytes * height; l++)
    desc_bits[l] = 0xff;

  unsigned char last_bit[] = {0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40};
  // 最后一列，高低位二进制字符串镜像
  for (int lc = 0; lc < weight_bytes * height; lc += weight_bytes)
    desc_bits[lc + weight_bytes - 1] |= last_bit[weight % 8];

  return;
}

const unsigned char *WL_FRAME::reverseHzBits(const unsigned char *source_bits) {
  static unsigned char desc_bits[(int(HZ_WEIGHT / 8) + 1) * HZ_HEIGHT];
  reverseAllBits(desc_bits, source_bits, HZ_WEIGHT, HZ_HEIGHT);

  return desc_bits;
}
