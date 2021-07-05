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

uint8_t Radio::formatTag() {
  if (nfc.tagPresent()) {
    return nfc.format();
  }
  return 0;
}

uint8_t Radio::writeNewTag() {
  if (nfc.tagPresent()) {
    NdefMessage msg;
    msg.addTextRecord(writeString);
    writeString = "";
    return nfc.write(msg);
  }
  return 0;
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
      if (writeString == "") {
        retVal = readCard();
      } else {
        // do writing of tag, probably need to format tag first
        uint8_t status = NFC_STATUS_FAILED;
        if (formatTag()) {
          if (writeNewTag()) {
            status = NFC_STATUS_SUCCESS;
          }
        }
      }
    }
  
    irqPrev = irqCurr;
  }
  return retVal;
}

void Radio::writeTag(String tagString) {
  writeString = tagString;
}

void Radio::cancelWrite() {
  writeString = "";
}