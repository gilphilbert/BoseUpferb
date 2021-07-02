#include <Arduino.h>
#include <Wire.h>

#include "pindefs.h"
#include "rftag.h"
#include "PN532.h"
#include "PN532_I2C.h"
#include "NfcAdapter.h"

PN532_I2C pn532i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532i2c);

//uint8_t connected = true;
//int readTimeout = 1000;
//int timer = 0;
uint8_t flag = false;

void IRAM_ATTR ISR() {
  flag = true;
}

void Radio::begin() {
  nfc.begin();
  pinMode(NFC_IRQ, INPUT);
  attachInterrupt(NFC_IRQ, ISR, FALLING);
}

String Radio::loop() {
  /*
  if (connected && millis() > timer) {
    readTimeout = 1000;
    timer = millis() + readTimeout;
    Serial.println("Looking for card...");
    if (nfc.tagPresent(100)) {
      readTimeout = 2000;
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
      return payloadAsString;
    }
  }
  return "";
  */
  if (flag) {
    flag = false;
    Serial.println("Reading card...");
    if (nfc.tagPresent(100)) {
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
      return payloadAsString;
    }
  }
  return "";
}


      /*
      NdefMessage message = NdefMessage();
      message.addTextRecord("/01. Threshold - Steve Miller Band.mp3");
      bool success = nfc.write(message);
      if (success) {
        Serial.println("Success. Try reading this tag with your phone.");        
      } else {
        Serial.println("Write failed.");
      }

      Serial.print("Cleaning card...");
      bool success = nfc.clean();
      if (success) {
          Serial.println("\nSuccess, tag restored to factory state.");
      } else {
          Serial.println("\nError, unable to clean tag.");
      }

      Serial.print("Formatting to NDEF...");
      success = nfc.format();
      if (success) {
        Serial.println("Done");
      } else {
        Serial.println("Failed");
      }

      sleep(2);
      */