#include "VirtualLcd.h"

MyVirtualLCD::MyVirtualLCD() {
    _slave = NULL;
}

void MyVirtualLCD::init(void* slave) {
    if (slave == NULL) return;

    _slave = (EthercatDevice_DmpLCD_Generic*)slave;
}

void MyVirtualLCD::begin(uint16_t lcdID) {
    if (_slave == NULL) return;
    
    _slave->lcdInit(lcdID);
}

int MyVirtualLCD::flush(int32_t timeout_ms) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdFlush(timeout_ms); // 0 is not empty, 1 is empty
}

int16_t MyVirtualLCD::width() {
    if (_slave == NULL) return 0;
    
    return _slave->lcdWidth();
}

int16_t MyVirtualLCD::height() {
    if (_slave == NULL) return 0;
    
    return _slave->lcdHeight();
}

uint8_t MyVirtualLCD::getRotation() {
    if (_slave == NULL) return 0;
    
    return _slave->lcdGetRotation();
}

int MyVirtualLCD::setRotation(uint8_t x) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdSetRotation(x);
}


int MyVirtualLCD::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdDrawPixel(x, y, color);
}

int MyVirtualLCD::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdDrawFastHLine(x, y, w, color);
}

int MyVirtualLCD::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdDrawFastVLine(x, y, h, color);
}

int MyVirtualLCD::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdDrawLine(x0, y0, x1, y1, color);
}

int MyVirtualLCD::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdFillRect(x, y, w, h, color);
}

int MyVirtualLCD::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdDrawRect(x, y, w, h, color);
}

int MyVirtualLCD::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdFillCircle(x, y, r, color);
}

int MyVirtualLCD::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdDrawCircle(x, y, r, color);
}

int MyVirtualLCD::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdFillTriangle(x0, y0, x1, y1, x2, y2, color);
}

int MyVirtualLCD::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdDrawTriangle(x0, y0, x1, y1, x2, y2, color);
}

int MyVirtualLCD::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdFillRoundRect(x, y, w, h, r, color);
}

int MyVirtualLCD::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdDrawRoundRect(x, y, w, h, r, color);
}

int MyVirtualLCD::fillScreen(uint16_t color) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdFillScreen(color);
}

int MyVirtualLCD::setAddrWindow(int x1, int y1, int x2, int y2) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdSetAddrWindow(x1, y1, x2, y2);
}

int MyVirtualLCD::pushColors(uint16_t *data, uint8_t len, bool first) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdPushColors(data, len, first);
}


int MyVirtualLCD::setCursor(int16_t x, int16_t y) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdSetTextCursor(x, y);
}

int MyVirtualLCD::setTextWrap(bool wrap) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdSetTextWrap(wrap);
}

int MyVirtualLCD::setTextColor(uint16_t color, uint16_t background) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdSetTextColor(color, background);
}

int MyVirtualLCD::setTextColor(uint16_t color) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdSetTextColor(color);
}

int MyVirtualLCD::setTextSize(uint8_t w, uint8_t h) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdSetTextSize(w, h);
}

int MyVirtualLCD::setTextSize(uint8_t s) {
    if (_slave == NULL) return 0;
    
    return _slave->lcdSetTextSize(s);
}


/************************
 *     Touch Screen     *
 ************************/
 int MyVirtualLCD::touchCalibration(int flag) {
    if (_slave == NULL) return 0;
    
    return _slave->touchCalibration(flag);
}

int MyVirtualLCD::touchX(size_t point) {
    if (_slave == NULL) return 0;
    
    return _slave->touchX(point);
}

int MyVirtualLCD::touchY(size_t point) {
    if (_slave == NULL) return 0;
    
    return _slave->touchY(point);
}

int MyVirtualLCD::isTouched() {
    if (_slave == NULL) return 0;
    
    return _slave->isTouched();
}

/************************
 *        Other         *
 ************************/
/*
void MyVirtualLCD::invertDisplay(bool i) {
}


void MyVirtualLCD::drawCircleHelper(int x0, int y0, int r, unsigned char cornername, unsigned int color) {
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x4) {
            drawPixel(x0 + x, y0 + y, color);
            drawPixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            drawPixel(x0 + x, y0 - y, color);
            drawPixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            drawPixel(x0 - y, y0 + x, color);
            drawPixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            drawPixel(x0 - y, y0 - x, color);
            drawPixel(x0 - x, y0 - y, color);
        }
    }
}


void MyVirtualLCD::fillCircleHelper(int x0, int y0, int r, unsigned char cornername, int delta, unsigned int color) {
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;

        if (cornername & 0x1) {
            drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
            drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
        }
        if (cornername & 0x2) {
            drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
            drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
        }
    }
}
*/

size_t MyVirtualLCD::write(const unsigned char *buffer, size_t size) {
    if (_slave == NULL) return 0;
    
    _slave->lcdPrint("%s", buffer);
    return size;
}

size_t MyVirtualLCD::write(unsigned char c) {
    if (_slave == NULL) return 0;
    
    _slave->lcdPrint("%c", c);
    return 1;
}

