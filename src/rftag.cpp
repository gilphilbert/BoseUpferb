#include <Arduino.h>
#include <Wire.h>

#include "pindefs.h"
#include "rftag.h"

#include "PN532_SPI.h"
#include "PN532.h"
#include "NfcAdapter.h"

#include "display.h"

//#define NFC_LOG

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
  #ifdef NFC_LOG
  Serial.println("Scan a NFC tag");
  #endif
}

String readCard() {
  String retStr = "";
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    if(!tag.hasNdefMessage()) {
      #ifdef NFC_LOG
      Serial.println("Error reading tag");
      #endif
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
  }
  return retStr;
}

bool writeNewTag() {
  if (nfc.tagPresent()) {
    String filename = writeString;
    mode = NFC_MODE_READ;

    #ifdef NFC_LOG
    Serial.println("NFC::Starting write");
    #endif
    bool success = nfc.clean();
    if (!success) {
      // handle error condition
      #ifdef NFC_LOG
      Serial.println("NFC::Couldn't clean tag");
      #endif
      return false;
    }
    #ifdef NFC_LOG
    Serial.println("NFC::Tag cleaned");
    #endif
    success = nfc.format();
    if (!success) {
      // handle error condition
      #ifdef NFC_LOG
      Serial.println("NFC::Tag formatted");
      #endif
      return false;
    }
    NdefMessage message = NdefMessage();
    message.addTextRecord(filename);
    success = nfc.write(message);
    if (!success) {
      // handle error condition
      #ifdef NFC_LOG
      Serial.println("NFC::Couldn't write tag");
      #endif
    } else {
      #ifdef NFC_LOG
      Serial.println("NFC::Tag written");
      #endif
    }
    return success;
  }
  #ifdef NFC_LOG
  Serial.println("NFC::No tag present");
  #endif
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
        #ifdef NFC_LOG
        Serial.println("Writing Tag");
        #endif
        displayWritingTag();
        bool success = writeNewTag();
        displayWriteSuccess(!success);
        startListening();
      } else {
        #ifdef NFC_LOG
        Serial.println("Reading Tag");
        #endif
        retVal = readCard();
      }
      lastRead = millis();
      enabled = false;
    }
  
    irqPrev = irqCurr;
  }
  return retVal;
}

void writeTag(String tagString) {
  #ifdef NFC_LOG
  Serial.print("NFC::Write request = ");Serial.println(tagString);
  #endif
  writeString = tagString;
  mode = NFC_MODE_WRITE;
}

void cancelWriteTag() {
  //writeString = "";
}