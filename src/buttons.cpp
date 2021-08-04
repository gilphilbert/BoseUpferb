#include <Arduino.h>
#include <Wire.h>

#include "buttons.h"

//#define BUTTON_LOG

const byte SLAVE_ADDR = 0x10;
const byte NUM_BYTES = 17;

static void (*int_cb)(buttonState buttons);

buttonState process(byte* data) {
  buttonState bs;

  if (data[0]) bs.set = true;
  if (data[1]) bs.mode = true;
  if (data[2]) bs.clkset = true;
  if (data[3]) bs.left = true;
  if (data[4]) bs.right = true;
  if (data[5]) bs.one = true;
  if (data[6]) bs.two = true;
  if (data[7]) bs.three = true;
  if (data[8]) bs.four = true;
  if (data[9]) bs.five = true;
  if (data[10]) bs.six = true;
  if (data[11]) bs.down = true;
  if (data[12]) bs.up = true;
  if (data[13]) bs.aux = true;
  if (data[14]) bs.amfm = true;
  if (data[15]) bs.onoff = true;
  if (data[16]) bs.snooze = true;

  return bs;
}

volatile bool trigger = false;

void IRAM_ATTR ISR() {
  trigger = true;
}

bool checkComms() {
  #ifdef BUTTON_LOG
  Wire.beginTransmission(SLAVE_ADDR);
  uint8_t test = Wire.endTransmission();
  switch(test) {
    case 0:
      Serial.println("BTN::Found controller");
      return true;
      break;
    case 1:
      Serial.println("BTN::Cannot find buttons (data too large)");
      break;
    case 2:
    case 3:
      Serial.println("BTN::Cannot find buttons (NACK)");
      break;
    default:
      Serial.println("BTN::Cannot find buttons (unknown)");
      break;
  }
  return false;
  #endif
}

void buttonSetup(uint8_t irqPin, void (*interrupt_callback_in)(buttonState buttons)) {
  #ifdef BUTTON_LOG
  Serial.print("BTN::Beginning, IRQ=");
  Serial.println(irqPin);
  #endif
  pinMode(irqPin, INPUT);
  attachInterrupt(irqPin, ISR, RISING);
  Wire.begin();
  //checkComms(); // <--- useful for dev work
  int_cb = interrupt_callback_in;
}

buttonState checkButtons() {
  byte data[NUM_BYTES] = { 0 };
  byte bytesReceived = 0;

  // Call the ATtiny once, to let it prepare the data. Ignore the result.
  Wire.requestFrom(SLAVE_ADDR, NUM_BYTES);
  // Give it a pause to prepare the data
  delayMicroseconds(10);
  // Call again, now it will have the data prepared
  Wire.requestFrom(SLAVE_ADDR, NUM_BYTES);
  bytesReceived = Wire.available();
  if (bytesReceived == NUM_BYTES) {
    for (byte i=0; i<NUM_BYTES; i++) {
      data[i] = Wire.read();
    }
    buttonState bs = process(data);
    return bs;
  }
}

bool powerButtonWake() {
  buttonState buttons = checkButtons();
  return buttons.onoff;
}

void buttonLoop() {
  if (trigger) {
    trigger = false;
    buttonState buttons = checkButtons();
    int_cb(buttons);
  }
}