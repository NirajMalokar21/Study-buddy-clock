#include "OLED.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Internal display object (one instance)
static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

OLED::OLED(uint8_t sdaPin, uint8_t sclPin, uint8_t i2cAddr)
  : _sda(sdaPin), _scl(sclPin), _addr(i2cAddr) {}

bool OLED::begin() {
  Wire.begin(_sda, _scl);

  if (!display.begin(SSD1306_SWITCHCAPVCC, _addr)) {
    return false;
  }

  display.clearDisplay();
  // white color for now, can be changed later
  display.setTextColor(SSD1306_WHITE);
  setTextSize(_textSize);
  resetCursor();
  display.display();
  return true;
}

void OLED::clear() {
  display.clearDisplay();
  resetCursor();
}

void OLED::update() {
  display.display();
}

void OLED::setTextSize(uint8_t size) {
  _textSize = size;
  display.setTextSize(_textSize);
}

void OLED::resetCursor() {
  _cursorY = 0;
  display.setCursor(0, 0);
}

void OLED::printLine(const String& text) {
  // Basic line spacing depending on size (8px font height * size)
  int lineHeight = 8 * _textSize;

  display.setCursor(0, _cursorY);
  display.println(text);
  _cursorY += lineHeight;

  // Prevent running off screen (simple wrap to top)
  if (_cursorY > (SCREEN_HEIGHT - lineHeight)) {
    _cursorY = 0;
  }
}

void OLED::printAt(int x, int y, const String& text) {
  display.setCursor(x, y);
  display.print(text);
}

void OLED::showWifiStatus(const String& ssid, const String& ip) {
  clear();
  setTextSize(1);

  printLine("WiFi:");
  printLine(ssid);
  printLine("IP:");
  printLine(ip);

  update();
}

void OLED::showTextCentered(const String& text, int y, uint8_t textSize) {
  display.setTextSize(textSize);

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  int x = (SCREEN_WIDTH - w) / 2;
  display.setCursor(x, y);
  display.print(text);

  // restore current size setting
  display.setTextSize(_textSize);
}