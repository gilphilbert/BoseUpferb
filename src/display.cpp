
#include "display.h"

#include <SPI.h>
#include "U8g2lib.h"

#define SCREEN_CS 27
#define SCREEN_DC 15
#define SCREEN_RST 2

//U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R2, SCREEN_CS, SCREEN_DC, SCREEN_RST);
U8G2_SSD1322_NHD_256X64_F_3W_HW_SPI u8g2(U8G2_R2, SCREEN_CS, SCREEN_RST);
#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 256

void Display::begin() {
  u8g2.begin();
  u8g2.setContrast(255);
}

void Display::updateScreen(const char * title, const char * artist) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_10x20_tf);
  u8g2.drawStr(0, 13, title);
  u8g2.setFont(u8g2_font_t0_18_tf);
  u8g2.drawStr(0, 25, artist);
  u8g2.drawStr(0, 50, "0:00");
  u8g2.sendBuffer();
}

void Display::welcome() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_10x20_tf);
  u8g2.drawStr(0, 13, "Welcome!");
  u8g2.setFont(u8g2_font_t0_18_tf);
  u8g2.drawStr(0, 25, "Scan a card to play a song");
  u8g2.sendBuffer();
}