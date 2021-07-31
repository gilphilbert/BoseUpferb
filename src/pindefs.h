#ifndef RADIO_PINS
#define RADIO_PINS

/* PINS */
// I2S DAC
#define I2S_WCLK      33 //13
#define I2S_BCLK      26 //14
#define I2S_DATA      25 //12

// DISPLAY (Uses SPI)
#define DISPLAY_RESET 32 //2
#define DISPLAY_DC    15 //15
#define DISPLAY_CS    27 //27

// NFC READER (Uses I2c)
#define NFC_IRQ       34
#define NFC_CS        4

// SD CARD (Uses SPI)
#define SD_CS         5

#define DAC_MUTE      14

// AMPLIFIER
#define AMP_SHUTDOWN  13

// BUTTONS (Front Panel)
#define BUTTONS_IRQ   35

#endif