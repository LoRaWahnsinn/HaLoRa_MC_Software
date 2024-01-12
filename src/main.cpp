#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "HT_SH1107Wire.h"

#include <./message.cpp>
#include <./screens/menu_empty.cpp>
#include <./screens/menu_empty_bigger.cpp>
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
#define numberOfMenuePages 3

// constructor for Display
// SH1106 is the display controller
//  127x64 is the resolution
//  NONAME is the manufacturer
//  F enables full frame buffer
//  HW enables hardware I2C
//  U8G2_R0 is the rotation of the display
//  U8X8_PIN_NONE is the reset pin
//  28 is the SCL pin OLD
//  29 is the SDA pin OLD
//  39 is the SCL pin NEW
//  40 is the SDA pin NEW

//original
//U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 28, 29);

//U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 39, 40);

// works but is very slow
//U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL1, SDA1, U8X8_PIN_NONE);

U8G2_SH1106_128X64_NONAME_F_2ND_HW_I2C u8g2(U8G2_R0);

SH1107Wire  display(0x3c, 500000, SDA, SCL ,GEOMETRY_128_64,GPIO10);
//SH1107Wire  display(0x3c, 500000, SDA, SCL ,GEOMETRY_128_64,38);

void printLoadingScreen();
void printThis(String text);
void printChar(char c);
void unprintChar();
void printUpperBar();
void printUpperInfoBar(String output);
void printBackArrow();
void printForwardArrow();
void printDeleteButton();
void optionsSwitch(char c);
void mainMenuPage1();
void mainMenuPage2();
void mainMenuPage3();
void printMenuPage(String firstInt, String secondInt , String fistText , String secondText, String menuTitle, int page);
void enterRecipient(String recipient);
void enterMessage(String recipient);
void showSendSuccess();
void showSendFail();
void printInboxEmpty();
void printInboxDeleted();
void displayInbox();
void printReceivingMessages();
void printSendingMessage();
void printInbox(Message* message1, Message* message2, bool hasPrevPage, bool hasNextPage);
void printContactsFriends();
void printContactsGroups();
void printMessage(Message* message, int messageNumber);
void sosOption();
void sosMessage();
void sendSOSMessageandLocation(String message);
char getCharFromKeyboard();
void removeLastChar(String* allChars);
void deleteMessageFromMessageArray(int messageNumber);
void deleteAllMessageFromMessageArray();
void addMessageToMessageArray(String messageString);
void sendUplinkForDownlink();
void contactsMenu();
String currentText = "";

bool downlinkMessagesQueued = false;

// initialize array of messages with null
Message* messages[100] = {NULL};

void setup(void) {

    //activate VEXT 3.3V output
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
    
    // we have to wait for the display to power up before initializing it
    delay(1000);

    Serial.begin(115200);
    //Wire1.begin(SDA1, SCL1, 400000);
    //u8g2.setBusClock(400000);
    //u8g2.setI2CAddress(0x3C * 2);
    u8g2.begin(); // initialize the display
    u8g2.setCursor(0, 15);
    u8g2.setFont(u8g2_font_6x13_tf);

    //Wire.begin();

    Message* message1 = new Message("testSender1", "Message1");
    Message* message2 = new Message("testSender2", "Message2");
    Message* message3 = new Message("testSender3", "Message3");

    messages[0] = message1;
    messages[1] = message2;
    messages[2] = message3;

    display.init();
    //display.setFont(ArialMT_Plain_10);
    display.setFont(ArialMT_Plain_16); //bigger font

    display.clear();

    display.screenRotate(ANGLE_270_DEGREE);

    display.drawString(3, 3, "HaLoRa");
    display.drawString(3, 23, "Test");
    display.drawString(3, 43, "Hello");
    display.drawString(3, 63, "World");
    // write the buffer to the display
    display.display();

/*
    LoRaWAN.begin(LORAWAN_CLASS, ACTIVE_REGION);

    LoRaWAN.setAdaptiveDR(false);

    printLoadingScreen();

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
*/

}

//main menu
void loop() {
    mainMenuPage1();
}

char getCharFromKeyboard(){
    while(true){
        Wire.requestFrom(CARDKB_ADDR, 1);
        while (Wire.available()) {
            char c = Wire.read();  // receive a byte as character

            if (c != 0) {
                Serial.println(c, HEX);
                return c;
            }
        }
    }
}

// ------- LOGIC FOR DIFFERENT SCREENS -------

void printLoadingScreen()
{
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_fub20_tf);
    
    u8g2.drawStr(15, 30, "HaLoRa");
    
    u8g2.setFont(u8g2_font_6x13_tf);

    u8g2.drawStr(25, 45, "connecting to");
    u8g2.drawStr(35, 55, "network...");

    u8g2.sendBuffer();
}

void optionsSwitch(char c){
        switch (c) {
        case '1':  // 1
            enterRecipient("");
            break;
        case '2':
            printReceivingMessages();
            sendUplinkForDownlink();
            while (downlinkMessagesQueued) {
                sendUplinkForDownlink();
            }
            displayInbox();
            break;
        case '3':
            //contactsMenu();
            break;
        case '4':
            deleteAllMessageFromMessageArray();
            printInboxDeleted();
            break;
        case '5':
            sosOption();
           
            break;
        case '6':
            //stopwatch();
            break;  
        }

}

void mainMenuPage1(){
    // draw main menu
    printMenuPage("1","2","Send Message","Recv. Message ", "Menu", 1);

    char c = getCharFromKeyboard();


        if(c == 0xB6)  // down key
          mainMenuPage2();
        if(c > 0x30 && c < 0x37) 
          optionsSwitch(c);
}

void mainMenuPage2() {
    // draw main menu
    
    printMenuPage("3","4","contacts","delete inbox", "Menu", 2);

    char c = getCharFromKeyboard();
    if(c == 0xB6)  // down key
          mainMenuPage3();
    if(c == 0xB5)  // up key
          mainMenuPage1();

    if(c > 0x30 && c < 0x37) 
          optionsSwitch(c);
    


}

void mainMenuPage3(){
    //draw main menu
    printMenuPage("5","6","SOS","Stopwatch", "Menu", 3);
     char c = getCharFromKeyboard();
     if(c == 0xB5)  // up key
          mainMenuPage2();

    if(c > 0x30 && c < 0x37) 
          optionsSwitch(c);
     
}

void printMenuPage(String firstInt ,String secondInt, String firstText, String secondText, String menuTitle, int page){
    u8g2.clearBuffer();
    u8g2.setDrawColor(0);  // otherwise XBM is inverted
    u8g2.drawXBM(0, 12, menu_empty_bigger_width, menu_empty_bigger_height, menu_empty_bigger_bits);
    printUpperInfoBar(menuTitle);
   
    u8g2.setDrawColor(1);
    //print menu options
    //int
    u8g2.drawStr(11, 31, String(firstInt).c_str());
    u8g2.drawStr(11, 52, String(secondInt).c_str());
    //text
    u8g2.drawStr(28, 31, String(firstText).c_str());
    u8g2.drawStr(28, 53, String(secondText).c_str());	

    //print arrows
    u8g2.setDrawColor(0);

    //down arrow
    if(page < numberOfMenuePages  ){
        u8g2.drawXBM(118, 50, down_arrow_width, down_arrow_height, down_arrow_bits);
    }

    //up arrow
    if (page > 1 && page <= numberOfMenuePages){
        
        u8g2.drawXBM(118, 15, up_arrow_width, up_arrow_height, up_arrow_bits);
       

    }
    
    u8g2.sendBuffer();
    
}

void contactsMenu(){
    //print menu
    printMenuPage("1","2","","friends ", "groups", numberOfMenuePages+1);
    
    char c = getCharFromKeyboard();
    switch (c){
        case '1':
            //go to freinds menu
            break;
        case '2':
            //go to groups menu
            break;
        case 0xB4:  // back key
            //go back to main menu
            break;
    }

}

void freindsMenu(){
    //freinds list 
    printMenuPage("1","2", "reload freinds", "friend1","freinds", numberOfMenuePages+1);
    char c = getCharFromKeyboard();
    switch (c){
        case '1':
            //reload freinds
            break;
        case '2':
            //reload groups
            break;
        case 0xB4:  // back key
            //go back to contacts menu
            break;
    }

}
void groupsqMenu(){
    printMenuPage("1","2", "reload groups", "group1","groups", numberOfMenuePages+1);

}

void enterRecipient(String oldRecipient){
    u8g2.setDrawColor(0);

    //print bitmap of enter_recipient screen
    u8g2.clearBuffer();
    printUpperBar();
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

    printUpperBar();   
    printBackArrow();
    printForwardArrow();


    u8g2.setDrawColor(0); //because we want to write the text in black

    //show recipient at top of screen
    u8g2.drawStr(5, 10, String("To: " + recipient).c_str());
    
    u8g2.sendBuffer();

    u8g2.setDrawColor(1);


    String message = "";
    String messageAndRecipient;
    u8g2.setCursor(5, 22);

    //TODO: Line breaks in message

    while (true) {
        char c = getCharFromKeyboard();

        switch (c) {
            case 0xB4:  // back key
                enterRecipient(recipient);
                return;
            case 0xB7:  // forward key
                
                printSendingMessage();

                messageAndRecipient = recipient + ";" + message;

                if (LoRaWAN.send(messageAndRecipient.length(), (uint8_t*)messageAndRecipient.c_str(), 1, true)) {
                    Serial.println("Send OK");
                    showSendSuccess();
                } else {
                    Serial.println("Send FAILED");
                    showSendFail();
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

void printReceivingMessages() {
    u8g2.setDrawColor(0);
    u8g2.clearBuffer();

    printUpperBar();

    u8g2.setDrawColor(1);

    // show sending
    u8g2.drawStr(40, 35, "receiving");
    u8g2.drawStr(35, 47, "messages...");

    u8g2.sendBuffer();
}

void printSendingMessage() {
    u8g2.setDrawColor(0);
    u8g2.clearBuffer();

    printUpperBar();

    u8g2.setDrawColor(1);

    // show sending
    u8g2.drawStr(42, 35, "sending");
    u8g2.drawStr(37, 47, "message...");

    u8g2.sendBuffer();
}

void showSendSuccess() {
    
    u8g2.setDrawColor(0);
    u8g2.clearBuffer();
    
    printUpperBar();
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

    printUpperBar();
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

void printInboxDeleted() {
    u8g2.setDrawColor(0);
    u8g2.clearBuffer();

    printUpperBar();
    printBackArrow();

    u8g2.setDrawColor(1);

    // show sucess message
    u8g2.drawStr(25, 40, "inbox deleted!");

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

    printUpperBar();
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

void sosOption(){
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    
    printUpperBar();
    printBackArrow();
    printForwardArrow();
    //write SOS in Battery Bar
    u8g2.setDrawColor(0); //because we want to write the text in black
    u8g2.drawStr(5, 10, "SOS");
    //write 
    u8g2.setDrawColor(1);
    u8g2.drawStr(5, 27, "SOS message contains");
    u8g2.drawStr(5, 38, "- location");
    u8g2.drawStr(5, 49, "- message (optional)");

    u8g2.sendBuffer();
    u8g2.setDrawColor(0);
    while(true){
        char c = getCharFromKeyboard();

        switch (c) {
            case 0xB4:  // back key
                return;
            case 0xB7:  // forward key
                sosMessage();
                return;
            default:
                break;
        }
    }

}
void sendSOSMessageAndLocation(String message){
    //todo send sos message
    //todo send location
    if(message=="1"){
        printSendingMessage();
        delay(3000);
        showSendSuccess();
        
        
    }
    else{
        showSendFail();
        delay(3000);
        //print try again

    }

    
    return;
}


void sosMessage(){
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    
    printUpperBar();
    printBackArrow();
    printForwardArrow();
    //write SOS in Battery Bar
    u8g2.setDrawColor(0); //because we want to write the text in black
    u8g2.drawStr(5, 10, "SOS message:");
    
    //write message
    u8g2.setDrawColor(1);
    String message = "";

        u8g2.setCursor(5, 22);

    //TODO: Line breaks in message

    while (true) {
        char c = getCharFromKeyboard();

        switch (c) {
            case 0xB4:  // back key
                sosOption();
                return;
            case 0xB7:  // forward key
                
                sendSOSMessageAndLocation(message);

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

void printUpperBar(){
    u8g2.setDrawColor(1);
    u8g2.drawBox(0, 0, 128, 11);
}

void printUpperInfoBar(String output){
    u8g2.setDrawColor(1);
    u8g2.drawBox(0, 0, 128, 11);
    u8g2.setDrawColor(0);
    u8g2.drawStr(4, 10, output.c_str());
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
    printUpperBar();
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

void printMessage(Message* message, int messageNumber){

    u8g2.setDrawColor(0);
    u8g2.clearBuffer();

    printUpperBar();
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
                deleteMessageFromMessageArray(i);
                displayInbox();
                return;
             default:
                break;
        }
    }
}

void deleteMessageFromMessageArray(int messageNumber){
    while (true) {
        messages[messageNumber] = messages[messageNumber + 1];
        if (messages[messageNumber] == NULL) {
             break;
        }
        messageNumber++;
    }
}

void deleteAllMessageFromMessageArray() {
    int i = 0;
    while (i < 100) {
        if (messages[i] == NULL) {
            break;
        }
        messages[i] = NULL;
        i++;
    }
}

void addMessageToMessageArray(String messageString){
    int i = 0;
    while (true) {
        if (messages[i] == NULL) {
            messages[i] = new Message(messageString);
            break;
        }
        i++;
    }
}

void sendUplinkForDownlink(){
    Serial.println("Sending uplink for downlink");

    String message = "uplink";

    if (LoRaWAN.send(message.length(), (uint8_t*)message.c_str(), 1, true)) {
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

    Serial.println("Received Message:");
    Serial.println(receivedString);

    //add received message to message array
    addMessageToMessageArray(receivedString);

    //if there are more downlink messages to be received, send another uplink
    if (mcpsIndication->FramePending) {
        Serial.println("Frame pending, sending uplink");
        downlinkMessagesQueued = true;
    } else {
        Serial.println("No more downlink messages");
        downlinkMessagesQueued = false;
    }
}
