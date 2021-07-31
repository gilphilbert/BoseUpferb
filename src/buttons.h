#ifndef CLASS_BUTTONS
#define CLASS_BUTTONS

#include <Arduino.h>

struct buttonState {
  bool set = false;
  bool mode = false;
  bool clkset = false;
  bool left = false;
  bool right = false;
  bool one = false;
  bool two = false;
  bool three = false;
  bool four = false;
  bool five = false;
  bool six = false;
  bool down = false;
  bool up = false;
  bool aux = false;
  bool amfm = false;
  bool onoff = false;
  bool snooze = false;
};

void buttonSetup(uint8_t irqPin, void (*interrupt_callback)(buttonState buttons));
void buttonLoop();

void navExit();

bool inMenu();

#endif