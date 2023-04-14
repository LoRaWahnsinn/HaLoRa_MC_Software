#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#define CARDKB_ADDR 0x5F

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

void printThis(String text);
void printChar(char c);
void unprintChar();

String currentText = "";

void setup(void) {
    // activate VEXT 3.3V output
    pinMode(GPIO6, OUTPUT);
    digitalWrite(GPIO6, LOW);

    Serial.begin(9600);
    u8g2.begin(); // initialize the display
    u8g2.setCursor(0, 15);
    u8g2.setFont(u8g2_font_t0_11_tr);

    Wire.begin();

    //printThis(currentText);
}

/* draw something on the display with the `firstPage()`/`nextPage()` loop*/
void loop() {
    Wire.requestFrom(CARDKB_ADDR, 1);
    while (Wire.available()) {
        char c = Wire.read();  // receive a byte as characterif
        if (c != 0) {
            Serial.println(c, HEX);
            if(c == 0x8){
                //remove last char from currentText
                currentText = currentText.substring(0, currentText.length() - 1);
                //remove last char from display
                unprintChar();
            } else if (c == 0xA){
                currentText += "\n";
            } else {
                printChar(c);
            }
        }
    }
}


void printThis(String text) {
    u8g2.clearBuffer();
    u8g2.drawStr(0, 20, text.c_str());
    u8g2.sendBuffer();
}

void printChar(char c) {
    u8g2.print(c);
    u8g2.sendBuffer();
}

// void unprintChar(){
//     //set cursor position to the left of the last char
//     u8g2.setCursor(u8g2.getCursorX() - 6, u8g2.getCursorY());
//     u8g2.print((char) 32);
//     u8g2.sendBuffer();
// }