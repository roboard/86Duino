#ifndef __VIRTUAL_LCD_H
#define __VIRTUAL_LCD_H

#include "Arduino.h"
#include "Ethercat.h"

// Color definitions
#define ILI9341_BLACK       0x0000 ///<   0,   0,   0
#define ILI9341_NAVY        0x000F ///<   0,   0, 123
#define ILI9341_DARKGREEN   0x03E0 ///<   0, 125,   0
#define ILI9341_DARKCYAN    0x03EF ///<   0, 125, 123
#define ILI9341_MAROON      0x7800 ///< 123,   0,   0
#define ILI9341_PURPLE      0x780F ///< 123,   0, 123
#define ILI9341_OLIVE       0x7BE0 ///< 123, 125,   0
#define ILI9341_LIGHTGREY   0xC618 ///< 198, 195, 198
#define ILI9341_DARKGREY    0x7BEF ///< 123, 125, 123
#define ILI9341_BLUE        0x001F ///<   0,   0, 255
#define ILI9341_GREEN       0x07E0 ///<   0, 255,   0
#define ILI9341_CYAN        0x07FF ///<   0, 255, 255
#define ILI9341_RED         0xF800 ///< 255,   0,   0
#define ILI9341_MAGENTA     0xF81F ///< 255,   0, 255
#define ILI9341_YELLOW      0xFFE0 ///< 255, 255,   0
#define ILI9341_WHITE       0xFFFF ///< 255, 255, 255
#define ILI9341_ORANGE      0xFD20 ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5 ///< 173, 255,  41
#define ILI9341_PINK        0xFC18 ///< 255, 130, 198

#define BLACK       0x0000
#define NAVY        0x000F
#define DARKGREEN   0x03E0
#define DARKCYAN    0x03EF
#define MAROON      0x7800
#define PURPLE      0x780F
#define OLIVE       0x7BE0
#define LIGHTGREY   0xC618
#define DARKGREY    0x7BEF 
#define BLUE        0x001F 
#define GREEN       0x07E0 
#define CYAN        0x07FF 
#define RED         0xF800 
#define MAGENTA     0xF81F 
#define YELLOW      0xFFE0 
#define WHITE       0xFFFF 
#define ORANGE      0xFD20 
#define GREENYELLOW 0xAFE5 
#define PINK        0xFC18

#define TFT_BLACK       0x0000
#define TFT_NAVY        0x000F
#define TFT_DARKGREEN   0x03E0
#define TFT_DARKCYAN    0x03EF
#define TFT_MAROON      0x7800
#define TFT_PURPLE      0x780F
#define TFT_OLIVE       0x7BE0
#define TFT_LIGHTGREY   0xC618
#define TFT_DARKGREY    0x7BEF 
#define TFT_BLUE        0x001F 
#define TFT_GREEN       0x07E0 
#define TFT_CYAN        0x07FF 
#define TFT_RED         0xF800 
#define TFT_MAGENTA     0xF81F 
#define TFT_YELLOW      0xFFE0 
#define TFT_WHITE       0xFFFF 
#define TFT_ORANGE      0xFD20 
#define TFT_GREENYELLOW 0xAFE5 
#define TFT_PINK        0xFC18

class MyVirtualLCD : public Print
{
    public:
        MyVirtualLCD();
        
        void init(void* slave); // used for code generation by 86EVA configurator
        void begin(uint16_t lcdID);
        
        int flush(int32_t timeout_ms = -1); 
        int16_t width(); // Adafruit_GFX.h
        int16_t height(); // Adafruit_GFX.h
        uint8_t getRotation(); // Adafruit_GFX.h
        int setRotation(uint8_t x); // Adafruit_TFTLCD.h

        // Adafruit_TFTLCD.h
        int drawPixel(int16_t x, int16_t y, uint16_t color);
        int drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
        int drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
        int drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
        int fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        int drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        int fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
        int drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
        int fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
        int drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
        int fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
        int drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
        int fillScreen(uint16_t color);
        int setAddrWindow(int x1, int y1, int x2, int y2);
        int pushColors(uint16_t *data, uint8_t len, bool first);
         // Adafruit_GFX.h
        int setCursor(int16_t x, int16_t y);
        int setTextWrap(bool wrap);
        int setTextColor(uint16_t color, uint16_t background);
        int setTextColor(uint16_t color);
        int setTextSize(uint8_t w, uint8_t h);
        int setTextSize(uint8_t s);

        /************************
         *     Touch Screen     *
         ************************/
        int touchCalibration(int flag = 0);
        int touchX(size_t point = 0);
        int touchY(size_t point = 0);
        int isTouched();
    
        uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) {
            return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
        }
        
        virtual size_t write(const unsigned char *buffer, size_t size);
        virtual size_t write(unsigned char);
        // using Print::write; // pull in write(str) and write(buf, size) from Print
    
    private:
        EthercatDevice_DmpLCD_Generic* _slave;
};

#endif