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

//Radio nfcReader;
//Update update;

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
    displayTrack(trackName, artistName);
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
        Serial.println("Playback ended");
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
        if (String(incoming.filename).endsWith(".mp3")) {
          if (mp3->isRunning()) {
            mp3->stop();
          }
          source->close();
          if (source->open(incoming.filename)) {
            Serial.print("File opened::");
            Serial.println(incoming.filename);
            id3 = new AudioFileSourceID3(source);
            id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
            mp3->begin(id3, audioOut);
            const int status = PLAYER_PLAY;
            xQueueSend(statusQueue, &status, 0);
          } else {
            Serial.print("Error opening::");
            Serial.println(incoming.filename);
          }
        }
    }
    vTaskDelay(1);
  }
}

uint8_t currentVolumeDivisor = 64;
void changeVolume(uint8_t divisor) {
  Serial.print("VOL::Requested ");Serial.println(divisor);
  if (divisor > 64 || divisor < 1) {
    return;
  }
  Serial.print("VOL::Set ");Serial.println(divisor);
  currentVolumeDivisor = divisor;
  audioOut -> SetGain(1.0/float(divisor));
}

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
    int status = 1;
    xQueueSend(controlQueue, &status, 0);
  }
}

void printDirectory(File dir, int numTabs) {
   while (true) {

      File entry = dir.openNextFile();
      if (! entry) {
         // no more files
         break;
      }
      for (uint8_t i = 0; i < numTabs; i++) {
         Serial.print('\t');
      }
      Serial.print(entry.name());
      if (entry.isDirectory()) {
         Serial.println("/");
         printDirectory(entry, numTabs + 1);
      } else {
         // files have sizes, directories do not
         Serial.print("\t\t");
         Serial.println(entry.size(), DEC);
      }
      entry.close();
   }
}

void setup() {
  Serial.begin(115200);
  delay(400);
  source = new AudioFileSourceSD();

  audioOut = new AudioOutputI2S();
  mp3 = new AudioGeneratorMP3();

  //audioOut -> SetGain(1.0/float(currentVolumeDivisor));
  changeVolume(currentVolumeDivisor);
  audioOut -> SetPinout(I2S_BCLK,I2S_WCLK,I2S_DATA);
  SD.begin(SD_CS);

  buttonSetup(BUTTONS_IRQ, buttonEvent);
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
  digitalWrite(DAC_MUTE, HIGH);
  digitalWrite(AMP_SHUTDOWN, HIGH);

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
    displayClear();
    playState = false;
  }

  buttonLoop();
  displayLoop();
}