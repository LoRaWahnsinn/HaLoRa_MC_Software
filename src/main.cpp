#include <Arduino.h>
#include <Wire.h>

#define CARDKB_ADDR 0x5F

void setup(void) {
    Serial.begin(9600);
    Wire.begin();
}

/* draw something on the display with the `firstPage()`/`nextPage()` loop*/
void loop() {
    Wire.requestFrom(CARDKB_ADDR, 1);
    while (Wire.available()) {
        char c = Wire.read();  // receive a byte as characterif
        if (c != 0) {
            Serial.println(c, HEX);
            // M5.Speaker.beep();
        }
    }
    // delay(10);
}