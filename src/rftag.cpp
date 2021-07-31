#include <Arduino.h>
#include <Wire.h>

#include "pindefs.h"
#include "rftag.h"

#include "PN532_SPI.h"
#include "PN532.h"
#include "NfcAdapter.h"

#define CARD_DELAY    2000  // wait 1s before reading another card

PN532_SPI pn532spi(SPI, NFC_CS);
NfcAdapter nfc = NfcAdapter(pn532spi, NFC_IRQ);

long lastRead = 0;
bool enabled = true;
int irqCurr;
int irqPrev;

#define NFC_MODE_READ   0
#define NFC_MODE_WRITE  1

String writeString = "";
uint8_t mode = NFC_MODE_READ;

void startListening() {
  irqPrev = irqCurr = HIGH;
  nfc.startPassive();
  Serial.println("Scan a NFC tag");
}

String readCard() {
  String retStr = "";
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    if(!tag.hasNdefMessage()) {
      Serial.println("Error reading tag");
      return "";
    }
    NdefMessage message = tag.getNdefMessage();
    if (message.getRecordCount() > 0) {
      NdefRecord record = message.getRecord(0);
      int payloadLength = record.getPayloadLength();
      byte payload[payloadLength];
      record.getPayload(payload);
      String payloadAsString = "";
      for (int c = 3; c < payloadLength; c++) {
        payloadAsString += (char)payload[c];
      }
      retStr = payloadAsString;
    }
    
    lastRead = millis();
  }
  enabled = false;
  return retStr;
}

bool writeNewTag() {
  if (nfc.tagPresent()) {
    String filename = writeString;
    mode = NFC_MODE_READ;

    Serial.println("NFC::Starting write");
    bool success = nfc.clean();
    if (!success) {
      // handle error condition
      Serial.println("NFC::Couldn't clean tag");
      return false;
    }
    Serial.println("NFC::Tag cleaned");
    success = nfc.format();
    if (!success) {
      // handle error condition
      Serial.println("NFC::Tag formatted");
      return false;
    }
    NdefMessage message = NdefMessage();
    message.addTextRecord(filename);
    success = nfc.write(message);
    if (!success) {
      // handle error condition
      Serial.println("NFC::Couldn't write tag");
    }
    Serial.println("NFC::Tag written");
    return success;
  }
  Serial.println("NFC::No tag present");
  return false;
}

void nfcSetup() {
  nfc.begin();
  startListening();
}

String nfcLoop() {
  String retVal = "";
  if (!enabled) {
    if (millis() - lastRead > CARD_DELAY) {
      enabled = true;
      startListening();
    }
  } else {
    irqCurr = digitalRead(NFC_IRQ);

    if (irqCurr == LOW && irqPrev == HIGH) {
      if (mode == NFC_MODE_WRITE) {
        Serial.println("Writing Tag");
        bool success = writeNewTag();
      } else {
        Serial.println("Reading Tag");
        retVal = readCard();
      }
    }
  
    irqPrev = irqCurr;
  }
  return retVal;
}

void writeTag(String tagString) {
  Serial.print("NFC::Write request = ");Serial.println(tagString);
  writeString = tagString;
  mode = NFC_MODE_WRITE;
}

void cancelWriteTag() {
  //writeString = "";
}