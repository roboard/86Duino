#ifndef LV_OSC_H
#define LV_OSC_H

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include "Arduino.h"
#include "../lv_conf.h"
#include "lvgl.h"
#else
#include "lv_conf.h"
#include "lvgl/lvgl.h"
#endif

enum OSC_TRIGGER_MODE
{
    OSC_FREE_RUN = 0,
    OSC_POSITIVE_SLOPE = 1,
    OSC_NEGATIVE_SLOPE = 2,
};

enum OSC_DIVISION_LINE_TYPE
{
    OSC_NO_LINE = 0,
    OSC_SOLID_LINE = 1,
    OSC_DASHED_LINE = 2,
};

enum OSC_BORDER_SIDE_TYPE
{
    OSC_NO_SIDE = 0,
    OSC_AXIS_SIDE = 1,
    OSC_ANOTHER_SIDE = 2,
    OSC_BOTH_SIDE = 3,
};

enum OSC_BORDER_THICKNESS_TYPE
{
    OSC_NORMAL_BORDER = 0,
    OSC_THICKER_BORDER = 1,
    OSC_THICKEST_BORDER = 2,
};

namespace LVGL86 {
    class Oscilloscope {
    public:
        Oscilloscope(bool _realtime = true, bool _autoRefresh = true, unsigned long _sampleTime = 1000, unsigned long _bufSize = 250000, uint8_t _maxDec = 3);
        ~Oscilloscope();
        lv_obj_t* begin(lv_obj_t* parent = NULL, int _x = 0, int _y = 0, int _width = 390, int _height = 260, bool _enable = true,
            uint8_t _divLineV = 4, uint8_t _divLineH = 3, OSC_DIVISION_LINE_TYPE _divLineType = OSC_DASHED_LINE, OSC_BORDER_SIDE_TYPE _borderX = OSC_NO_SIDE, OSC_BORDER_SIDE_TYPE _borderY = OSC_NO_SIDE, OSC_BORDER_THICKNESS_TYPE _borderThickness = OSC_NORMAL_BORDER, bool _enableTickTextX = true, bool _enableTickTextY = true,
            uint8_t _interTickX = 0, uint8_t _interTickY = 0, uint8_t _tickSize = 1, double _channelColor = 0x01A2B1,
            unsigned long _timeDiv = 1000, double _min = -10, double _max = 10);

        bool refreshTimeUp();
        void refresh();
        void addSample(double sample); // if you want to use float, first call setDecimalPlaces() to set N decimal places.
        void addSampleInInterrupt(int Numerator, int denominator = 1);
        void clear(void);

        // Config fuctions must setup before begin()
        void configRealtimeMode(bool mode);
        void configAutoRefreshMode(bool mode);
        void configSampleingCycle(unsigned long val);
        void configSampleBufferSize(unsigned long size);
        void configDecimalPlaces(uint8_t n);

        // The functions affect the waveform display.
        void setTriggerMode(OSC_TRIGGER_MODE mode);
        void setTriggerLevel(double val);
        void setOffset(long microSec);
        void setTimeDiv(unsigned long microSec);
        void setYRange(double min, double max);
        void setFPS(unsigned int fps);
        void setDisplayQuality(unsigned int gap);

        // The functions affect the appearance of OSC
        void setXY(int x, int y);
        void setSize(int w, int h);
        void setEnable(bool enable);
        void setDivisionLines(uint8_t x = 4, uint8_t y = 3);
        void setDivisionLineType(OSC_DIVISION_LINE_TYPE type);
        void setBorderSide(OSC_BORDER_SIDE_TYPE typeX, OSC_BORDER_SIDE_TYPE typeY);
        void setBorderThickness(OSC_BORDER_THICKNESS_TYPE type);
        void setTickTextEnable(bool x, bool y);
        void setIntervalTicks(uint8_t x, uint8_t y);
        void setTickSize(uint8_t size);
        void setChannelColor(double color);
        void setLeftPadSize(int size);
        void setRightPadSize(int size);
        void setTopPadSize(int size);
        void setBottomPadSize(int size);
        void setHMITextFont(int fontId);
        void setTextFont(lv_font_t* font);

        // Configuration parameters
        bool getRealtimeMode() { return RealtimeMode; }
        bool getAutoRefreshMode() { return AutoRefreshMode; }
        int8_t getDecimalPlaces() { return DecimalPlaces; }
        unsigned long getSampleBufferSize() { return SampleBufferSize; }
        unsigned long getSampleingCycle() { return SampleingCycle; }

        // Waveform display parameters
        unsigned long getTimeDiv() { return TimeDiv; }
        double getMaxValue() { return (double)MaxValue / Magnification; }
        double getMinValue() { return (double)MinValue / Magnification; }
        unsigned int getFPS() { return FPS; }
        unsigned int getDisplayQuality() { return DisplayQuality; }
        OSC_TRIGGER_MODE getTriggerMode() { return TriggerMode; }
        double getTriggerLevel() { return TriggerLevel; }
        long getOffset() { return Offset; }

        // OSC appearance parameters
        int getX() { return X; }
        int getY() { return Y; }
        int getWidth() { return Width; }
        int getHeight() { return Height; }
        bool getEnable() { return Enable; }
        unsigned short getXDivisionLines() { return DivLineV; }
        unsigned short getYDivisionLines() { return DivLineH; }
        OSC_DIVISION_LINE_TYPE getDivisionLineType() { return DivLineType; }
        OSC_BORDER_SIDE_TYPE getXBorder() { return BorderX; }
        OSC_BORDER_SIDE_TYPE getYBorder() { return BorderY; }
        OSC_BORDER_THICKNESS_TYPE getBorderThickness() { return BorderThickness; }
        bool getTickTextEnableX() { return TickTextEnableX; }
        bool getTickTextEnableY() { return TickTextEnableY; }
        int8_t getIntervalTicksX() { return IntervalTicksX; }
        int8_t getIntervalTicksY() { return IntervalTicksY; }
        int8_t getTickSize() { return TickSize; }
        double getChannelColor();
        int getLeftPadSize() { return PadSizeL; }
        int getRightPadSize() { return PadSizeR; }
        int getTopPadSize() { return PadSizeT; }
        int getBottomPadSize() { return PadSizeB; }
        int getHMITextFont() { return get_font_id(obj); }
        lv_font_t* getTextFont() { return Font; }

        lv_obj_t* getObj() { return obj; }

        void sinWaveDemo(double offset = 0);
        lv_obj_user_data_t user_data;
    private:
        void refreshText();
        void insertSample(int val);
        void insertSample(int val, unsigned long long us);

        bool IsReady, RealtimeMode, AutoRefreshMode;
        uint8_t DecimalPlaces;
        unsigned long SampleBufferSize, SampleingCycle, Magnification;

        OSC_TRIGGER_MODE TriggerMode;
        int TriggerLevel;
        double Offset;
        unsigned long TimeDiv;
        int64_t MinValue, MaxValue;
        unsigned int FPS, DisplayQuality;

        int X, Y, Width, Height, PadSizeL, PadSizeR, PadSizeT, PadSizeB;
        bool Enable, TickTextEnableX, TickTextEnableY;
        uint8_t DivLineH, DivLineV, IntervalTicksX, IntervalTicksY, TickSize;
        OSC_DIVISION_LINE_TYPE DivLineType;
        OSC_BORDER_SIDE_TYPE BorderX, BorderY;
        OSC_BORDER_THICKNESS_TYPE BorderThickness;
        double ChannelColor;
        lv_font_t* Font;

        unsigned long long refreshTimer, rtTimer;
        uint8_t refreshFSM;
        int trigger_p;
        double prett;
        char* ticktextX, * ticktextY;

        int* sampleBuf, * intBuf;
        unsigned long long* intTimerBuf;
        volatile bool sBufFull;
        volatile int sBufPtr, intBufCnt;

        lv_obj_t* obj;
        lv_chart_series_t* cst;
    };

    extern Oscilloscope osc;
}
#endif
