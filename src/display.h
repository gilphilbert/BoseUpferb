#ifndef BOSE_MENU
#define BOSE_MENU

void displaySetup();
void displayLoop();

void displayTrack(String trackName, String artist);
void displayClear();

void displayWritingTag();
void displayWriteSuccess(bool failed);

void displaySleep();
void displayWake();

void navUp();
void navDown();
void navEnter();
void navEscape();

#endif