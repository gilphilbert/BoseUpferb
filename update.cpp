//#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClient.h>

#include "update.h"
//#include "display.h"
//Display display;

WiFiClient client;

bool HTTP_OTA = false;

void Update::begin(String _ssid, String _key) {
  ssid = _ssid;
  key = _key;
}

void Update::loop() {
  if (wifiStatus == UPDATE_WIFI_STATE_CONNECTING) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Wifi connected");
      wifiStatus = UPDATE_WIFI_STATE_CONNECTED;

      unsigned short perform = false;
      if (action == UPDATE_ACTION_UPDATE) {
        perform = true;
      }

      bool newVer = _update(perform);
      if (newVer && action == UPDATE_ACTION_CHECK) {
        // show there's a new version on the screen
        //display.updating();
      }
    }
  }
}

void Update::_connect() {
  Serial.print("Connecting to WiFi network:");
  Serial.println(ssid);
  wifiStatus = UPDATE_WIFI_STATE_CONNECTING;
  WiFi.begin(ssid.c_str(), key.c_str());
}

void Update::check() {
  if (status == UPDATE_ACTION_NONE) {
    status = UPDATE_ACTION_CHECK;
  }

  if (wifiStatus != UPDATE_WIFI_STATE_CONNECTED) {
    _connect();
  } else {
    bool newVer = _update(false);
    if (newVer && action == UPDATE_ACTION_CHECK) {
      // show there's a new version on the screen
    }
  }
}

void Update::update() {
  Serial.println("Update requested");
  status = UPDATE_ACTION_UPDATE;

  if (wifiStatus != UPDATE_WIFI_STATE_CONNECTED) {
    _connect();
  } else {
    _update(true);
  }
}

bool Update::_update(bool perform) {
  Serial.println("Checking for firmware updates");

  String verURL = String(UrlBase);
  verURL.concat("/fwver.txt");

  Serial.print("Current firmware version: ");
  Serial.println(version);
  Serial.print("URL: ");
  Serial.println(verURL);

  HTTPClient httpClient;
  httpClient.begin(client, verURL);
  int httpCode = httpClient.GET();
  if (httpCode == 200) {
    String newFWVersion = httpClient.getString();

    Serial.print("Available firmware version: ");
    Serial.println(newFWVersion);

    int newVersion = newFWVersion.toInt();
    if (newVersion > version) {
      if (!perform) {
        httpClient.end();
        action = UPDATE_ACTION_NONE;
        return true;
      } else {
        Serial.println( "Preparing to update" );
        String fwImageURL = UrlBase;
        fwImageURL.concat("/bose-firmware-");
        fwImageURL.concat(String(newVersion));
        fwImageURL.concat(".bin");

        Serial.print("Update firmware file: ");
        Serial.println(fwImageURL);

        t_httpUpdate_return ret = httpUpdate.update(client, fwImageURL);
        switch(ret) {
          case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            char currentString[64];
            sprintf(currentString, "\nHTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            break;

          case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;

          case HTTP_UPDATE_OK:
            Serial.println("Update success");
            httpClient.end();
            action = UPDATE_ACTION_NONE;
            return true;
        }
      }
    }
    else {
      Serial.println( "Already on latest version" );
    }
  }
  else {
    Serial.print( "Firmware version check failed, got HTTP response code " );
    Serial.println(httpCode);
  }
  httpClient.end();
  action = UPDATE_ACTION_NONE;
  return false;
}