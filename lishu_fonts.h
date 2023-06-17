#ifndef LISHU_FONTS_H_
#define LISHU_FONTS_H_

#include <Arduino.h>

#define LISHU_FONT_NUMBER 29
#define LISHU_FONT_LENGTH 120 // 4*30
#define LISHU_FONT_WEIGHT 30
#define LISHU_FONT_HEIGHT 30

#define UTF8_HZ_LENGTH 3

#define FZYT_FONT_NUMBER 21
#define FZYT_FONT_LENGTH 120 // 4*30
#define FZYT_FONT_WEIGHT 30
#define FZYT_FONT_HEIGHT 30

typedef struct
{
  char fzyt_str[UTF8_HZ_LENGTH + 1]; // utf-8汉字
  unsigned char bits[FZYT_FONT_LENGTH];
} FzytFont3030; // 方正姚体30x30

extern const char fzyt_font_3030[FZYT_FONT_NUMBER][FZYT_FONT_LENGTH];
extern const char fzyt_font_ids[FZYT_FONT_NUMBER][UTF8_HZ_LENGTH + 1]; // utf-8汉字

typedef struct
{
  char solar_term[UTF8_HZ_LENGTH + 1]; // utf-8汉字
  unsigned char bits[LISHU_FONT_LENGTH];
} SolarFont3030;

extern const char lishu_font_3030[LISHU_FONT_NUMBER][LISHU_FONT_LENGTH];
extern const char lishu_font_ids[LISHU_FONT_NUMBER][UTF8_HZ_LENGTH + 1]; // utf-8汉字

#endif
