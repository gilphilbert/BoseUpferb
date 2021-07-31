#ifndef BOSE_UPDATE
#define BOSE_UPDATE

#include "Arduino.h"

#define UPDATE_STATE_IDLE       0
#define UPDATE_STATE_AVAILABLE  1
#define UPDATE_STATE_UPDATING   2

#define UPDATE_WIFI_STATE_DISCONNECTED  0
#define UPDATE_WIFI_STATE_CONNECTING    1
#define UPDATE_WIFI_STATE_CONNECTED     2

#define UPDATE_ACTION_NONE      0
#define UPDATE_ACTION_CHECK     1
#define UPDATE_ACTION_UPDATE    2

class Update {
  public:
    void begin(String _ssid, String _key);
    void check();
    void update();
    void loop();
  private:
    const char* UrlBase = "http://192.168.68.124";
    const int version = 0;

    unsigned short status = UPDATE_STATE_IDLE;
    unsigned short wifiStatus = UPDATE_WIFI_STATE_DISCONNECTED;
    unsigned short action = UPDATE_ACTION_NONE;

    String ssid;
    String key;

    void _connect();
    bool _update(bool perform);
};

#endif