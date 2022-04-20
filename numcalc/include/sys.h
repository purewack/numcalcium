#pragma once
#include <Arduino.h>
#include <MapleFreeRTOS900.h>
#include <U8g2lib.h>

#define SYS_PDOWN PB5

#define LCD_LIGHT PA6
#define LCD_CS PA4
#define LCD_DC PA3
#define LCD_CK PA5
#define LCD_RST PA2
#define LCD_MOSI PA7

#define SEG_A PB11
#define SEG_B PB10
#define SEG_C PB1
#define SEG_D PB0

#define ROW_A PA14
#define ROW_B PA15
#define ROW_C PB3
#define ROW_D PB4
#define ROW_E PB8
#define ROW_F PB9

#define B_OK PA8

#define GP_A PA0
#define GP_B PA1
#define COMMS_SDA PB7
#define COMMS_SDL PB6
#define COMMS_MOSI PB15
#define COMMS_MISO PB14
#define COMMS_CK   PB13
#define COMMS_CS PB12
#define COMMS_RX PA10
#define COMMS_TX PA9

#define K_Y 0
#define K_0 1
#define K_DOT 2
#define K_R 3
#define K_1 4
#define K_2 5
#define K_3 6
#define K_P 7
#define K_4 8
#define K_5 9
#define K_6 10
#define K_N 11
#define K_7 12
#define K_8 13
#define K_9 14
#define K_D 15
#define K_F1 16
#define K_F2 17
#define K_F3 18
#define K_X 19

extern U8G2_ST7565_ERC12864_ALT_F_4W_SW_SPI u8g2;

typedef struct t_io {
  uint8_t state;
  uint8_t old_state;
  uint8_t dt;
};

typedef struct HW
{
  t_io io[21];
  uint8_t enc_a, enc_b,enc_a_old,enc_b_old;
  int8_t enc_turns, enc_turns_old;
  uint8_t ok, oko;
  const uint8_t rows[6];
  const uint8_t cols[4];
} hw_t;

typedef struct Program {
  char* title;
  char* txt_f1;
  char* txt_f2;
  char* txt_f3;
  void (*on_begin)(void);
  void (*on_end)(void);
  void (*on_ok)(void);
  void (*on_nav)(int);
  void (*on_press)(int);
  void (*on_release)(int);
  void (*on_gfx)(void);

  int inactive_inc;
  int inactive_lim;
} prog_t; 

typedef struct Stats {
  prog_t* cprog;
  prog_t progs[6];
  int c_i;
  int fmode;
  int cprog_sel;
  int inactive_time;
  int gfx_text_count;
  char** gfx_text;
} stats_t;

extern stats_t stats;
extern hw_t hw;

void changeToProg(int i);
void resetInactiveTime();