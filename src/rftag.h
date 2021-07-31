
#ifndef RADIO_RADIO
#define RADIO_RADIO

#define NFC_CMD_READ  0x0
#define NFC_CMD_WRITE 0x1

#define NFC_STATUS_SUCCESS  0x0
#define NFC_STATUS_FAILED   0x1

void nfcSetup();
String nfcLoop();
void writeTag(String tagString);
void cancelWriteTag();

#endif