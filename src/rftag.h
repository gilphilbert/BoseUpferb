
#ifndef RADIO_RADIO
#define RADIO_RADIO

class Radio {
  public:
    void begin();
    String loop();
  private:
    String readCard();
    void startListening();
};
#endif