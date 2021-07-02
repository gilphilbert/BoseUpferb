
#ifndef RADIO_DISP
#define RADIO_DISP

class Display {
  public:
    void begin();
    void updateScreen(const char* title, const char* artist);
    void welcome();
};
#endif