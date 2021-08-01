#include <Arduino.h>

#include "pindefs.h"

#include <display.h>

#include <SD.h>

#include "rftag.h"

#include <SPI.h>
#include <menu.h>
#include <menuIO/u8g2Out.h>
#include <menuIO/chainStream.h>
#include <plugin/SDMenu.h>

using namespace Menu;

result filePick(eventMask event, navNode& nav, prompt &item);
void _displayWriteTag(String fn);

/* -------- information about the idle screen -------- */
String _artist = "";
String _trackName = "";
String _filename = "";

#define DISPLAY_MODE_IDLE       0
#define DISPLAY_MODE_TRACK      1
#define DISPLAY_MODE_WRITE_TAG  2
#define DISPLAY_MODE_WRITE_DONE 3
#define DISPLAY_MODE_WRITE_FAIL 4
uint8_t idleMode = DISPLAY_MODE_IDLE;

bool idleHasChanged = false;

/* -------- configure u8g2 -------- */
#define fontName u8g2_font_6x10_tf
#define fontX 7
#define fontY 11
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R2, DISPLAY_CS, DISPLAY_DC, DISPLAY_RESET);

/* -------- configure sd card file picker -------- */
CachedSDMenu<32> filePickMenu("Write Tag","/",filePick,enterEvent);
result filePick(eventMask event, navNode& navN, prompt &item) {
  if (navN.root->navFocus==(navTarget*)&filePickMenu) {
    _displayWriteTag(filePickMenu.selectedFile);
  }
  return proceed;
}

/* -------- configure menu system -------- */
#define MAX_DEPTH 3

MENU(mainMenu,"Menu",doNothing,noEvent,wrapStyle
  ,SUBMENU(filePickMenu)
  ,OP("Check for updates",doNothing,noEvent)
  ,EXIT("< Exit")
);

const colorDef<uint8_t> colors[6] MEMMODE={
  {{0,0},{0,1,1}},//bgColor
  {{1,1},{1,0,0}},//fgColor
  {{1,1},{1,0,0}},//valColor
  {{1,1},{1,0,0}},//unitColor
  {{0,1},{0,0,1}},//cursorColor
  {{1,1},{1,0,0}},//titleColor
};

chainStream<0> in(NULL);

MENU_OUTPUTS(out,MAX_DEPTH
  ,U8G2_OUT(u8g2,colors,fontX,fontY,0,0,{0,0,256/fontX,64/fontY})
  ,NONE
);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

uint8_t centerText(const char* str) {
  uint8_t x = u8g2.getStrWidth(str);
  x = 128 - round(float(x) / 2.0);
  return x;
}

/* currently this is a little wasteful - we should only update if there's something new to show */
void mainDisplay() {
  u8g2.clearBuffer();

  switch(idleMode) {
    case DISPLAY_MODE_IDLE: {
      #ifdef DISP_DEBUG
      Serial.println("DISP::Drawing idle screen");
      #endif
      u8g2.setFont(u8g2_font_open_iconic_play_2x_t);
      u8g2.drawGlyph(120, 29, 0x004D);

      String str = "Scan a card";
      u8g2.setFont(u8g2_font_crox2hb_tf);
      u8g2.drawStr(centerText(str.c_str()), 54, str.c_str());
      break;
    }
    case DISPLAY_MODE_TRACK: {
      #ifdef DISP_DEBUG
      Serial.println("DISP::Drawing playing song");
      #endif
      u8g2.setFont(u8g2_font_crox4hb_tf);
      u8g2.drawStr(centerText(_trackName.c_str()), 20, _trackName.c_str());

      u8g2.setFont(u8g2_font_crox2hb_tf);
      u8g2.drawStr(centerText(_artist.c_str()), 40, _artist.c_str());

      u8g2.drawFrame(80, 50, 96, 10);
      u8g2.drawBox(80, 50, 23, 10);
      break;
    }
    case DISPLAY_MODE_WRITE_TAG: {
      #ifdef DISP_DEBUG
      Serial.println("DISP::Drawing write tag");
      #endif
      u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t);
      u8g2.drawGlyph(120, 29, 0x0045);

      u8g2.setFont(u8g2_font_crox1h_tf);
      u8g2.drawStr(centerText(_filename.c_str()), 54, _filename.c_str());
      break;
    }
    case DISPLAY_MODE_WRITE_DONE: {
      #ifdef DISP_DEBUG
      Serial.println("DISP::Drawing successful write");
      #endif
      u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t);
      u8g2.drawGlyph(120, 29, 0x0045);

      String str = "Write Complete";
      u8g2.setFont(u8g2_font_crox1h_tf);
      u8g2.drawStr(centerText(str.c_str()), 54, str.c_str());
      idleMode = DISPLAY_MODE_IDLE;
      break;
    }
    case DISPLAY_MODE_WRITE_FAIL: {
      #ifdef DISP_DEBUG
      Serial.println("DISP::Drawing failed write");
      #endif
      u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t);
      u8g2.drawGlyph(120, 29, 0x0045);

      String str = "Write Failed";
      u8g2.setFont(u8g2_font_crox1h_tf);
      u8g2.drawStr(centerText(str.c_str()), 54, str.c_str());
      idleMode = DISPLAY_MODE_IDLE;
      break;
    }
  }
  u8g2.sendBuffer();

  nav.idleChanged = false;
}

void displayTrack(String trackName, String artist) {
  #ifdef DISP_DEBUG
  Serial.println("DISP::New track display request");
  #endif
  idleMode = DISPLAY_MODE_TRACK;
  _artist = artist;
  _trackName = trackName;
  nav.idleChanged = true;
}

void displayClear() {
  idleMode = DISPLAY_MODE_IDLE;
  nav.idleChanged = true;
}

void displayWriteSuccess(bool failed = false) {
  if (failed)
    idleMode = DISPLAY_MODE_WRITE_FAIL;
  else
    idleMode = DISPLAY_MODE_WRITE_DONE;
  nav.idleChanged = true;
}

void _displayWriteTag(String fn) {
  _filename = fn;
  idleMode = DISPLAY_MODE_WRITE_TAG;
  nav.idleChanged = true;
  nav.exit();
  // here, we call the request to write the NFC tag...
  writeTag(filePickMenu.selectedFile);
}

result idle(menuOut& o,idleEvent e) {
  switch(e) {
    case idleStart:
      //nav.idleChanged = true;
      break;
    case idling:
      mainDisplay();
      break;
    case idleEnd:
      break;
  }
  return proceed;
}

/* -------- setup module -------- */
void displaySetup() {
  u8g2.begin();
  u8g2.setFont(fontName);

  filePickMenu.begin();

  nav.idleTask = idle;
  nav.exit();
  // nav.idleChanged = true;
}

/* -------- main loop -------- */
void displayLoop() {
  nav.doInput();
  if (nav.changed(0)) {
    u8g2.clearBuffer();
    u8g2.setFont(fontName);
    nav.doOutput();
    u8g2.sendBuffer();
  }
}

// returns whether or not we're in the menu
bool inMenu() {
  if (nav.sleepTask) {
    return false;
  }
  return true;
}

/* -------- button handlers -------- */
void navUp() {
  nav.doNav(navCmd(downCmd));
}

void navDown() {
  nav.doNav(navCmd(upCmd));
}

void navEnter() {
  nav.doNav(navCmd(enterCmd));
}

void navExit() {
  nav.exit();
}

void navEscape() {
  nav.doNav(navCmd(escCmd));
}