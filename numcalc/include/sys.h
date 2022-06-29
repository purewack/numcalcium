#pragma once
#include <Arduino.h>

#include "../numcalcium-base/hw.h"

#define DEBUG

#ifdef DEBUG
#define LOG(X) Serial.print(X)
#define LOGL(X) Serial.println(X)
#else
#define LOG(X) 
#define LOGL(X) 
#endif

// #define LCD_DEF_FONT u8g2_font_t0_12_tf
// #define LCD_LOG_FONT u8g2_font_tom_thumb_4x6_mf

// extern U8G2LOG u8g2log;
// extern uint8_t u8log_buffer[]; 

typedef struct Program {
  char* title;
  char* txt_f1;
  char* txt_f2;
  char* txt_f3;
  void (*onBegin)(void);
  void (*onEnd)(void);
  void (*onProcess)(void);
  // void (*onOk)(void);
  // void (*onNav)(int);
  // int (*onPress)(int);
  // int (*onRelease)(int);
  // void (*onLoop)(unsigned long);
  // void (*onGfx)(void);

  volatile int inactive_inc;
  volatile int inactive_lim;
  volatile int no_input_lim;
} prog_t; 

typedef struct Stats {
  volatile prog_t* cprog;
  volatile prog_t progs[6];
  volatile int c_i;
  volatile int fmode;
  volatile int cprog_sel;
  volatile int inactive_time;
  volatile int no_input_time;
  volatile int gfx_refresh;
  volatile int gfx_log;
  volatile int usb_state;
  volatile int usb_conn;
} stats_t;

extern stats_t stats;
extern font_t sys_font;

int changeToProg(int i);
void resetInactiveTime();

void connectUSB();
void disconnectUSB();
void drawUSBStatus();
void drawTitle();
void drawFooter();
void clearProgGFX();
void updateProgGFX();
