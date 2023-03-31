#include <Arduino.h>
#include <U8g2lib.h>

// constructor for Display
// SH1106 is the display controller
//  127x64 is the resolution
//  NONAME is the manufacturer
//  F enables full frame buffer
//  HW enables hardware I2C
//  U8G2_R0 is the rotation of the display
//  U8X8_PIN_NONE is the reset pin
//  28 is the SDA pin
//  29 is the SCL pin
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 28, 29);

void setup(void) {
    Serial.begin(9600);
    u8g2.begin(); // initialize the display
}

/* draw something on the display with the `firstPage()`/`nextPage()` loop*/
void loop(void) {
    u8g2.firstPage();

    do {
        u8g2.setFont(u8g2_font_ncenB14_tr);
        u8g2.drawStr(0, 20, "Hello World!");

    } while (u8g2.nextPage());

    delay(1000);
}