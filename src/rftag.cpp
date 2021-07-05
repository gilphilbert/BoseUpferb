#include <Arduino.h>
#include <Wire.h>

#include "pindefs.h"
#include "rftag.h"

#include "PN532_I2C.h"
#include "PN532.h"
#include "NfcAdapter.h"

#define CARD_DELAY    1000  // wait 1s before reading another card

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c, NFC_IRQ);

long lastRead = 0;
bool enabled = true;
int irqCurr;
int irqPrev;

void Radio::startListening() {
  irqPrev = irqCurr = HIGH;
  nfc.startPassive();
  Serial.println("Scan a NFC tag");
}

String Radio::readCard() {
  String retStr = "";
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    NdefMessage message = tag.getNdefMessage();
    NdefRecord record = message.getRecord(0);
    int payloadLength = record.getPayloadLength();
    byte payload[payloadLength];
    record.getPayload(payload);
    String payloadAsString = "";
    for (int c = 3; c < payloadLength; c++) {
      payloadAsString += (char)payload[c];
    }
    retStr = payloadAsString;
    
    lastRead = millis();
  }
  enabled = false;
  return retStr;
}

void Radio::begin() {
  nfc.begin();
  startListening();
}

String Radio::loop() {
  String retVal = "";
  if (!enabled) {
    if (millis() - lastRead > CARD_DELAY) {
      enabled = true;
      startListening();
    }
  } else {
    irqCurr = digitalRead(NFC_IRQ);

    if (irqCurr == LOW && irqPrev == HIGH) {
      retVal = readCard();
    }
  
    irqPrev = irqCurr;
  }
  return retVal;
}