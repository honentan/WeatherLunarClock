#ifndef NUMBER_FONTS_H_
#define NUMBER_FONTS_H_

typedef struct
{
  int weight;
  int height;
  unsigned char bits[10][5 * 60];
} NumberFont3660;

typedef struct
{
  int weight;
  int height;
  unsigned char bits[10][3 * 30];
} NumberFont1830;

extern const NumberFont1830 n_font1830;
extern const NumberFont3660 n_font3660;

#endif
