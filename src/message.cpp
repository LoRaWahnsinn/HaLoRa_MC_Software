#include <Arduino.h>

class Message {
   private:
    String sender;
    String message;
    bool read;

   public:
    // Default constructor
    Message() {
        sender = "";
        message = "";
        read = false;
    }

    // Parameterized constructor
    Message(const String& sender, const String& message) {
        this->sender = sender;
        this->message = message;
        read = false;
    }

    // Getter and setter for the sender
    String getSender() const { return sender; }

    void setSender(const String& sender) { this->sender = sender; }

    // Getter and setter for the message
    String getMessage() const { return message; }

    void setMessage(const String& message) { this->message = message; }

    // Getter and setter for the read status
    bool isRead() const { return read; }

    void setRead(bool read) { this->read = read; }
};