#ifndef ___WZ_ETHERCAT_DEVICE_DMP_TOUCH_LCD_H
#define ___WZ_ETHERCAT_DEVICE_DMP_TOUCH_LCD_H

#include "EthercatDevice_DmpCommon.h"
#include <stdint.h>

/* LCD Moudle List:
    ID  LCM Driver IC   Resolution  Xp  Yp  Xm  Ym
     0    ILI9341       240 X 320   D9  A2  A3  D8
     1    ILI9341       240 X 320   D6  A1  A2  D7
     2    ILI9488       320 X 480   D8  A3  A2  D9
     3    ILI9486       320 X 480   X   X   X   X
     4    HX8347-I(T)   240 X 320   D9  A2  A3  D8
     5    HX8347-D      240 X 320   D9  A2  A3  D8
*/
#define ECAT_LCD_UNKNOWN_ID (0xFFFF)
#define ECAT_LCD_ILI9341_1  (0)
#define ECAT_LCD_ILI9341_2  (1)
#define ECAT_LCD_ILI9488_1  (2)
#define ECAT_LCD_ILI9486_1  (3)
#define ECAT_LCD_HX8347I_1  (4)
#define ECAT_LCD_HX8347D_1  (5)

class EthercatDevice_QECR11UN01;
class EthercatDevice_QECR00UN01;

class EthercatDevice_DmpLCD_Generic : public _EthercatDevice_DmpCommonDriver {

public:
    EthercatDevice_DmpLCD_Generic(uint32_t product = 0);
    ~EthercatDevice_DmpLCD_Generic();

    int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster *master, uint16_t lcd_id, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster &master, uint16_t lcd_id, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int detach();
    int update();

    /************************
     *         LCD          *
     ************************/
    /* LCD initialize. */
    int lcdInit(uint16_t lcd_id = ECAT_LCD_UNKNOWN_ID);
    /* Flush LCD command queue. */
    int lcdFlush(int32_t timeout_ms = -1);

    /* Get width of the display, accounting for current rotation. */
    int16_t lcdWidth();
    /* Get height of the display, accounting for current rotation. */
    int16_t lcdHeight();

    /* Get rotation setting for display. */
    uint8_t lcdGetRotation();
    /* Set rotation setting for display. */
    int lcdSetRotation(uint8_t x);

    /* Draw a pixel to the canvas framebuffer. */
    int lcdDrawPixel(int16_t x, int16_t y, uint16_t color);
    /* Draw a horizontal line to the canvas framebuffer. */
    int lcdDrawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    /* Draw a vertical line to the canvas framebuffer. */
    int lcdDrawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    /* Draw a line. */
    int lcdDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    /* Fill a rectangle completely with one color. */
    int lcdFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    /* Draw a rectangle with no fill color. */
    int lcdDrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    /* Draw a circle with filled color. */
    int lcdFillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    /* Draw a circle outline. */
    int lcdDrawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    /* Draw a triangle with color-fill. */
    int lcdFillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    /* Draw a triangle with no fill color.*/
    int lcdDrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    /* Draw a rounded rectangle with fill color. */
    int lcdFillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    /* Draw a rounded rectangle with no fill color. */
    int lcdDrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    /* Fill the framebuffer completely with one color. */
    int lcdFillScreen(uint16_t color);
    /* Sets the LCD address window.
       Relevant to rect/screen fills and H/V lines.  Input coordinates are
       assumed pre-sorted (e.g. x2 >= x1). */
    int lcdSetAddrWindow(int x1, int y1, int x2, int y2);
    /* Issues 'raw' an array of 16-bit color values to the LCD; used
       externally by BMP examples.  Assumes that lcdSetAddrWindow() has
       previously been set to define the bounds. */
    int lcdPushColors(uint16_t *data, uint8_t len, bool first);

    /* Set text cursor location. */
    int lcdSetTextCursor(int16_t x, int16_t y);
    /* Set whether text that is too long for the screen width should
       automatically wrap around to the next line (else clip right). */
    int lcdSetTextWrap(bool wrap);
    /* Set text font color with custom background color. */
    int lcdSetTextColor(uint16_t color, uint16_t background);
    /* Set text font color with transparant background. */
    int lcdSetTextColor(uint16_t color);
    /* Set text 'magnification' size. */
    int lcdSetTextSize(uint8_t w, uint8_t h);
    /* Set text 'magnification' size. */
    int lcdSetTextSize(uint8_t s);
    /* Print string. */
    int lcdPrint(const char *fmt, ...);

    /************************
     *     Touch Screen     *
     ************************/
    int isTouched();
    int touchX(size_t point = 0);
    int touchY(size_t point = 0);
    int touchCalibration(int flag = 0);

    /************************************************************************
     *  The following APIs are only for internal use.                       *
     ************************************************************************/
    int _loadParamsFromEcdev(void *buf, size_t size);
    int _saveParamsToEcdev(void *buf, size_t size);

private:
    void *private_data;

};

class EthercatDevice_QECR11UN01 : public EthercatDevice_DmpLCD_Generic {
public:
    EthercatDevice_QECR11UN01();
};

class EthercatDevice_QECR00UN01 : public EthercatDevice_DmpLCD_Generic {
public:
    EthercatDevice_QECR00UN01();
};

#endif
