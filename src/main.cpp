#include <Arduino.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourceID3.h"

#include "SPIFFS.h"
#include "SPI.h"
#include "SD.h"
#include "WiFiMulti.h"
#include "WiFiClientSecure.h"

#include "pindefs.h"
#include "display.h"
#include "rftag.h"

AudioGeneratorMP3 *mp3;
AudioOutputI2S *out;
AudioFileSourceSD *source = NULL;
AudioFileSourceID3 *id3;

Display screen;
Radio radio;

File dir;

String trackName;
String albumName;
String artistName;

QueueHandle_t controlQueue;
QueueHandle_t statusQueue;
int queueSize = 10;

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
    Serial.print(trackName);
    Serial.print(" by ");
    Serial.println(artistName);
    screen.updateScreen(trackName.c_str(), artistName.c_str());
  }
}

#define PLAYER_PLAY     1
#define PLAYER_STOP     2
#define PLAYER_PAUSE    3
typedef struct {
  uint8_t command = 0;
	const char * filename;
} playerMessage;

void playerTask(void * parameter) {
  Serial.println("player started");

  while (true) {
    if ((mp3) && (mp3->isRunning())) {
      if (!mp3->loop()) {
        mp3->stop();
        const int status = PLAYER_STOP;
        xQueueSend(statusQueue, &status, 0);
      }
    }

    playerMessage incoming;
    xQueueReceive(controlQueue, &incoming, 0);
    if (incoming.command == PLAYER_STOP) {
      mp3->stop();
      const int status = PLAYER_STOP;
      xQueueSend(statusQueue, &status, 0);
    } else if (incoming.command == PLAYER_PLAY) {
        //Serial.print("Time to play: ");
        //Serial.println(incoming.filename);
        if (String(incoming.filename).endsWith(".mp3")) {
          //Serial.println("File matches");
          if (mp3->isRunning()) {
            mp3->stop();
          }
          source->close();
          if (source->open(incoming.filename)) {
            Serial.println("File opened");
            //Serial.print("Playing from SD card...");
            Serial.println(incoming.filename);
            id3 = new AudioFileSourceID3(source);
            id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
            mp3->begin(id3, out);
            const int status = PLAYER_PLAY;
            xQueueSend(statusQueue, &status, 0);
          } else {
            Serial.print("Error opening");
            Serial.println(incoming.filename);
          }
        }
    }
    vTaskDelay(1);
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  source = new AudioFileSourceSD();

  out = new AudioOutputI2S();
  mp3 = new AudioGeneratorMP3();

  out -> SetGain(1.0/64.0);
  out -> SetPinout(I2S_BCLK,I2S_WCLK,I2S_DATA);
  SD.begin(SD_CS);
  dir = SD.open("/");

  screen.begin();
  screen.welcome();

  radio.begin();

  controlQueue = xQueueCreate( queueSize, sizeof( String ) );
  statusQueue = xQueueCreate( queueSize, sizeof( int ) );

  xTaskCreatePinnedToCore(playerTask, "Player", 10000, NULL, 1, NULL, 0);
}

bool playState = false;
void loop() {
  const String file = radio.loop();
  if (!file.equals("")) {
    if (SD.exists(file)) {
      playerMessage pf;
      pf.filename = file.c_str();
      pf.command = PLAYER_PLAY;
      xQueueSend(controlQueue, &pf, 0);
      
      playState = true;
    } else {
      Serial.println("File doesn't exist");
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
    Serial.println("Playing started");
    playState = true;
  } else if (status == 2) {
    Serial.println("Playing stopped");
    screen.welcome();
    playState = false;
  }
}