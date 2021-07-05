
#ifndef RADIO_RADIO
#define RADIO_RADIO

#define NFC_CMD_READ  0x0
#define NFC_CMD_WRITE 0x1

#define NFC_STATUS_SUCCESS  0x0
#define NFC_STATUS_FAILED   0x1

class Radio {
  public:
    void begin();
    //String loop();
    String loop();
    void writeTag(String tagString);
    void cancelWrite();
  private:
    String writeString = "";
    String readCard();
    void startListening();
    uint8_t formatTag();
    uint8_t writeNewTag();
};
#endif