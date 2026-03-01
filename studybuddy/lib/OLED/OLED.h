#ifndef OLED_H
#define OLED_H
#include <Arduino.h>


class OLED
{
private:
    uint8_t _sda, _scl, _addr;
    uint8_t _textSize = 1;
    int _cursorY = 0;

    void resetCursor();
public:
    OLED(uint8_t sdaPin = 21, uint8_t sclPin = 22, uint8_t i2cAddr = 0x3C);

    // initializes OLED screen with the scl, sda and i2c mount
    bool begin();
    // cleans whatever is on the screen
    void clear();
    // display on screen
    void update();

    // set text size of letters
    void setTextSize(uint8_t size);
    void printLine(const String& text);
    void printAt(int x, int y, const String& text);

    void showTextCentered(const String& text, int y, uint8_t textSize = 1);
    void showWifiStatus(const String& ssid, const String& ip);
};

#endif


