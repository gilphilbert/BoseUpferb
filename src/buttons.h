#ifndef CLASS_BUTTONS
#define CLASS_BUTTONS

#include <Arduino.h>

#define BTNS_SET      1
#define BTNS_MODE     2
#define BTNS_CLKSET   4
#define BTNS_LEFT     8
#define BTNS_RIGHT    16
#define BTNS_ONE      32
#define BTNS_TWO      64
#define BTNS_THREE    128
#define BTNS_FOUR     256
#define BTNS_FIVE     512
#define BTNS_SIX      1024
#define BTNS_DOWN     2048
#define BTNS_UP       4096
#define BTNS_AUX      8192
#define BTNS_AMFM     16384
#define BTNS_ONOFF    32768
#define BTNS_SNOOZE   65536

struct buttonState {
  bool set = 0;
  bool mode = 0;
  bool clkset = 0;
  bool left = 0;
  bool right = 0;
  bool one = 0;
  bool two = 0;
  bool three = 0;
  bool four = 0;
  bool five = 0;
  bool six = 0;
  bool down = 0;
  bool up = 0;
  bool aux = 0;
  bool amfm = 0;
  bool onoff = 0;
  bool snooze = 0;
};

class Buttons {
  private:
  public:
    void begin(uint8_t irqPin, void (*interrupt_callback)(buttonState buttons));
    //void begin(uint8_t irqPin);
    void loop();
};

#endif