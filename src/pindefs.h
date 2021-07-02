#ifndef RADIO_PINS
#define RADIO_PINS

/* PINS */
// I2S DAC
#define I2S_WCLK      13
#define I2S_BCLK      14
#define I2S_DATA      12

// DISPLAY (Uses SPI)
#define DISPLAY_RESET 2
#define DISPLAY_DC    15
#define DISPLAY_CS    27

// NFC READER (Uses I2c)
#define NFC_IRQ       34

// SD CARD (Uses SPI)
#define SD_CS         5

// AMPLIFIER
#define AMP_MUTE      26
#define AMP_SHUTDOWN  4

// BUTTONS (Front Panel)
#define BUTTONS_IRQ   35

#endif