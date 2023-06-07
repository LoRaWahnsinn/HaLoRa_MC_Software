#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include <./message.cpp>
#include <./screens/back_arrow.cpp>
#include <./screens/battery_bar.cpp>
#include <./screens/down_arrow.cpp>
#include <./screens/up_arrow.cpp>
#include <./screens/enter_recipient.cpp>
#include <./screens/forward_arrow.cpp>
#include <./screens/home_screen.cpp>
#include <./screens/home_screen_2.cpp>
#include <./screens/message_box/message_box_1_read.cpp>
#include <./screens/message_box/message_box_1_unread.cpp>
#include <./screens/message_box/message_box_2_read.cpp>
#include <./screens/message_box/message_box_2_unread.cpp>

#include "Arduino.h"
#include "LoRaWanMinimal_APP.h"

static uint8_t devEui[] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0xE0, 0xA2};
static uint8_t appEui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t appKey[] = {0x76, 0xE9, 0x0A, 0xDB, 0x62, 0x9F, 0x63, 0x05, 0xCD, 0x39, 0x77, 0xF6, 0xA0, 0xB6, 0xF6, 0x48};

uint16_t userChannelsMask[6] = {0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

#define CARDKB_ADDR 0x5F
#define heigth 64
#define width 128

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
void printBatteryBar();
void printBackArrow();
void printForwardArrow();
void printDeleteButton();
void mainMenuPage1();
void mainMenuPage2();
void enterRecipient(String recipient);
void enterMessage(String recipient);
void showSendSuccess();
void showSendFail();
void printInboxEmpty();
void displayInbox();
void printInbox(Message* message1, Message* message2, bool hasPrevPage, bool hasNextPage);
void printMessage(Message* message, int messageNumber);
char getCharFromKeyboard();
void removeLastChar(String* allChars);
void sendUplinkForDownlink();
void showSending();

String currentText = "";

bool downlinkMessagesQueued = false;

// initialize array of messages with null
Message* messages[100] = {NULL};

void setup(void) {
    //activate VEXT 3.3V output
    pinMode(GPIO6, OUTPUT);
    digitalWrite(GPIO6, LOW);

    Serial.begin(115200);
    u8g2.begin(); // initialize the display
    u8g2.setCursor(0, 15);
    u8g2.setFont(u8g2_font_6x13_tf);


    Wire.begin();

    Message* message1 = new Message("testSender1", "Message1");
    Message* message2 = new Message("testSender2", "Message2");
    Message* message3 = new Message("testSender3", "Message3");
    Message* message4 = new Message("testSender4", "Message4");
    Message* message5 = new Message("testSender5", "Message5");

    messages[0] = message1;
    messages[1] = message2;
    messages[2] = message3;
    messages[3] = message4;
    messages[4] = message5;

        LoRaWAN.begin(LORAWAN_CLASS, ACTIVE_REGION);

        LoRaWAN.setAdaptiveDR(false);

    //wait until joined
    while (1) {
        Serial.print("Joining... \n");
        LoRaWAN.joinOTAA(appEui, appKey, devEui);
        if (!LoRaWAN.isJoined()) {
            Serial.println("JOIN FAILED!\n");
        } else {
            Serial.println("JOINED.");
            break;
        }
    }

    // We request confirmation packets
    bool requestack = true;

    char msg[] = "Test message";

    if (LoRaWAN.send(sizeof(msg) - 1, (uint8_t*)msg, 1, requestack)) {
        Serial.println("SENT");
    } else {
        Serial.println("SEND FAILED");
    }

    while(downlinkMessagesQueued){
        sendUplinkForDownlink();
    }

}

//main menu
void loop() {

    mainMenuPage1();
    // B6: down key
    // B5: up key
}


char getCharFromKeyboard(){
    while(true){
        Wire.requestFrom(CARDKB_ADDR, 1);
        while (Wire.available()) {
            char c = Wire.read();  // receive a byte as character

            if (c != 0) {
                //Serial.println(c, HEX);
                return c;
            }
        }
    }
}

// ------- LOGIC FOR DIFFERENT SCREENS -------

void mainMenuPage1(){
    // draw main menu
    u8g2.clearBuffer();
    u8g2.setDrawColor(0);  // otherwise XBM is inverted
    u8g2.drawXBM(0, 0, home_screen_width, home_screen_height, home_screen_bits);
    u8g2.drawStr(87, 10, "25%");  // TODO: Actual battery percentage
    u8g2.sendBuffer();

    char c = getCharFromKeyboard();

    switch (c) {
        case '1':  // 1
            enterRecipient("");
            break;
        case '2':
            // Serial.println("show received messages");
            // displayInbox();

            sendUplinkForDownlink();
            while (downlinkMessagesQueued) {
                sendUplinkForDownlink();
            }

            break;
        case '3':
            // TODO: clear inbox
            Serial.println("clear inbox");
            break;
        case 0xB6:  // down key
            mainMenuPage2();
            break;
        default:
            //mainMenuPage1();
            break;
    }
}

void mainMenuPage2() {
    // draw main menu
    u8g2.clearBuffer();
    u8g2.setDrawColor(0);  // otherwise XBM is inverted
    u8g2.drawXBM(0, 0, home_screen_2_width, home_screen_2_height, home_screen_2_bits);
    u8g2.drawStr(87, 10, "25%");  // TODO: Actual battery percentage
    u8g2.sendBuffer();

    char c = getCharFromKeyboard();

    switch (c) {
        case '1':  // 1
            enterRecipient("");
            break;
        case '2':
            // TODO: show received messages
            Serial.println("show received messages");
            break;
        case '3':
            // TODO: clear inbox
            Serial.println("clear inbox");
            break;
        case 0xB5:  // up key
            mainMenuPage1();
            break;
        default:
            mainMenuPage2();
            break;
    }
}

void enterRecipient(String oldRecipient){
    u8g2.setDrawColor(0);

    //print bitmap of enter_recipient screen
    u8g2.clearBuffer();
    printBatteryBar();
    u8g2.setDrawColor(0);  // otherwise XBM is inverted
    u8g2.drawXBM(20, 17, enter_recipient_width, enter_recipient_height, enter_recipient_bits);
    printForwardArrow();
    printBackArrow();
    u8g2.sendBuffer();

    u8g2.setDrawColor(1);

    String recipient = "";
    recipient += oldRecipient;
    u8g2.setCursor(25, 43);
    
    u8g2.print(oldRecipient);
    u8g2.sendBuffer();

    while(true){
        char c = getCharFromKeyboard();

        switch (c) {
            case 0xB4:  // back key
                return;
            case 0xB7:  // forward key
                enterMessage(recipient);
                return;
            case 0x8: //delete key
                removeLastChar(&recipient);
                break;
            default:

                if (recipient.length() > 10) break;

                recipient += c;

                u8g2.print(c);
                u8g2.sendBuffer();
        }
    }
}

void enterMessage(String recipient) {
    
    u8g2.setDrawColor(0);
    u8g2.clearBuffer();

    printBatteryBar();
    printBackArrow();
    printForwardArrow();


    u8g2.setDrawColor(0); //because we want to write the text in black

    //show recipient at top of screen
    u8g2.drawStr(5, 10, String("To " + recipient).c_str());
    
    u8g2.sendBuffer();

    u8g2.setDrawColor(1);


    String message = "";
    u8g2.setCursor(5, 22);

    //TODO: Line breaks in message

    while (true) {
        char c = getCharFromKeyboard();

        switch (c) {
            case 0xB4:  // back key
                enterRecipient(recipient);
                return;
            case 0xB7:  // forward key
                //TODO: send message

                showSending();

                if (LoRaWAN.send(sizeof(message.c_str()) - 1, (uint8_t*)message.c_str(), 1, true)) {
                    Serial.println("Send OK");
                    showSendSuccess();
                } else {
                    Serial.println("Send FAILED");
                    showSendFail();
                }

                while (downlinkMessagesQueued) {
                    sendUplinkForDownlink();
                }

                return;
            case 0x8:  // delete key
                removeLastChar(&message);
                break;
            default:

                if (message.length() > 1000) break; //TODO: change to LoRa max message length

                message += c;

                u8g2.print(c);
                u8g2.sendBuffer();
        }
    }
}

void showSending() {
    
    u8g2.setDrawColor(0);
    u8g2.clearBuffer();
    
    printBatteryBar();

    u8g2.setDrawColor(1);

    // show sending
    u8g2.drawStr(27, 40, "sending...");

    u8g2.sendBuffer();
}

void showSendSuccess() {
    
    u8g2.setDrawColor(0);
    u8g2.clearBuffer();
    
    printBatteryBar();
    printBackArrow();

    u8g2.setDrawColor(1);

    // show sucess message
    u8g2.drawStr(27, 40, "message sent!");

    u8g2.sendBuffer();

    while (true) {
        char c = getCharFromKeyboard();

        switch (c) {
            case 0xB4:  // back key
                return; //back to main
            default:
                break;
        }
    }
}

void printInboxEmpty() {
    u8g2.setDrawColor(0);
    u8g2.clearBuffer();

    printBatteryBar();
    printBackArrow();

    u8g2.setDrawColor(1);

    // show sucess message
    u8g2.drawStr(30, 40, "inbox empty");

    u8g2.sendBuffer();

    while (true) {
        char c = getCharFromKeyboard();

        switch (c) {
            case 0xB4:   // back key
                return;  // back to main
            default:
                break;
        }
    }
}

void showSendFail() {
    u8g2.setDrawColor(0);
    u8g2.clearBuffer();

    printBatteryBar();
    printBackArrow();

    // show sucess message
    u8g2.drawStr(25, 32, "error sending");
    u8g2.drawStr(40, 45, "message!");

    u8g2.sendBuffer();

    while (true) {
        char c = getCharFromKeyboard();

        switch (c) {
            case 0xB4:   // back key
                return;  // back to main
            default:
                break;
        }
    }
}

void removeLastChar(String* allChars){
    if (allChars->length() == 0) {
        return;
    }

    char last_char = allChars->charAt(allChars->length() - 1);
    *allChars = allChars->substring(0, allChars->length() - 1);

    u8g2.setCursor(u8g2.getCursorX() - 6, u8g2.getCursorY());

    u8g2.setDrawColor(2);
    u8g2.print(last_char);
    u8g2.setDrawColor(1);

    u8g2.setCursor(u8g2.getCursorX() - 6, u8g2.getCursorY());

    u8g2.sendBuffer();
}

void displayInbox(){

    if (messages[0] == NULL) {
        printInboxEmpty();
        return;
    }

    int numberOfMessags = 0;

    for (int i = 0; i < 100; i++) {
        if (messages[i] == NULL) {
            numberOfMessags = i;
            break;
        }
    }

    int numberOfPages = (numberOfMessags + 1) / 2; // +1 to account for odd number of pages, /2 because 2 messages per page
    
    int currentPage = 0;

    while (true) {

        Message* upperMessage = messages[currentPage * 2];
        Message* lowerMessage = messages[currentPage * 2 + 1];

        printInbox(upperMessage, lowerMessage, currentPage != 0, currentPage != numberOfPages - 1);

        char c = getCharFromKeyboard();

        switch (c) {
            case 0xB4:   // back key
                return;  // back to main
            case 0xB6:   // down key - next page of messages
                //TODO: print next page of messages
                if (currentPage < numberOfPages - 1) {
                    currentPage++;
                }
                break;
            case 0xB5:  // up key - previous page of messages
                //TODO: print previous page of messages
                if (currentPage > 0) {
                    currentPage--;
                }
                break;
            case '1': //select upper message
                if (upperMessage != NULL) {
                    upperMessage->setRead(true);
                    printMessage(upperMessage, currentPage * 2);
                }
                return;
            case '2': //select lower message
                if (lowerMessage != NULL) {
                    lowerMessage->setRead(true);
                    printMessage(lowerMessage, currentPage * 2 + 1);
                }
                return;
            default:
                break;
        }
    }
}

void printBatteryBar(){
    u8g2.setDrawColor(0);
    u8g2.drawXBM(0, 0, battery_bar_width, battery_bar_height, battery_bar_bits);
    u8g2.drawStr(87, 10, "25%");  // TODO: Actual battery percentage
}

void printBackArrow(){
    u8g2.setDrawColor(0);
    u8g2.drawXBM(2, heigth - 9, back_arrow_width, back_arrow_height, back_arrow_bits);
    u8g2.setDrawColor(1);
    u8g2.drawStr(10, heigth - 2, "back");
    u8g2.sendBuffer();
}

void printForwardArrow(){
    u8g2.setDrawColor(0);
    u8g2.drawXBM(122, heigth - 9, forward_arrow_width, forward_arrow_height, forward_arrow_bits);
    u8g2.setDrawColor(1);
    u8g2.drawStr(width - 32, heigth - 2, "next");
    u8g2.sendBuffer();
}

void printDeleteButton() {
    u8g2.setDrawColor(0);
    u8g2.drawXBM(122, heigth - 9, forward_arrow_width, forward_arrow_height,
                 forward_arrow_bits);
    u8g2.setDrawColor(1);
    u8g2.drawStr(width - 45, heigth - 2, "delete");
    u8g2.sendBuffer();
}

void printInbox(Message* message1, Message* message2, bool hasPrevPage, bool hasNextPage){
    u8g2.clearBuffer();
    printBatteryBar();
    printBackArrow();

    u8g2.setDrawColor(0);
    
    //print up and down arrows
    if(hasPrevPage){
        u8g2.drawXBM(width-11, 15, up_arrow_width, up_arrow_height, up_arrow_bits);
    }
    if(hasNextPage){
        //128-11 14
        u8g2.drawXBM(width-11, heigth - 11  , down_arrow_width, down_arrow_height, down_arrow_bits);
    }
    
    //print right messageBox
    //MessageBox 1
    if(message1->isRead()){
        u8g2.drawXBM(5, 17, message_box_1_read_width, message_box_1_read_height, message_box_1_read_bits);
    }
    else{
        u8g2.drawXBM(5, 17, message_box_1_unread_width, message_box_1_unread_height, message_box_1_unread_bits);
    }
    
    //MessageBox 2
    if(message2 != NULL){
        if (message2->isRead()) {
            u8g2.drawXBM(5, 35, message_box_2_read_width,
                         message_box_2_read_height, message_box_2_read_bits);
        } else {
            u8g2.drawXBM(5, 35, message_box_2_unread_width,
                         message_box_2_unread_height, message_box_2_unread_bits);
        }
    }

    //print messages
    u8g2.setDrawColor(1);
    
    u8g2.drawStr(32,28, message1->getSender().c_str());


    if(message2 != NULL){
        //Message Info 2
        u8g2.drawStr(32, 46, message2->getSender().c_str());
    }

    u8g2.sendBuffer();
    
}

void printMessage(Message* message, int messageNumber) {

    u8g2.setDrawColor(0);
    u8g2.clearBuffer();

    printBatteryBar();
    printBackArrow();
    printDeleteButton();

    u8g2.setDrawColor(0);  // because we want to write the text in black

    // show recipient at top of screen
    u8g2.drawStr(5, 10, message->getSender().c_str());

    // now we can switch to white text
    u8g2.setDrawColor(1);

    // show message body
    u8g2.drawStr(5, 25, message->getMessage().c_str());

    u8g2.sendBuffer();

    while (true) {
        char c = getCharFromKeyboard();
        int i = messageNumber;

        switch (c) {
            case 0xB4:   // back key
                displayInbox();
                return;  // back to message list
            case 0xB7: // forward key - delete message

                while (true) {
                    messages[i] = messages[i + 1];
                    if(messages[i] == NULL){
                        break;
                    }
                    i++;
                }
                displayInbox();
                return;
             default:
                break;
        }
    }
}

void sendUplinkForDownlink(){
    Serial.println("Sending uplink for downlink");

    //sleep for 1 second
    delay(3000);

    String message = "HelloWorld";

    if (LoRaWAN.send(sizeof(message.c_str()) - 1, (uint8_t*)message.c_str(), 1, true)) {
        Serial.println("Send OK");
    } else {
        Serial.println("Send FAILED");
    }
}

void downLinkDataHandle(McpsIndication_t* mcpsIndication) {
    Serial.printf("Received downlink: %s, RXSIZE %d, PORT %d, DATA: ",
                  mcpsIndication->RxSlot ? "RXWIN2" : "RXWIN1",
                  mcpsIndication->BufferSize, mcpsIndication->Port);

    Serial.printf("RxData: %d", mcpsIndication->RxData);
    Serial.printf("FramePending: %d", mcpsIndication->FramePending);

    char receivedString[mcpsIndication->BufferSize +
                        1];  // +1 for null terminator
    int stringLength = 0;

    for (uint8_t i = 0; i < mcpsIndication->BufferSize; i++) {
        receivedString[i] = (char)mcpsIndication->Buffer[i];
        stringLength++;
    }
    receivedString[stringLength] =
        '\0';  // Add null terminator to mark the end of the string

    Serial.println(receivedString);

    //if there are more downlink messages to be received, send another uplink
    if (mcpsIndication->FramePending) {
        Serial.println("Frame pending, sending uplink");
        downlinkMessagesQueued = true;
    } else {
        Serial.println("No more downlink messages");
        downlinkMessagesQueued = false;
    }
}
