#include <Arduino.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourceID3.h"

#include "SPIFFS.h"
#include "SPI.h"
#include "SD.h"

#include "WiFiClient.h"
#include "WiFiClientSecure.h"

#include "pindefs.h"
#include "rftag.h"
#include "display.h"
#include "buttons.h"
//#include "update.h"

AudioGeneratorMP3 *mp3;
AudioOutputI2S *audioOut;
AudioFileSourceSD *source = NULL;
AudioFileSourceID3 *id3;

//Update update;

//#define MAIN_LOGGING

File dir;

String trackName;
String albumName;
String artistName;

QueueHandle_t controlQueue;
QueueHandle_t statusQueue;
int queueSize = 10;

unsigned long ampTimer = 0;

// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  (void)cbData;

  if (strcmp(type, "Title") == 0) {
    trackName = String(string);
  } else if (strcmp(type, "Album") == 0) {
    albumName = String(string);
  } else if (strcmp(type, "Performer") == 0) {
    artistName = String(string);
  } else if (strcmp(type, "eof") == 0) {
    #ifdef MAIN_LOGGING
    Serial.print(trackName);
    Serial.print(" by ");
    Serial.println(artistName);
    #endif
    displayTrack(trackName, artistName);
  } else {
    #ifdef MAIN_LOGGING
    Serial.print(type);Serial.print("::");Serial.println(string);
    #endif
  }
}

#define PLAYER_PLAY     1
#define PLAYER_STOP     2
#define PLAYER_PAUSE    3
typedef struct {
  uint8_t command = 0;
	const char * filename;
} playerMessage;

void muteDAC() {
  digitalWrite(DAC_MUTE, 0);
}
void unmuteDAC() {
  digitalWrite(DAC_MUTE, 1);
}
void shutdownAmp() {
  digitalWrite(AMP_SHUTDOWN, 0);
}
void startupAmp() {
  digitalWrite(AMP_SHUTDOWN, 1);
}


void playerTask(void * parameter) {
  #ifdef MAIN_LOGGING
  Serial.println("player started");
  #endif

  const uint8_t dacDelay = 100;
  unsigned long dacTimer = 0;

  while (true) {
    if ((mp3) && (mp3->isRunning())) {
      if (!mp3->loop()) {
        #ifdef MAIN_LOGGING
        Serial.println("Playback ended");
        #endif
        mp3->stop();
        const int status = PLAYER_STOP;
        xQueueSend(statusQueue, &status, 0);
        muteDAC();
      }
    }

    playerMessage incoming;
    xQueueReceive(controlQueue, &incoming, 0);
    if (incoming.command == PLAYER_STOP) {
      #ifdef MAIN_LOGGING
      Serial.println("PLAYER::Asked to stop");
      #endif
      if ((mp3) && (mp3->isRunning())) {
        muteDAC();
        mp3->stop(); 
      }
      const int status = PLAYER_STOP;
      xQueueSend(statusQueue, &status, 0);
    } else if (incoming.command == PLAYER_PLAY) {
        if (String(incoming.filename).endsWith(".mp3")) {
          if (mp3->isRunning()) {
            muteDAC();
            mp3->stop();
          }
          source->close();
          if (source->open(incoming.filename)) {
            startupAmp();
            #ifdef MAIN_LOGGING
            Serial.print("File opened::");Serial.println(incoming.filename);
            #endif
            id3 = new AudioFileSourceID3(source);
            id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
            mp3->begin(id3, audioOut);
            dacTimer = millis() + dacDelay;
            const int status = PLAYER_PLAY;
            xQueueSend(statusQueue, &status, 0);
          } else {
            #ifdef MAIN_LOGGING
            Serial.print("Error opening::");
            Serial.println(incoming.filename);
            #endif
          }
        }
    }
    if (dacTimer && millis() > dacTimer) {
      dacTimer = 0;
      unmuteDAC();
    }
    vTaskDelay(1);
  }
}

uint8_t currentVolumeDivisor = 64;
void changeVolume(uint8_t divisor) {
  #ifdef MAIN_LOGGING
  Serial.print("VOL::Requested ");Serial.println(divisor);
  #endif
  if (divisor > 64 || divisor < 1) {
    return;
  }
  #ifdef MAIN_LOGGING
  Serial.print("VOL::Set ");Serial.println(divisor);
  #endif
  currentVolumeDivisor = divisor;
  audioOut -> SetGain(1.0/float(divisor));
}

void powerDown();
//void woken();

void powerDown() {
  // put display to sleep
  displaySleep();
  // put amp to sleep
  shutdownAmp();
  // mute DAC
  muteDAC();
  // set up interrupt for buttons
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 1);
  // put esp into deep sleep
  //esp_deep_sleep_start();

  playerMessage pf;
  pf.filename = "";
  pf.command = PLAYER_STOP;
  xQueueSend(controlQueue, &pf, 0);

  delay(10);

  esp_light_sleep_start();

  while (!powerButtonWake()) {
    esp_light_sleep_start();
  }
  displayWake();
  //woken();
}

/*
void woken() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: {
      bool isPowerOn = powerButtonWake();
      if (isPowerOn) {
        Serial.println("BOOT::Power button pressed");
      } else {
        powerDown();
      }
      break;
    }
    default:
      Serial.println("BOOT::Power button not pressed");
  }
}
*/

void buttonEvent (buttonState buttons) {
  if (buttons.aux) {
    if (inMenu()) {
      navExit();
    } else {
      navEnter();
    }
  } else
  if (buttons.amfm) {
    if (inMenu()) {
      navEnter();
    }
  } else
  if (buttons.down) {
    if (inMenu()) {
      navDown();
    } else {
      // volume down
      changeVolume(currentVolumeDivisor + 1);
    }
  } else
  if (buttons.up) {
    if (inMenu()) {
      navUp();
    } else {
      // volume up
      changeVolume(currentVolumeDivisor - 1);
    }
  } else
  if (buttons.snooze) {
    #ifdef MAIN_LOGGING
    Serial.println("BTN::Snooze, trying to stop playback");
    #endif
      playerMessage pf;
      pf.filename = "";
      pf.command = PLAYER_STOP;
      xQueueSend(controlQueue, &pf, 0);
  } else
  if (buttons.onoff) {
    #ifdef MAIN_LOGGING
    Serial.println("BTN::Powering down");
    #endif
    powerDown();
  }
}

void setup() {
  #ifdef MAIN_LOGGING
  Serial.begin(115200);
  #endif
  delay(400);
  source = new AudioFileSourceSD();

  audioOut = new AudioOutputI2S();
  mp3 = new AudioGeneratorMP3();

  changeVolume(currentVolumeDivisor);
  audioOut -> SetPinout(I2S_BCLK,I2S_WCLK,I2S_DATA);
  SD.begin(SD_CS);

  buttonSetup(BUTTONS_IRQ, buttonEvent);
  //woken();
  // wait for i2c bus to settle back down
  delay(100);

  displaySetup();
  nfcSetup();

  //String ssid = "Glibertvue";
  //String key = "InThePNW1234";
  //update.begin(ssid, key);
  //update.update();

  pinMode(DAC_MUTE, OUTPUT);
  pinMode(AMP_SHUTDOWN, OUTPUT);
  muteDAC();
  startupAmp();
  ampTimer = millis();

  controlQueue = xQueueCreate( queueSize, sizeof( String ) );
  statusQueue = xQueueCreate( queueSize, sizeof( int ) );

  xTaskCreatePinnedToCore(playerTask, "Player", 10000, NULL, 1, NULL, 0);
}

bool playState = false;
void loop() {
  const String file = nfcLoop();
  if (!file.equals("")) {
    if (SD.exists(file)) {
      playerMessage pf;
      pf.filename = file.c_str();
      pf.command = PLAYER_PLAY;
      xQueueSend(controlQueue, &pf, 0);
      
      playState = true;
    } else {
      #ifdef MAIN_LOGGING
      Serial.println("File doesn't exist");
      #endif
    }
  }

  // STOP PLAYBACK
  /*
  int status = 1;
  xQueueSend(controlQueue, &status, 0);
  */
  int status = 0;
  xQueueReceive(statusQueue, &status, 0);
  if (status == 1) {
    #ifdef MAIN_LOGGING
    Serial.println("Playing started");
    #endif
    playState = true;
  } else if (status == 2) {
    #ifdef MAIN_LOGGING
    Serial.println("Playing stopped");
    #endif
    displayClear();
    playState = false;
  }

  buttonLoop();
  displayLoop();

  if (ampTimer > 0 && millis() > ampTimer + 1200000) { // shutdown amp after 2 minutes with no playback
    ampTimer = 0;
    shutdownAmp();
  }
}