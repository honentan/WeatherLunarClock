#ifndef WEATHER_FONTS_H_
#define WEATHER_FONTS_H_

#define WEATHER_FONT_NUMBER 9
#define WEATHER_ICON_L_LENGTH 288 // 6*48
#define WEATHER_ICON_S_LENGTH 120 // 4*30
#define WEATHER_ICON_DICT_LENGTH 48


typedef struct
{
  //int8_t weight;
  //int8_t height;
  char icon_id[3];
  int8_t index;
  unsigned char bits[WEATHER_ICON_L_LENGTH];
} WeatherIcon4848;

typedef struct
{
  //int8_t weight;
  //int8_t height;
  char icon_id[3];
  int8_t index;
  unsigned char bits[WEATHER_ICON_S_LENGTH];
} WeatherIcon3030;

extern const char weather_icon_4848[WEATHER_FONT_NUMBER][WEATHER_ICON_L_LENGTH];
extern const char weather_icon_3030[WEATHER_FONT_NUMBER][WEATHER_ICON_S_LENGTH];

extern const char weather_icon_ids[WEATHER_FONT_NUMBER][WEATHER_ICON_DICT_LENGTH];

#endif
