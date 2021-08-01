#ifndef BOSE_MENU
#define BOSE_MENU

void displaySetup();
void displayLoop();

void displayTrack(String trackName, String artist);
void displayClear();

void displayWriteSuccess(bool failed);

void navUp();
void navDown();
void navEnter();
void navEscape();

#endif