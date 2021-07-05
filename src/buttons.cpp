#include <Arduino.h>
#include <Wire.h>

#include "buttons.h"

const byte SLAVE_ADDR = 0x10;
const byte NUM_BYTES = 4;

static void (*int_cb)(buttonState buttons);

buttonState process(byte* data) {
  buttonState bs;
  uint32_t stateInt;
  stateInt = data[0];
  stateInt = (stateInt << 8) | data[1];
  stateInt = (stateInt << 8) | data[2];
  stateInt = (stateInt << 8) | data[3];

  //Serial.print("INTEGER::");
  //Serial.println(stateInt);

  if (stateInt >= BTNS_SNOOZE) {
    stateInt -= BTNS_SNOOZE;
    bs.snooze = true;
  }
  if (stateInt >= BTNS_ONOFF) {
    stateInt -= BTNS_ONOFF;
    bs.onoff = true;
  }
  if (stateInt >= BTNS_AMFM) {
    stateInt -= BTNS_AMFM;
    bs.amfm = true;
  }
  if (stateInt >= BTNS_AUX) {
    stateInt -= BTNS_AUX;
    bs.aux = true;
  }
  if (stateInt >= BTNS_UP) {
    stateInt -= BTNS_UP;
    bs.up = true;
  }
  if (stateInt >= BTNS_DOWN) {
    stateInt -= BTNS_DOWN;
    bs.down = true;
  }
  if (stateInt >= BTNS_SIX) {
    stateInt -= BTNS_SIX;
    bs.six = true;
  }
  if (stateInt >= BTNS_FIVE) {
    stateInt -= BTNS_FIVE;
    bs.five = true;
  }
  if (stateInt >= BTNS_FOUR) {
    stateInt -= BTNS_FOUR;
    bs.four = true;
  }
  if (stateInt >= BTNS_THREE) {
    stateInt -= BTNS_THREE;
    bs.three = true;
  }
  if (stateInt >= BTNS_TWO) {
    stateInt -= BTNS_TWO;
    bs.two = true;
  }
  if (stateInt >= BTNS_ONE) {
    stateInt -= BTNS_ONE;
    bs.one = true;
  }
  if (stateInt >= BTNS_RIGHT) {
    stateInt -= BTNS_RIGHT;
    bs.right = true;
  }
  if (stateInt >= BTNS_LEFT) {
    stateInt -= BTNS_LEFT;
    bs.left = true;
  }
  if (stateInt >= BTNS_CLKSET) {
    stateInt -= BTNS_CLKSET;
    bs.clkset = true;
  }
  if (stateInt >= BTNS_MODE) {
    stateInt -= BTNS_MODE;
    bs.mode = true;
  }
  if (stateInt >= BTNS_SET) {
    stateInt -= BTNS_SET;
    bs.set = true;
  }
  return bs;
}

bool trigger = false;

void IRAM_ATTR ISR() {
  trigger = true;
}

void Buttons::begin(uint8_t irqPin, void (*interrupt_callback_in)(buttonState buttons)) {
  //Serial.println("Beginning Buttons");
  //Serial.println(irqPin);
  pinMode(irqPin, INPUT);
  attachInterrupt(irqPin, ISR, HIGH);
  Wire.begin(21, 22);
  int_cb = interrupt_callback_in;
}

void Buttons::loop() {
  if (trigger) {
    //Serial.println("Got trigger");
    trigger = false;
    byte data[4] = { 0 };
    byte bytesReceived = 0;

    Wire.requestFrom(SLAVE_ADDR, NUM_BYTES);
    bytesReceived = Wire.available();
    //Serial.print("Length::");
    //Serial.println(bytesReceived);
    if (bytesReceived == NUM_BYTES) {
      //Serial.print("Data::");
      for (byte i=0; i<NUM_BYTES; i++) {
        data[i] = Wire.read();
        //Serial.println(data[i]);
      }
      buttonState bs = process(data);
      int_cb(bs);
      //Serial.print("Four::");
      //Serial.println(bs.four);
    }
  }
}