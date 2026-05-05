#include <lv_qrcode.h>
#include <lv_freetype.h>
#include <lv_lodepng.h>
#include <lv_sjpg.h>
#include "myhmi.h"

/* User's global code */
/* Write your own global code in the following, allows you to define variables and functions that will be available in all code.For instance,you can use global code to define constants that will be used in local scripts you want to use repeatedly. */
#define EVENT_QUEUE_SIZE 256
#define ALARM_BANNER_BANK_SIZE 16
#define _ISOBJ(_id,_objsn) (ID_TO_SN(_id) / 1000 % 100 == _objsn)

MyHMI Hmi = *new MyHMI();

/* Struct declaration */
struct HMI_SlidePage_Data
{
	int8_t TimerMode;
	uint32_t TimerInterval;
	bool TimerEnable;
	char* Name;
	int _t;
	bool _en;
	int _igat;
};

struct HMI_BasicObj_Data
{
	char* Name;
};

struct HMI_Arc_Data
{
	char* Name;
	int8_t Decimal;
	double MaxValue;
	double MinValue;
	double Value;
	int8_t ShowValue;
	char* SuffixList[9];
	int8_t Background;
};

struct HMI_Bar_Data
{
	char* Name;
	int8_t Decimal;
	double MaxValue;
	double MinValue;
	double Value;
	int8_t ShowValue;
	char* PrefixList[9];
	char* SuffixList[9];
};

struct HMI_Button_Data
{
	char* Name;
	char* Symbol;
	char* TextList[9];
};

struct HMI_Chart_Data
{
	char* Name;
	uint32_t PointCount;
	int32_t MaxValue;
	int32_t MinValue;
	int8_t ChannelCount;
	double ChannelColor1;
	double ChannelColor2;
	double ChannelColor3;
	double ChannelColor4;
	int8_t TickX;
	int8_t TickY;
	int8_t TickSize;
	char* TickTextXList[9];
	char* TickTextYList[9];
};

struct HMI_Checkbox_Data
{
	char* Name;
	char* TextList[9];
};

struct HMI_DateInput_Data
{
	char* Name;
	int8_t DisplayType;
	uint16_t DefaultYear;
	uint16_t DefaultMonth;
	uint16_t DefaultDay;
};

struct HMI_DropDownList_Data
{
	char* Name;
	char* DropDownListText[9];
};

struct HMI_FileBrowser_Data
{
	char* Name;
	FileBrowser* _fb;
};

struct HMI_Gauge_Data
{
	char* Name;
	int32_t NeedleValue1;
	int32_t NeedleValue2;
	int32_t NeedleValue3;
	int32_t NeedleValue4;
	double NeedleColor1;
	double NeedleColor2;
	double NeedleColor3;
	double NeedleColor4;
};

struct HMI_Image_Data
{
	char* Name;
	char* ImageSource;
};

struct HMI_ImageBar_Data
{
	char* Name;
	double MaxValue;
	double MinValue;
	double Value;
	char* BgImageSource;
	char* BarImageSource;
	int32_t Pad;
};

struct HMI_ImageButton_Data
{
	char* Name;
	char* ImageSource;
};

struct HMI_ImageGauge_Data
{
	char* Name;
	int32_t NeedleValue1;
	char* NeedleImageSource;
};

struct HMI_ImageSlider_Data
{
	char* Name;
	char* BgImageSource;
	char* BarImageSource;
	char* KnobImageSource;
	int32_t Pad;
};

struct HMI_ImageToggleButton_Data
{
	char* Name;
	char* ImageSource;
	char* ImageChecked;
};

struct HMI_Label_Data
{
	char* Name;
	char* TextList[9];
	char* PrefixList[9];
	char* SuffixlList[9];
	char* _textlist[9];
};

struct HMI_List_Data
{
	char* Name;
	int8_t ItemCount;
	char* ListItems[144];
	char* Symbols[16];
};

struct HMI_Number_Data
{
	char* Name;
	char* PrefixList[9];
	char* SuffixlList[9];
	int8_t Decimal;
	double Value;
};

struct HMI_NumberInput_Data
{
	char* Name;
	int8_t ValueType;
};

struct HMI_Oscilloscope_Data
{
	char* Name;
	int8_t TickSize;
	uint32_t PointCount;
	uint32_t SamplingCycle;
	uint32_t TimeDiv;
	double ChannelColor1;
	int8_t TickX;
	int8_t TickY;
	double MaxValueDouble;
	double MinValueDouble;
	Oscilloscope* _osc;
};

struct HMI_Roller_Data
{
	char* Name;
	char* RollerText[9];
	int8_t RollerMode;
};

struct HMI_Slider_Data
{
	char* Name;
	int8_t ShowValue;
	char* PrefixList[9];
	char* SuffixList[9];
};

struct HMI_Spinbox_Data
{
	char* Name;
	int8_t Digit;
	int8_t Decimal;
};

struct HMI_Table_Data
{
	char* Name;
	int32_t ColWidth0;
	int32_t ColWidth1;
	int32_t ColWidth2;
	int32_t ColWidth3;
	int32_t ColWidth4;
	int32_t ColWidth5;
	int32_t ColWidth6;
	int32_t ColWidth7;
	int32_t ColWidth8;
	int32_t ColWidth9;
	int32_t ColWidth10;
	double RowHeaderBgColor;
	double RowHeaderTextColor;
	int8_t RowHeaderTextAlign;
	char* RowHeaderTextList[9];
	double ColHeaderBgColor;
	double ColHeaderTextColor;
	int8_t ColHeaderTextAlign;
	char* ColHeaderTextList[9];
	int32_t LineSpacingGap;
};

struct HMI_TextInput_Data
{
	char* Name;
	char _buffer[256];
};

struct HMI_TextList_Data
{
	char* Name;
	TextList* _tl;
};

struct HMI_ToggleButton_Data
{
	char* Name;
	char* Symbol;
	char* TextList[9];
	char* CheckedTextList[9];
};

struct EventNode
{
	lv_obj_t* obj;
	int eventId;
};

/* Global variable */
lv_obj_t *p1, *p1rect1, *p1l1, *p1b1, *p1b2, *p1rect2, *p1rect3, *p1rect4, *p1rect5, *p1rect6, *p1rect7, *p1l2, *p1l3, *p1l4, *p1l5, *p1l6, *p1l7, *p1l8, *p1l9, *p1num1, *p1num2, *p1rect8, *p1cir1, *p1qr1, *p1rect9, *p1rect10, *p1cir2, *p1cir3, *p1img1;
lv_obj_t *p2, *p2rect1, *p2l1, *p2b1, *p2b2, *p2rect2, *p2rect3, *p2rect4, *p2rect5, *p2l2, *p2l5, *p2l6, *p2l7, *p2b3, *p2tb1, *p2ib1, *p2itb1, *p2b4, *p2tb2, *p2ib2, *p2itb2, *p2b5, *p2b6, *p2tb3, *p2tb4, *p2itb3, *p2itb4, *p2ib3, *p2ib4;
lv_obj_t *p3, *p3rect1, *p3l1, *p3b1, *p3b2, *p3rect2, *p3rect3, *p3rect9, *p3l9, *p3rect5, *p3rect8, *p3l8, *p3l2, *p3l6, *p3l7, *p3sl2, *p3sl3, *p3isl1, *p3asl1, *p3asl2, *p3sw1, *p3sw2, *p3cb1, *p3cb2, *p3cb3, *p3cb4, *p3sw3, *p3sw4, *p3sw5, *p3sw6, *p3sw7, *p3sw8;
lv_obj_t *p4, *p4rect1, *p4l1, *p4b1, *p4b2, *p4rect2, *p4rect3, *p4rect5, *p4rect6, *p4rect7, *p4l2, *p4l3, *p4l4, *p4l6, *p4l7, *p4rect8, *p4l8, *p4ibar1, *p4bar2, *p4bar3, *p4ga1, *p4iga1, *p4led1, *p4led2, *p4led3, *p4led4, *p4abar1, *p4abar2;
lv_obj_t *p5, *p5rect1, *p5l1, *p5b1, *p5b2, *p5rect2, *p5rect3, *p5rect5, *p5rect8, *p5l2, *p5l6, *p5l7, *p5tbl1, *p5ch1, *p5l8;
lv_obj_t *p6, *p6rect1, *p6l1, *p6b1, *p6b2, *p6rect2, *p6l2, *p6rect3, *p6l3, *p6tin1, *p6nin1, *p6nin2, *p6rect4, *p6rect5, *p6l4, *p6l5, *p6tin2, *p6din1, *p6din2, *p6sp2, *p6sp4;
lv_obj_t *p7, *p7rect1, *p7l1, *p7b1, *p7b2, *p7rect2, *p7l2, *p7rect3, *p7l3, *p7rect4, *p7l4, *p7ddl1, *p7ddl2, *p7li1, *p7ro1, *p7ro2, *p7rect6, *p7l6;
TextList *p5tl1;
Oscilloscope *p5osc1;
FileBrowser *p7fb1;
static lv_obj_t* tileview;
static lv_obj_t* currentPage;
static lv_obj_t* kb;
static lv_obj_t* cal;
static lv_font_t* themeFonts[6];
static double themePalette[49] = {0x01A2B1, 0x44D1B6, 0xC83546, 0xDE426F, 0xFFC107, 0xFFC866, 0x2B84D3, 0x33A1FF, 0x01A2B1, 0x44D1B6, 0x01A2B1, 0x44D1B6, 0x01A2B1, 0x44D1B6, 0x01A2B1, 0x44D1B6, 0x01A2B1, 0x44D1B6, 0x01A2B1, 0x44D1B6, 0x01A2B1, 0x44D1B6, 0x01A2B1, 0x44D1B6, 0x444B5A, 0x586273, 0x494F57, 0xFFFFFF, 0xE7E9EC, 0xFFFFFF, 0xFFFFFF, 0x494F57, 0x5F656E, 0x999999, 0x5F656E, 0x586273, 0x888888, 0x888888, 0x41404F, 0xFFFFFF, 0xA1ADBD, 0xFFFFFF, 0x888888, -2, 0x999999, 0x777F85, 0xFFFFFF, 0xFFFFFF, 0x888888};
static int _LangID = 0;
static bool inUse = false;
static _HelperTask* _taskRTC = NULL;
static EventNode eventQueue[EVENT_QUEUE_SIZE];
static EventNode* popOutEvent = NULL;
static int eqHead = 0;
static int eqRail = 0;
static bool popUpBlock = false;
static lv_obj_t* popUpMsgBox = NULL;
static MBOX_WIDTH popUpMsgWidth = MBOX_NORMAL;
static lv_obj_t* ab = NULL;
static lv_obj_t* abLabel = NULL;
static char* abBank[ALARM_BANNER_BANK_SIZE] = { NULL };
static int abBankSpacing = 10;
static ABANNER_POS abPos = ABANNER_CENTER;

void widgetsInit();
void audioInit();
void audioLoad();
void pageTimerUpdate();
void oscUpdate();
void audioUpdate();
void hmi_task_handler();
EventNode* eventQueuePop();
bool eventQueuePush(EventNode e);
void alarmBannerRefreshText();
char* envPathCombine(char* env, char* path);

/* Widgets init declaration */
lv_obj_t* SlidePageInit(lv_obj_t* parent, char* _Wallpaper, HMI_SlidePage_Data* data);

void BasicObjInit(lv_obj_t* obj, lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, HMI_BasicObj_Data* data);

lv_obj_t* ArcInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                  uint16_t _AngleStart, uint16_t _AngleEnd, double _TextColor, int8_t _TextFont, double _BgColor, int8_t _BarThickness, HMI_Arc_Data* data);

lv_obj_t* ArcBarInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     uint16_t _AngleStart, uint16_t _AngleEnd, double _TextColor, int8_t _TextFont, double _BgColor, int8_t _BarThickness, HMI_Arc_Data* data);

void ArcBarValueChange(lv_obj_t* o, lv_event_t e);

void ArcBarRefreshText(lv_obj_t* o);

lv_obj_t* ArcSliderInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                        uint16_t _AngleStart, uint16_t _AngleEnd, double _TextColor, int8_t _TextFont, double _BgColor, int8_t _BarThickness, HMI_Arc_Data* data);

void ArcSliderRefreshText(lv_obj_t* o);

lv_obj_t* BarInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                  double _TextColor, int8_t _TextFont, HMI_Bar_Data* data);

void BarValueChange(lv_obj_t* o, lv_event_t e);

void BarRefreshText(lv_obj_t* o);

lv_obj_t* ButtonInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     int8_t _ButtonType, double _TextColor, int8_t _TextFont, HMI_Button_Data* data);

void ButtonRefreshText(lv_obj_t* o);

lv_obj_t* ChartInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                    int8_t _ChartType, int8_t _TextFont, uint16_t _DivLineV, uint16_t _DivLineH, int8_t _DivLineType, int8_t _BorderX, int8_t _BorderY, int8_t _BorderThickness, HMI_Chart_Data* data);

void ChartRefreshText(lv_obj_t* o);

lv_obj_t* CheckboxInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                       bool _Checked, double _TextColor, int8_t _TextFont, HMI_Checkbox_Data* data);

void CheckboxRefreshText(lv_obj_t* o);

lv_obj_t* CircleInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     double _BgColor, uint16_t _Opacity, HMI_BasicObj_Data* data);

lv_obj_t* DateInputInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                        int8_t _TextFont, HMI_DateInput_Data* data);

void DateInputRefreshText(lv_obj_t* o);

void cal_create(lv_obj_t* o);

void cal_event_cb(lv_obj_t* o, lv_event_t e);

lv_obj_t* DropDownListInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                           int8_t _TextFont, HMI_DropDownList_Data* data);

void DropDownListRefreshText(lv_obj_t* o);

FileBrowser* FileBrowserInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                             int8_t _Spacing, int8_t _TextFont, HMI_FileBrowser_Data* data);

lv_obj_t* GaugeInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                    int8_t _NeedleCount, int32_t _MaxValue, int32_t _MinValue, int8_t _NeedleThickness, int8_t _CenterPointSize, uint16_t _AngleStart, uint16_t _AngleEnd, int8_t _LabelCnt, int8_t _IntervalTicks, uint16_t _CriticalValue, bool _Background, HMI_Gauge_Data* data);

lv_obj_t* ImageInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                    uint16_t _Opacity, bool _Hidden, HMI_Image_Data* data);

lv_obj_t* ImageBarInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                       double _TextColor, int8_t _TextFont, HMI_ImageBar_Data* data);

lv_obj_t* ImageButtonInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                          int8_t _ButtonType, uint16_t _Opacity, HMI_ImageButton_Data* data);

lv_obj_t* ImageGaugeInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                         int8_t _NeedleCount, int32_t _MaxValue, int32_t _MinValue, int8_t _NeedleThickness, int8_t _CenterPointSize, uint16_t _AngleStart, uint16_t _AngleEnd, int8_t _LabelCnt, int8_t _IntervalTicks, uint16_t _CriticalValue, bool _Background, 
                         bool _NeedleMode, HMI_ImageGauge_Data* data);

lv_obj_t* ImageSliderInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                          int16_t _MaxValue, int16_t _MinValue, int16_t _Value, double _TextColor, int8_t _TextFont, HMI_ImageSlider_Data* data);

lv_obj_t* ImageToggleButtonInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                                int8_t _ButtonType, uint16_t _Opacity, 
                                bool _Checked, HMI_ImageToggleButton_Data* data);

lv_obj_t* LabelInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                    int8_t _TextAlign, double _TextColor, int8_t _TextFont, HMI_Label_Data* data);

void LabelRefreshText(lv_obj_t* o);

lv_obj_t* LEDInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                  uint16_t _Bright, HMI_BasicObj_Data* data);

lv_obj_t* ListInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                   int8_t _Spacing, int8_t _TextFont, HMI_List_Data* data);

void ListRefreshText(lv_obj_t* o);

void list_btn_event_handler(lv_obj_t* btn, lv_event_t e);

lv_obj_t* NumberInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     int8_t _TextAlign, double _TextColor, int8_t _TextFont, HMI_Number_Data* data);

void NumberRefreshText(lv_obj_t* o);

lv_obj_t* NumberInputInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                          int8_t _TextFont, HMI_NumberInput_Data* data);

Oscilloscope* OscilloscopeInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                               int8_t _OscMode, int8_t _TickTextXOption, int8_t _TickTextYOption, int8_t _TextFont, uint16_t _DivLineV, uint16_t _DivLineH, int8_t _LeftPadSize, int8_t _RightPadSize, int8_t _BottomPadSize, int8_t _TopPadSize, int8_t _RealtimeMode, int8_t _DisplayQuality, int8_t _DivLineType, int8_t _BorderX, int8_t _BorderY, int8_t _BorderThickness, HMI_Oscilloscope_Data* data);

lv_obj_t* QrCodeInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     char* _LinkSource, HMI_BasicObj_Data* data);

lv_obj_t* RectangleInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                        double _BgColor, uint16_t _Opacity, HMI_BasicObj_Data* data);

lv_obj_t* RollerInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     int8_t _TextFont, HMI_Roller_Data* data);

void RollerRefreshText(lv_obj_t* o);

lv_obj_t* SliderInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     int16_t _MaxValue, int16_t _MinValue, int16_t _Value, double _TextColor, int8_t _TextFont, HMI_Slider_Data* data);

void SliderRefreshText(lv_obj_t* o);

lv_obj_t* SpinboxInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                      int8_t _ButtonType, int8_t _NumericLimit, int8_t _TextFont, HMI_Spinbox_Data* data);

void lv_spinbox_event_cb(lv_obj_t* obj, lv_event_t e);

lv_obj_t* SwitchInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     bool _Checked, HMI_BasicObj_Data* data);

lv_obj_t* TableInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                    uint16_t _RowCnt, int8_t _BaseTextAlign, uint16_t _ColCnt, int8_t _Header, double _BaseBgColor, double _BaseTextColor, int8_t _BaseBorderSide, int8_t _TextFont, bool _Background, uint16_t _Opacity, HMI_Table_Data* data);

void TableRefreshText(lv_obj_t* o);

lv_obj_t* TextAreaInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                       int8_t _TextFont, HMI_BasicObj_Data* data);

void kb_create(lv_obj_t* o);

void kb_event_cb(lv_obj_t* keyboard, lv_event_t e);

lv_obj_t* TextInputInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                        int8_t _TextFont, HMI_TextInput_Data* data);

TextList* TextListInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                       int8_t _LineNumbers, int8_t _LineBorder, int8_t _LineSpacing, int8_t _TextIndent, int8_t _LineCount, double _NumColor, double _TextColor, int8_t _NumFont, int8_t _TextFont, double _OddLineColor, double _EvenLineColor, int8_t _FocusLineColor, HMI_TextList_Data* data);

lv_obj_t* ToggleButtonInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                           int8_t _ButtonType, double _TextColor, int8_t _TextFont, 
                           bool _Checked, HMI_ToggleButton_Data* data);

void ToggleButtonRefreshText(lv_obj_t* o);


/* Widgets event_handler declaration */
void tileviewEventCallback(lv_obj_t* o, lv_event_t e);
void p1EventCallback(lv_obj_t* o, lv_event_t e);
void p1b1EventCallback(lv_obj_t* o, lv_event_t e);
void p1b2EventCallback(lv_obj_t* o, lv_event_t e);
void p2EventCallback(lv_obj_t* o, lv_event_t e);
void p2b1EventCallback(lv_obj_t* o, lv_event_t e);
void p2b2EventCallback(lv_obj_t* o, lv_event_t e);
void p2b3EventCallback(lv_obj_t* o, lv_event_t e);
void p2tb1EventCallback(lv_obj_t* o, lv_event_t e);
void p2ib1EventCallback(lv_obj_t* o, lv_event_t e);
void p2itb1EventCallback(lv_obj_t* o, lv_event_t e);
void p2b4EventCallback(lv_obj_t* o, lv_event_t e);
void p2tb2EventCallback(lv_obj_t* o, lv_event_t e);
void p2ib2EventCallback(lv_obj_t* o, lv_event_t e);
void p2itb2EventCallback(lv_obj_t* o, lv_event_t e);
void p2b5EventCallback(lv_obj_t* o, lv_event_t e);
void p2b6EventCallback(lv_obj_t* o, lv_event_t e);
void p2tb3EventCallback(lv_obj_t* o, lv_event_t e);
void p2tb4EventCallback(lv_obj_t* o, lv_event_t e);
void p2itb3EventCallback(lv_obj_t* o, lv_event_t e);
void p2itb4EventCallback(lv_obj_t* o, lv_event_t e);
void p2ib3EventCallback(lv_obj_t* o, lv_event_t e);
void p2ib4EventCallback(lv_obj_t* o, lv_event_t e);
void p3EventCallback(lv_obj_t* o, lv_event_t e);
void p3b1EventCallback(lv_obj_t* o, lv_event_t e);
void p3b2EventCallback(lv_obj_t* o, lv_event_t e);
void p3sl2EventCallback(lv_obj_t* o, lv_event_t e);
void p3sl3EventCallback(lv_obj_t* o, lv_event_t e);
void p3isl1EventCallback(lv_obj_t* o, lv_event_t e);
void p3asl1EventCallback(lv_obj_t* o, lv_event_t e);
void p3asl2EventCallback(lv_obj_t* o, lv_event_t e);
void p3sw1EventCallback(lv_obj_t* o, lv_event_t e);
void p3sw2EventCallback(lv_obj_t* o, lv_event_t e);
void p3cb1EventCallback(lv_obj_t* o, lv_event_t e);
void p3cb2EventCallback(lv_obj_t* o, lv_event_t e);
void p3cb3EventCallback(lv_obj_t* o, lv_event_t e);
void p3cb4EventCallback(lv_obj_t* o, lv_event_t e);
void p3sw3EventCallback(lv_obj_t* o, lv_event_t e);
void p3sw4EventCallback(lv_obj_t* o, lv_event_t e);
void p3sw5EventCallback(lv_obj_t* o, lv_event_t e);
void p3sw6EventCallback(lv_obj_t* o, lv_event_t e);
void p3sw7EventCallback(lv_obj_t* o, lv_event_t e);
void p3sw8EventCallback(lv_obj_t* o, lv_event_t e);
void p4EventCallback(lv_obj_t* o, lv_event_t e);
void p4b1EventCallback(lv_obj_t* o, lv_event_t e);
void p4b2EventCallback(lv_obj_t* o, lv_event_t e);
void p5EventCallback(lv_obj_t* o, lv_event_t e);
void p5b1EventCallback(lv_obj_t* o, lv_event_t e);
void p5b2EventCallback(lv_obj_t* o, lv_event_t e);
void p6EventCallback(lv_obj_t* o, lv_event_t e);
void p6b1EventCallback(lv_obj_t* o, lv_event_t e);
void p6b2EventCallback(lv_obj_t* o, lv_event_t e);
void p6tin1EventCallback(lv_obj_t* o, lv_event_t e);
void p6nin1EventCallback(lv_obj_t* o, lv_event_t e);
void p6nin2EventCallback(lv_obj_t* o, lv_event_t e);
void p6tin2EventCallback(lv_obj_t* o, lv_event_t e);
void p6din1EventCallback(lv_obj_t* o, lv_event_t e);
void p6din2EventCallback(lv_obj_t* o, lv_event_t e);
void p7EventCallback(lv_obj_t* o, lv_event_t e);
void p7b1EventCallback(lv_obj_t* o, lv_event_t e);
void p7b2EventCallback(lv_obj_t* o, lv_event_t e);
void p7ddl1EventCallback(lv_obj_t* o, lv_event_t e);
void p7ddl2EventCallback(lv_obj_t* o, lv_event_t e);
void p7li1EventCallback(lv_obj_t* o, lv_event_t e);
void p7ro1EventCallback(lv_obj_t* o, lv_event_t e);
void p7ro2EventCallback(lv_obj_t* o, lv_event_t e);
void popUpMsgBoxEventCallback(lv_obj_t* o, lv_event_t e);

MyHMI::MyHMI()
{
}

void MyHMI::begin()
{
	if (inUse) return;
	lv86_init(0);
	char* buf = envPathCombine("IRSPATH", "FONT/ICON.TTF");
	lv_freetype_init(buf);
	free(buf);
	lv_split_jpeg_init();
	lv_lodepng_init();
	widgetsInit();
	audioInit();
	audioLoad();
	_taskRTC = myHelper.startLoop(100 * 1024);
	if (_taskRTC == NULL) return;
	myHelper.start(0);
	_taskRTC->registerFunc(hmi_task_handler);
	_taskRTC->start();
	inUse = true;
}

lv_obj_t* MyHMI::SN_TO_ID(int sn)
{
	switch(sn % 1000)
	{
		case 1:
		switch(sn / 1000 % 100)
		{
			case 0: return p1;
			case HMI_B:
				switch(sn / 100000)
				{
					case 1: return p1b1;
					case 2: return p1b2;
				}
			case HMI_CIR:
				switch(sn / 100000)
				{
					case 1: return p1cir1;
					case 2: return p1cir2;
					case 3: return p1cir3;
				}
			case HMI_IMG: return p1img1;
			case HMI_L:
				switch(sn / 100000)
				{
					case 1: return p1l1;
					case 2: return p1l2;
					case 3: return p1l3;
					case 4: return p1l4;
					case 5: return p1l5;
					case 6: return p1l6;
					case 7: return p1l7;
					case 8: return p1l8;
					case 9: return p1l9;
				}
			case HMI_NUM:
				switch(sn / 100000)
				{
					case 1: return p1num1;
					case 2: return p1num2;
				}
			case HMI_QR: return p1qr1;
			case HMI_RECT:
				switch(sn / 100000)
				{
					case 1: return p1rect1;
					case 2: return p1rect2;
					case 3: return p1rect3;
					case 4: return p1rect4;
					case 5: return p1rect5;
					case 6: return p1rect6;
					case 7: return p1rect7;
					case 8: return p1rect8;
					case 9: return p1rect9;
					case 10: return p1rect10;
				}
		}
		case 2:
		switch(sn / 1000 % 100)
		{
			case 0: return p2;
			case HMI_B:
				switch(sn / 100000)
				{
					case 1: return p2b1;
					case 2: return p2b2;
					case 3: return p2b3;
					case 4: return p2b4;
					case 5: return p2b5;
					case 6: return p2b6;
				}
			case HMI_IB:
				switch(sn / 100000)
				{
					case 1: return p2ib1;
					case 2: return p2ib2;
					case 3: return p2ib3;
					case 4: return p2ib4;
				}
			case HMI_ITB:
				switch(sn / 100000)
				{
					case 1: return p2itb1;
					case 2: return p2itb2;
					case 3: return p2itb3;
					case 4: return p2itb4;
				}
			case HMI_L:
				switch(sn / 100000)
				{
					case 1: return p2l1;
					case 2: return p2l2;
					case 5: return p2l5;
					case 6: return p2l6;
					case 7: return p2l7;
				}
			case HMI_RECT:
				switch(sn / 100000)
				{
					case 1: return p2rect1;
					case 2: return p2rect2;
					case 3: return p2rect3;
					case 4: return p2rect4;
					case 5: return p2rect5;
				}
			case HMI_TB:
				switch(sn / 100000)
				{
					case 1: return p2tb1;
					case 2: return p2tb2;
					case 3: return p2tb3;
					case 4: return p2tb4;
				}
		}
		case 3:
		switch(sn / 1000 % 100)
		{
			case 0: return p3;
			case HMI_ASL:
				switch(sn / 100000)
				{
					case 1: return p3asl1;
					case 2: return p3asl2;
				}
			case HMI_B:
				switch(sn / 100000)
				{
					case 1: return p3b1;
					case 2: return p3b2;
				}
			case HMI_CB:
				switch(sn / 100000)
				{
					case 1: return p3cb1;
					case 2: return p3cb2;
					case 3: return p3cb3;
					case 4: return p3cb4;
				}
			case HMI_ISL: return p3isl1;
			case HMI_L:
				switch(sn / 100000)
				{
					case 1: return p3l1;
					case 9: return p3l9;
					case 8: return p3l8;
					case 2: return p3l2;
					case 6: return p3l6;
					case 7: return p3l7;
				}
			case HMI_RECT:
				switch(sn / 100000)
				{
					case 1: return p3rect1;
					case 2: return p3rect2;
					case 3: return p3rect3;
					case 9: return p3rect9;
					case 5: return p3rect5;
					case 8: return p3rect8;
				}
			case HMI_SL:
				switch(sn / 100000)
				{
					case 2: return p3sl2;
					case 3: return p3sl3;
				}
			case HMI_SW:
				switch(sn / 100000)
				{
					case 1: return p3sw1;
					case 2: return p3sw2;
					case 3: return p3sw3;
					case 4: return p3sw4;
					case 5: return p3sw5;
					case 6: return p3sw6;
					case 7: return p3sw7;
					case 8: return p3sw8;
				}
		}
		case 4:
		switch(sn / 1000 % 100)
		{
			case 0: return p4;
			case HMI_ABAR:
				switch(sn / 100000)
				{
					case 1: return p4abar1;
					case 2: return p4abar2;
				}
			case HMI_BAR:
				switch(sn / 100000)
				{
					case 2: return p4bar2;
					case 3: return p4bar3;
				}
			case HMI_B:
				switch(sn / 100000)
				{
					case 1: return p4b1;
					case 2: return p4b2;
				}
			case HMI_GA: return p4ga1;
			case HMI_IBAR: return p4ibar1;
			case HMI_IGA: return p4iga1;
			case HMI_L:
				switch(sn / 100000)
				{
					case 1: return p4l1;
					case 2: return p4l2;
					case 3: return p4l3;
					case 4: return p4l4;
					case 6: return p4l6;
					case 7: return p4l7;
					case 8: return p4l8;
				}
			case HMI_LED:
				switch(sn / 100000)
				{
					case 1: return p4led1;
					case 2: return p4led2;
					case 3: return p4led3;
					case 4: return p4led4;
				}
			case HMI_RECT:
				switch(sn / 100000)
				{
					case 1: return p4rect1;
					case 2: return p4rect2;
					case 3: return p4rect3;
					case 5: return p4rect5;
					case 6: return p4rect6;
					case 7: return p4rect7;
					case 8: return p4rect8;
				}
		}
		case 5:
		switch(sn / 1000 % 100)
		{
			case 0: return p5;
			case HMI_B:
				switch(sn / 100000)
				{
					case 1: return p5b1;
					case 2: return p5b2;
				}
			case HMI_CH: return p5ch1;
			case HMI_L:
				switch(sn / 100000)
				{
					case 1: return p5l1;
					case 2: return p5l2;
					case 6: return p5l6;
					case 7: return p5l7;
					case 8: return p5l8;
				}
			case HMI_OSC: return p5osc1->getObj();
			case HMI_RECT:
				switch(sn / 100000)
				{
					case 1: return p5rect1;
					case 2: return p5rect2;
					case 3: return p5rect3;
					case 5: return p5rect5;
					case 8: return p5rect8;
				}
			case HMI_TBL: return p5tbl1;
			case HMI_TL: return p5tl1->getObj();
		}
		case 6:
		switch(sn / 1000 % 100)
		{
			case 0: return p6;
			case HMI_B:
				switch(sn / 100000)
				{
					case 1: return p6b1;
					case 2: return p6b2;
				}
			case HMI_DIN:
				switch(sn / 100000)
				{
					case 1: return p6din1;
					case 2: return p6din2;
				}
			case HMI_L:
				switch(sn / 100000)
				{
					case 1: return p6l1;
					case 2: return p6l2;
					case 3: return p6l3;
					case 4: return p6l4;
					case 5: return p6l5;
				}
			case HMI_NIN:
				switch(sn / 100000)
				{
					case 1: return p6nin1;
					case 2: return p6nin2;
				}
			case HMI_RECT:
				switch(sn / 100000)
				{
					case 1: return p6rect1;
					case 2: return p6rect2;
					case 3: return p6rect3;
					case 4: return p6rect4;
					case 5: return p6rect5;
				}
			case HMI_SP:
				switch(sn / 100000)
				{
					case 2: return p6sp2;
					case 4: return p6sp4;
				}
			case HMI_TIN:
				switch(sn / 100000)
				{
					case 1: return p6tin1;
					case 2: return p6tin2;
				}
		}
		case 7:
		switch(sn / 1000 % 100)
		{
			case 0: return p7;
			case HMI_B:
				switch(sn / 100000)
				{
					case 1: return p7b1;
					case 2: return p7b2;
				}
			case HMI_DDL:
				switch(sn / 100000)
				{
					case 1: return p7ddl1;
					case 2: return p7ddl2;
				}
			case HMI_FB: return p7fb1->getObj();
			case HMI_L:
				switch(sn / 100000)
				{
					case 1: return p7l1;
					case 2: return p7l2;
					case 3: return p7l3;
					case 4: return p7l4;
					case 6: return p7l6;
				}
			case HMI_LI: return p7li1;
			case HMI_RECT:
				switch(sn / 100000)
				{
					case 1: return p7rect1;
					case 2: return p7rect2;
					case 3: return p7rect3;
					case 4: return p7rect4;
					case 6: return p7rect6;
				}
			case HMI_RO:
				switch(sn / 100000)
				{
					case 1: return p7ro1;
					case 2: return p7ro2;
				}
		}
	}
	return NULL;
}

lv_obj_t* MyHMI::NAME_TO_ID(char* name)
{
	if (strcmp(name, ((HMI_SlidePage_Data*)lv_obj_get_user_data(p1))->Name) == 0) return p1;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect1))->Name) == 0) return p1rect1;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l1))->Name) == 0) return p1l1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p1b1))->Name) == 0) return p1b1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p1b2))->Name) == 0) return p1b2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect2))->Name) == 0) return p1rect2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect3))->Name) == 0) return p1rect3;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect4))->Name) == 0) return p1rect4;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect5))->Name) == 0) return p1rect5;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect6))->Name) == 0) return p1rect6;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect7))->Name) == 0) return p1rect7;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l2))->Name) == 0) return p1l2;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l3))->Name) == 0) return p1l3;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l4))->Name) == 0) return p1l4;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l5))->Name) == 0) return p1l5;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l6))->Name) == 0) return p1l6;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l7))->Name) == 0) return p1l7;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l8))->Name) == 0) return p1l8;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l9))->Name) == 0) return p1l9;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p1num1))->Name) == 0) return p1num1;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p1num2))->Name) == 0) return p1num2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect8))->Name) == 0) return p1rect8;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1cir1))->Name) == 0) return p1cir1;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_qrcode_get_private_data(p1qr1))->Name) == 0) return p1qr1;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect9))->Name) == 0) return p1rect9;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect10))->Name) == 0) return p1rect10;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1cir2))->Name) == 0) return p1cir2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1cir3))->Name) == 0) return p1cir3;
	if (strcmp(name, ((HMI_Image_Data*)lv_obj_get_user_data(p1img1))->Name) == 0) return p1img1;
	if (strcmp(name, ((HMI_SlidePage_Data*)lv_obj_get_user_data(p2))->Name) == 0) return p2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p2rect1))->Name) == 0) return p2rect1;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p2l1))->Name) == 0) return p2l1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p2b1))->Name) == 0) return p2b1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p2b2))->Name) == 0) return p2b2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p2rect2))->Name) == 0) return p2rect2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p2rect3))->Name) == 0) return p2rect3;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p2rect4))->Name) == 0) return p2rect4;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p2rect5))->Name) == 0) return p2rect5;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p2l2))->Name) == 0) return p2l2;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p2l5))->Name) == 0) return p2l5;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p2l6))->Name) == 0) return p2l6;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p2l7))->Name) == 0) return p2l7;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p2b3))->Name) == 0) return p2b3;
	if (strcmp(name, ((HMI_ToggleButton_Data*)lv_obj_get_user_data(p2tb1))->Name) == 0) return p2tb1;
	if (strcmp(name, ((HMI_ImageButton_Data*)lv_obj_get_user_data(p2ib1))->Name) == 0) return p2ib1;
	if (strcmp(name, ((HMI_ImageToggleButton_Data*)lv_obj_get_user_data(p2itb1))->Name) == 0) return p2itb1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p2b4))->Name) == 0) return p2b4;
	if (strcmp(name, ((HMI_ToggleButton_Data*)lv_obj_get_user_data(p2tb2))->Name) == 0) return p2tb2;
	if (strcmp(name, ((HMI_ImageButton_Data*)lv_obj_get_user_data(p2ib2))->Name) == 0) return p2ib2;
	if (strcmp(name, ((HMI_ImageToggleButton_Data*)lv_obj_get_user_data(p2itb2))->Name) == 0) return p2itb2;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p2b5))->Name) == 0) return p2b5;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p2b6))->Name) == 0) return p2b6;
	if (strcmp(name, ((HMI_ToggleButton_Data*)lv_obj_get_user_data(p2tb3))->Name) == 0) return p2tb3;
	if (strcmp(name, ((HMI_ToggleButton_Data*)lv_obj_get_user_data(p2tb4))->Name) == 0) return p2tb4;
	if (strcmp(name, ((HMI_ImageToggleButton_Data*)lv_obj_get_user_data(p2itb3))->Name) == 0) return p2itb3;
	if (strcmp(name, ((HMI_ImageToggleButton_Data*)lv_obj_get_user_data(p2itb4))->Name) == 0) return p2itb4;
	if (strcmp(name, ((HMI_ImageButton_Data*)lv_obj_get_user_data(p2ib3))->Name) == 0) return p2ib3;
	if (strcmp(name, ((HMI_ImageButton_Data*)lv_obj_get_user_data(p2ib4))->Name) == 0) return p2ib4;
	if (strcmp(name, ((HMI_SlidePage_Data*)lv_obj_get_user_data(p3))->Name) == 0) return p3;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3rect1))->Name) == 0) return p3rect1;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p3l1))->Name) == 0) return p3l1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p3b1))->Name) == 0) return p3b1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p3b2))->Name) == 0) return p3b2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3rect2))->Name) == 0) return p3rect2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3rect3))->Name) == 0) return p3rect3;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3rect9))->Name) == 0) return p3rect9;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p3l9))->Name) == 0) return p3l9;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3rect5))->Name) == 0) return p3rect5;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3rect8))->Name) == 0) return p3rect8;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p3l8))->Name) == 0) return p3l8;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p3l2))->Name) == 0) return p3l2;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p3l6))->Name) == 0) return p3l6;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p3l7))->Name) == 0) return p3l7;
	if (strcmp(name, ((HMI_Slider_Data*)lv_obj_get_user_data(p3sl2))->Name) == 0) return p3sl2;
	if (strcmp(name, ((HMI_Slider_Data*)lv_obj_get_user_data(p3sl3))->Name) == 0) return p3sl3;
	if (strcmp(name, ((HMI_ImageSlider_Data*)lv_obj_get_user_data(p3isl1))->Name) == 0) return p3isl1;
	if (strcmp(name, ((HMI_Arc_Data*)lv_obj_get_user_data(p3asl1))->Name) == 0) return p3asl1;
	if (strcmp(name, ((HMI_Arc_Data*)lv_obj_get_user_data(p3asl2))->Name) == 0) return p3asl2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw1))->Name) == 0) return p3sw1;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw2))->Name) == 0) return p3sw2;
	if (strcmp(name, ((HMI_Checkbox_Data*)lv_obj_get_user_data(p3cb1))->Name) == 0) return p3cb1;
	if (strcmp(name, ((HMI_Checkbox_Data*)lv_obj_get_user_data(p3cb2))->Name) == 0) return p3cb2;
	if (strcmp(name, ((HMI_Checkbox_Data*)lv_obj_get_user_data(p3cb3))->Name) == 0) return p3cb3;
	if (strcmp(name, ((HMI_Checkbox_Data*)lv_obj_get_user_data(p3cb4))->Name) == 0) return p3cb4;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw3))->Name) == 0) return p3sw3;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw4))->Name) == 0) return p3sw4;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw5))->Name) == 0) return p3sw5;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw6))->Name) == 0) return p3sw6;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw7))->Name) == 0) return p3sw7;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw8))->Name) == 0) return p3sw8;
	if (strcmp(name, ((HMI_SlidePage_Data*)lv_obj_get_user_data(p4))->Name) == 0) return p4;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect1))->Name) == 0) return p4rect1;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p4l1))->Name) == 0) return p4l1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p4b1))->Name) == 0) return p4b1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p4b2))->Name) == 0) return p4b2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect2))->Name) == 0) return p4rect2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect3))->Name) == 0) return p4rect3;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect5))->Name) == 0) return p4rect5;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect6))->Name) == 0) return p4rect6;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect7))->Name) == 0) return p4rect7;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p4l2))->Name) == 0) return p4l2;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p4l3))->Name) == 0) return p4l3;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p4l4))->Name) == 0) return p4l4;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p4l6))->Name) == 0) return p4l6;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p4l7))->Name) == 0) return p4l7;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect8))->Name) == 0) return p4rect8;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p4l8))->Name) == 0) return p4l8;
	if (strcmp(name, ((HMI_ImageBar_Data*)lv_obj_get_user_data(p4ibar1))->Name) == 0) return p4ibar1;
	if (strcmp(name, ((HMI_Bar_Data*)lv_obj_get_user_data(p4bar2))->Name) == 0) return p4bar2;
	if (strcmp(name, ((HMI_Bar_Data*)lv_obj_get_user_data(p4bar3))->Name) == 0) return p4bar3;
	if (strcmp(name, ((HMI_Gauge_Data*)lv_obj_get_user_data(p4ga1))->Name) == 0) return p4ga1;
	if (strcmp(name, ((HMI_ImageGauge_Data*)lv_obj_get_user_data(p4iga1))->Name) == 0) return p4iga1;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4led1))->Name) == 0) return p4led1;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4led2))->Name) == 0) return p4led2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4led3))->Name) == 0) return p4led3;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4led4))->Name) == 0) return p4led4;
	if (strcmp(name, ((HMI_Arc_Data*)lv_obj_get_user_data(p4abar1))->Name) == 0) return p4abar1;
	if (strcmp(name, ((HMI_Arc_Data*)lv_obj_get_user_data(p4abar2))->Name) == 0) return p4abar2;
	if (strcmp(name, ((HMI_SlidePage_Data*)lv_obj_get_user_data(p5))->Name) == 0) return p5;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p5rect1))->Name) == 0) return p5rect1;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p5l1))->Name) == 0) return p5l1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p5b1))->Name) == 0) return p5b1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p5b2))->Name) == 0) return p5b2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p5rect2))->Name) == 0) return p5rect2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p5rect3))->Name) == 0) return p5rect3;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p5rect5))->Name) == 0) return p5rect5;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p5rect8))->Name) == 0) return p5rect8;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p5l2))->Name) == 0) return p5l2;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p5l6))->Name) == 0) return p5l6;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p5l7))->Name) == 0) return p5l7;
	if (strcmp(name, ((HMI_TextList_Data*)lv_obj_get_user_data(p5tl1->getObj()))->Name) == 0) return p5tl1->getObj();
	if (strcmp(name, ((HMI_Table_Data*)lv_obj_get_user_data(p5tbl1))->Name) == 0) return p5tbl1;
	if (strcmp(name, ((HMI_Chart_Data*)lv_obj_get_user_data(p5ch1))->Name) == 0) return p5ch1;
	if (strcmp(name, ((HMI_Oscilloscope_Data*)lv_obj_get_user_data(p5osc1->getObj()))->Name) == 0) return p5osc1->getObj();
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p5l8))->Name) == 0) return p5l8;
	if (strcmp(name, ((HMI_SlidePage_Data*)lv_obj_get_user_data(p6))->Name) == 0) return p6;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect1))->Name) == 0) return p6rect1;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l1))->Name) == 0) return p6l1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p6b1))->Name) == 0) return p6b1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p6b2))->Name) == 0) return p6b2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect2))->Name) == 0) return p6rect2;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l2))->Name) == 0) return p6l2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect3))->Name) == 0) return p6rect3;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l3))->Name) == 0) return p6l3;
	if (strcmp(name, ((HMI_TextInput_Data*)lv_obj_get_user_data(p6tin1))->Name) == 0) return p6tin1;
	if (strcmp(name, ((HMI_NumberInput_Data*)lv_obj_get_user_data(p6nin1))->Name) == 0) return p6nin1;
	if (strcmp(name, ((HMI_NumberInput_Data*)lv_obj_get_user_data(p6nin2))->Name) == 0) return p6nin2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect4))->Name) == 0) return p6rect4;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect5))->Name) == 0) return p6rect5;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l4))->Name) == 0) return p6l4;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l5))->Name) == 0) return p6l5;
	if (strcmp(name, ((HMI_TextInput_Data*)lv_obj_get_user_data(p6tin2))->Name) == 0) return p6tin2;
	if (strcmp(name, ((HMI_DateInput_Data*)lv_obj_get_user_data(p6din1))->Name) == 0) return p6din1;
	if (strcmp(name, ((HMI_DateInput_Data*)lv_obj_get_user_data(p6din2))->Name) == 0) return p6din2;
	if (strcmp(name, ((HMI_Spinbox_Data*)lv_obj_get_user_data(p6sp2))->Name) == 0) return p6sp2;
	if (strcmp(name, ((HMI_Spinbox_Data*)lv_obj_get_user_data(p6sp4))->Name) == 0) return p6sp4;
	if (strcmp(name, ((HMI_SlidePage_Data*)lv_obj_get_user_data(p7))->Name) == 0) return p7;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p7rect1))->Name) == 0) return p7rect1;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p7l1))->Name) == 0) return p7l1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p7b1))->Name) == 0) return p7b1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p7b2))->Name) == 0) return p7b2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p7rect2))->Name) == 0) return p7rect2;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p7l2))->Name) == 0) return p7l2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p7rect3))->Name) == 0) return p7rect3;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p7l3))->Name) == 0) return p7l3;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p7rect4))->Name) == 0) return p7rect4;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p7l4))->Name) == 0) return p7l4;
	if (strcmp(name, ((HMI_DropDownList_Data*)lv_obj_get_user_data(p7ddl1))->Name) == 0) return p7ddl1;
	if (strcmp(name, ((HMI_DropDownList_Data*)lv_obj_get_user_data(p7ddl2))->Name) == 0) return p7ddl2;
	if (strcmp(name, ((HMI_List_Data*)lv_obj_get_user_data(p7li1))->Name) == 0) return p7li1;
	if (strcmp(name, ((HMI_Roller_Data*)lv_obj_get_user_data(p7ro1))->Name) == 0) return p7ro1;
	if (strcmp(name, ((HMI_Roller_Data*)lv_obj_get_user_data(p7ro2))->Name) == 0) return p7ro2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p7rect6))->Name) == 0) return p7rect6;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p7l6))->Name) == 0) return p7l6;
	if (strcmp(name, ((HMI_FileBrowser_Data*)get_list_user_data(p7fb1->getObj()))->Name) == 0) return p7fb1->getObj();
	return NULL;
}

int MyHMI::ID_TO_SN(lv_obj_t* id)
{
	if (id == p1) return 1;
	if (id == p2) return 2;
	if (id == p3) return 3;
	if (id == p4) return 4;
	if (id == p5) return 5;
	if (id == p6) return 6;
	if (id == p7) return 7;
	if (id == p1rect1) return 143001;
	if (id == p1l1) return 119001;
	if (id == p1b1) return 105001;
	if (id == p1b2) return 205001;
	if (id == p1rect2) return 243001;
	if (id == p1rect3) return 343001;
	if (id == p1rect4) return 443001;
	if (id == p1rect5) return 543001;
	if (id == p1rect6) return 643001;
	if (id == p1rect7) return 743001;
	if (id == p1l2) return 219001;
	if (id == p1l3) return 319001;
	if (id == p1l4) return 419001;
	if (id == p1l5) return 519001;
	if (id == p1l6) return 619001;
	if (id == p1l7) return 719001;
	if (id == p1l8) return 819001;
	if (id == p1l9) return 919001;
	if (id == p1num1) return 120001;
	if (id == p1num2) return 220001;
	if (id == p1rect8) return 843001;
	if (id == p1cir1) return 146001;
	if (id == p1qr1) return 140001;
	if (id == p1rect9) return 943001;
	if (id == p1rect10) return 1043001;
	if (id == p1cir2) return 246001;
	if (id == p1cir3) return 346001;
	if (id == p1img1) return 116001;
	if (id == p2rect1) return 143002;
	if (id == p2l1) return 119002;
	if (id == p2b1) return 105002;
	if (id == p2b2) return 205002;
	if (id == p2rect2) return 243002;
	if (id == p2rect3) return 343002;
	if (id == p2rect4) return 443002;
	if (id == p2rect5) return 543002;
	if (id == p2l2) return 219002;
	if (id == p2l5) return 519002;
	if (id == p2l6) return 619002;
	if (id == p2l7) return 719002;
	if (id == p2b3) return 305002;
	if (id == p2tb1) return 106002;
	if (id == p2ib1) return 117002;
	if (id == p2itb1) return 141002;
	if (id == p2b4) return 405002;
	if (id == p2tb2) return 206002;
	if (id == p2ib2) return 217002;
	if (id == p2itb2) return 241002;
	if (id == p2b5) return 505002;
	if (id == p2b6) return 605002;
	if (id == p2tb3) return 306002;
	if (id == p2tb4) return 406002;
	if (id == p2itb3) return 341002;
	if (id == p2itb4) return 441002;
	if (id == p2ib3) return 317002;
	if (id == p2ib4) return 417002;
	if (id == p3rect1) return 143003;
	if (id == p3l1) return 119003;
	if (id == p3b1) return 105003;
	if (id == p3b2) return 205003;
	if (id == p3rect2) return 243003;
	if (id == p3rect3) return 343003;
	if (id == p3rect9) return 943003;
	if (id == p3l9) return 919003;
	if (id == p3rect5) return 543003;
	if (id == p3rect8) return 843003;
	if (id == p3l8) return 819003;
	if (id == p3l2) return 219003;
	if (id == p3l6) return 619003;
	if (id == p3l7) return 719003;
	if (id == p3sl2) return 230003;
	if (id == p3sl3) return 330003;
	if (id == p3isl1) return 148003;
	if (id == p3asl1) return 103003;
	if (id == p3asl2) return 203003;
	if (id == p3sw1) return 133003;
	if (id == p3sw2) return 233003;
	if (id == p3cb1) return 110003;
	if (id == p3cb2) return 210003;
	if (id == p3cb3) return 310003;
	if (id == p3cb4) return 410003;
	if (id == p3sw3) return 333003;
	if (id == p3sw4) return 433003;
	if (id == p3sw5) return 533003;
	if (id == p3sw6) return 633003;
	if (id == p3sw7) return 733003;
	if (id == p3sw8) return 833003;
	if (id == p4rect1) return 143004;
	if (id == p4l1) return 119004;
	if (id == p4b1) return 105004;
	if (id == p4b2) return 205004;
	if (id == p4rect2) return 243004;
	if (id == p4rect3) return 343004;
	if (id == p4rect5) return 543004;
	if (id == p4rect6) return 643004;
	if (id == p4rect7) return 743004;
	if (id == p4l2) return 219004;
	if (id == p4l3) return 319004;
	if (id == p4l4) return 419004;
	if (id == p4l6) return 619004;
	if (id == p4l7) return 719004;
	if (id == p4rect8) return 843004;
	if (id == p4l8) return 819004;
	if (id == p4ibar1) return 147004;
	if (id == p4bar2) return 204004;
	if (id == p4bar3) return 304004;
	if (id == p4ga1) return 115004;
	if (id == p4iga1) return 149004;
	if (id == p4led1) return 121004;
	if (id == p4led2) return 221004;
	if (id == p4led3) return 321004;
	if (id == p4led4) return 421004;
	if (id == p4abar1) return 102004;
	if (id == p4abar2) return 202004;
	if (id == p5rect1) return 143005;
	if (id == p5l1) return 119005;
	if (id == p5b1) return 105005;
	if (id == p5b2) return 205005;
	if (id == p5rect2) return 243005;
	if (id == p5rect3) return 343005;
	if (id == p5rect5) return 543005;
	if (id == p5rect8) return 843005;
	if (id == p5l2) return 219005;
	if (id == p5l6) return 619005;
	if (id == p5l7) return 719005;
	if (id == p5tl1->getObj()) return 151005;
	if (id == p5tbl1) return 134005;
	if (id == p5ch1) return 111005;
	if (id == p5osc1->getObj()) return 142005;
	if (id == p5l8) return 819005;
	if (id == p6rect1) return 143006;
	if (id == p6l1) return 119006;
	if (id == p6b1) return 105006;
	if (id == p6b2) return 205006;
	if (id == p6rect2) return 243006;
	if (id == p6l2) return 219006;
	if (id == p6rect3) return 343006;
	if (id == p6l3) return 319006;
	if (id == p6tin1) return 137006;
	if (id == p6nin1) return 144006;
	if (id == p6nin2) return 244006;
	if (id == p6rect4) return 443006;
	if (id == p6rect5) return 543006;
	if (id == p6l4) return 419006;
	if (id == p6l5) return 519006;
	if (id == p6tin2) return 237006;
	if (id == p6din1) return 145006;
	if (id == p6din2) return 245006;
	if (id == p6sp2) return 231006;
	if (id == p6sp4) return 431006;
	if (id == p7rect1) return 143007;
	if (id == p7l1) return 119007;
	if (id == p7b1) return 105007;
	if (id == p7b2) return 205007;
	if (id == p7rect2) return 243007;
	if (id == p7l2) return 219007;
	if (id == p7rect3) return 343007;
	if (id == p7l3) return 319007;
	if (id == p7rect4) return 443007;
	if (id == p7l4) return 419007;
	if (id == p7ddl1) return 114007;
	if (id == p7ddl2) return 214007;
	if (id == p7li1) return 123007;
	if (id == p7ro1) return 129007;
	if (id == p7ro2) return 229007;
	if (id == p7rect6) return 643007;
	if (id == p7l6) return 619007;
	if (id == p7fb1->getObj()) return 150007;
	return 0;
}

char* MyHMI::ID_TO_NAME(lv_obj_t* id)
{
	if (id == p1) return ((HMI_SlidePage_Data*)lv_obj_get_user_data(p1))->Name;
	if (id == p1rect1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect1))->Name;
	if (id == p1l1) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l1))->Name;
	if (id == p1b1) return ((HMI_Button_Data*)lv_obj_get_user_data(p1b1))->Name;
	if (id == p1b2) return ((HMI_Button_Data*)lv_obj_get_user_data(p1b2))->Name;
	if (id == p1rect2) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect2))->Name;
	if (id == p1rect3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect3))->Name;
	if (id == p1rect4) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect4))->Name;
	if (id == p1rect5) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect5))->Name;
	if (id == p1rect6) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect6))->Name;
	if (id == p1rect7) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect7))->Name;
	if (id == p1l2) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l2))->Name;
	if (id == p1l3) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l3))->Name;
	if (id == p1l4) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l4))->Name;
	if (id == p1l5) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l5))->Name;
	if (id == p1l6) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l6))->Name;
	if (id == p1l7) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l7))->Name;
	if (id == p1l8) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l8))->Name;
	if (id == p1l9) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l9))->Name;
	if (id == p1num1) return ((HMI_Number_Data*)lv_obj_get_user_data(p1num1))->Name;
	if (id == p1num2) return ((HMI_Number_Data*)lv_obj_get_user_data(p1num2))->Name;
	if (id == p1rect8) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect8))->Name;
	if (id == p1cir1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1cir1))->Name;
	if (id == p1qr1) return ((HMI_BasicObj_Data*)lv_qrcode_get_private_data(p1qr1))->Name;
	if (id == p1rect9) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect9))->Name;
	if (id == p1rect10) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect10))->Name;
	if (id == p1cir2) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1cir2))->Name;
	if (id == p1cir3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1cir3))->Name;
	if (id == p1img1) return ((HMI_Image_Data*)lv_obj_get_user_data(p1img1))->Name;
	if (id == p2) return ((HMI_SlidePage_Data*)lv_obj_get_user_data(p2))->Name;
	if (id == p2rect1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p2rect1))->Name;
	if (id == p2l1) return ((HMI_Label_Data*)lv_obj_get_user_data(p2l1))->Name;
	if (id == p2b1) return ((HMI_Button_Data*)lv_obj_get_user_data(p2b1))->Name;
	if (id == p2b2) return ((HMI_Button_Data*)lv_obj_get_user_data(p2b2))->Name;
	if (id == p2rect2) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p2rect2))->Name;
	if (id == p2rect3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p2rect3))->Name;
	if (id == p2rect4) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p2rect4))->Name;
	if (id == p2rect5) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p2rect5))->Name;
	if (id == p2l2) return ((HMI_Label_Data*)lv_obj_get_user_data(p2l2))->Name;
	if (id == p2l5) return ((HMI_Label_Data*)lv_obj_get_user_data(p2l5))->Name;
	if (id == p2l6) return ((HMI_Label_Data*)lv_obj_get_user_data(p2l6))->Name;
	if (id == p2l7) return ((HMI_Label_Data*)lv_obj_get_user_data(p2l7))->Name;
	if (id == p2b3) return ((HMI_Button_Data*)lv_obj_get_user_data(p2b3))->Name;
	if (id == p2tb1) return ((HMI_ToggleButton_Data*)lv_obj_get_user_data(p2tb1))->Name;
	if (id == p2ib1) return ((HMI_ImageButton_Data*)lv_obj_get_user_data(p2ib1))->Name;
	if (id == p2itb1) return ((HMI_ImageToggleButton_Data*)lv_obj_get_user_data(p2itb1))->Name;
	if (id == p2b4) return ((HMI_Button_Data*)lv_obj_get_user_data(p2b4))->Name;
	if (id == p2tb2) return ((HMI_ToggleButton_Data*)lv_obj_get_user_data(p2tb2))->Name;
	if (id == p2ib2) return ((HMI_ImageButton_Data*)lv_obj_get_user_data(p2ib2))->Name;
	if (id == p2itb2) return ((HMI_ImageToggleButton_Data*)lv_obj_get_user_data(p2itb2))->Name;
	if (id == p2b5) return ((HMI_Button_Data*)lv_obj_get_user_data(p2b5))->Name;
	if (id == p2b6) return ((HMI_Button_Data*)lv_obj_get_user_data(p2b6))->Name;
	if (id == p2tb3) return ((HMI_ToggleButton_Data*)lv_obj_get_user_data(p2tb3))->Name;
	if (id == p2tb4) return ((HMI_ToggleButton_Data*)lv_obj_get_user_data(p2tb4))->Name;
	if (id == p2itb3) return ((HMI_ImageToggleButton_Data*)lv_obj_get_user_data(p2itb3))->Name;
	if (id == p2itb4) return ((HMI_ImageToggleButton_Data*)lv_obj_get_user_data(p2itb4))->Name;
	if (id == p2ib3) return ((HMI_ImageButton_Data*)lv_obj_get_user_data(p2ib3))->Name;
	if (id == p2ib4) return ((HMI_ImageButton_Data*)lv_obj_get_user_data(p2ib4))->Name;
	if (id == p3) return ((HMI_SlidePage_Data*)lv_obj_get_user_data(p3))->Name;
	if (id == p3rect1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3rect1))->Name;
	if (id == p3l1) return ((HMI_Label_Data*)lv_obj_get_user_data(p3l1))->Name;
	if (id == p3b1) return ((HMI_Button_Data*)lv_obj_get_user_data(p3b1))->Name;
	if (id == p3b2) return ((HMI_Button_Data*)lv_obj_get_user_data(p3b2))->Name;
	if (id == p3rect2) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3rect2))->Name;
	if (id == p3rect3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3rect3))->Name;
	if (id == p3rect9) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3rect9))->Name;
	if (id == p3l9) return ((HMI_Label_Data*)lv_obj_get_user_data(p3l9))->Name;
	if (id == p3rect5) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3rect5))->Name;
	if (id == p3rect8) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3rect8))->Name;
	if (id == p3l8) return ((HMI_Label_Data*)lv_obj_get_user_data(p3l8))->Name;
	if (id == p3l2) return ((HMI_Label_Data*)lv_obj_get_user_data(p3l2))->Name;
	if (id == p3l6) return ((HMI_Label_Data*)lv_obj_get_user_data(p3l6))->Name;
	if (id == p3l7) return ((HMI_Label_Data*)lv_obj_get_user_data(p3l7))->Name;
	if (id == p3sl2) return ((HMI_Slider_Data*)lv_obj_get_user_data(p3sl2))->Name;
	if (id == p3sl3) return ((HMI_Slider_Data*)lv_obj_get_user_data(p3sl3))->Name;
	if (id == p3isl1) return ((HMI_ImageSlider_Data*)lv_obj_get_user_data(p3isl1))->Name;
	if (id == p3asl1) return ((HMI_Arc_Data*)lv_obj_get_user_data(p3asl1))->Name;
	if (id == p3asl2) return ((HMI_Arc_Data*)lv_obj_get_user_data(p3asl2))->Name;
	if (id == p3sw1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw1))->Name;
	if (id == p3sw2) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw2))->Name;
	if (id == p3cb1) return ((HMI_Checkbox_Data*)lv_obj_get_user_data(p3cb1))->Name;
	if (id == p3cb2) return ((HMI_Checkbox_Data*)lv_obj_get_user_data(p3cb2))->Name;
	if (id == p3cb3) return ((HMI_Checkbox_Data*)lv_obj_get_user_data(p3cb3))->Name;
	if (id == p3cb4) return ((HMI_Checkbox_Data*)lv_obj_get_user_data(p3cb4))->Name;
	if (id == p3sw3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw3))->Name;
	if (id == p3sw4) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw4))->Name;
	if (id == p3sw5) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw5))->Name;
	if (id == p3sw6) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw6))->Name;
	if (id == p3sw7) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw7))->Name;
	if (id == p3sw8) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p3sw8))->Name;
	if (id == p4) return ((HMI_SlidePage_Data*)lv_obj_get_user_data(p4))->Name;
	if (id == p4rect1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect1))->Name;
	if (id == p4l1) return ((HMI_Label_Data*)lv_obj_get_user_data(p4l1))->Name;
	if (id == p4b1) return ((HMI_Button_Data*)lv_obj_get_user_data(p4b1))->Name;
	if (id == p4b2) return ((HMI_Button_Data*)lv_obj_get_user_data(p4b2))->Name;
	if (id == p4rect2) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect2))->Name;
	if (id == p4rect3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect3))->Name;
	if (id == p4rect5) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect5))->Name;
	if (id == p4rect6) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect6))->Name;
	if (id == p4rect7) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect7))->Name;
	if (id == p4l2) return ((HMI_Label_Data*)lv_obj_get_user_data(p4l2))->Name;
	if (id == p4l3) return ((HMI_Label_Data*)lv_obj_get_user_data(p4l3))->Name;
	if (id == p4l4) return ((HMI_Label_Data*)lv_obj_get_user_data(p4l4))->Name;
	if (id == p4l6) return ((HMI_Label_Data*)lv_obj_get_user_data(p4l6))->Name;
	if (id == p4l7) return ((HMI_Label_Data*)lv_obj_get_user_data(p4l7))->Name;
	if (id == p4rect8) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect8))->Name;
	if (id == p4l8) return ((HMI_Label_Data*)lv_obj_get_user_data(p4l8))->Name;
	if (id == p4ibar1) return ((HMI_ImageBar_Data*)lv_obj_get_user_data(p4ibar1))->Name;
	if (id == p4bar2) return ((HMI_Bar_Data*)lv_obj_get_user_data(p4bar2))->Name;
	if (id == p4bar3) return ((HMI_Bar_Data*)lv_obj_get_user_data(p4bar3))->Name;
	if (id == p4ga1) return ((HMI_Gauge_Data*)lv_obj_get_user_data(p4ga1))->Name;
	if (id == p4iga1) return ((HMI_ImageGauge_Data*)lv_obj_get_user_data(p4iga1))->Name;
	if (id == p4led1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4led1))->Name;
	if (id == p4led2) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4led2))->Name;
	if (id == p4led3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4led3))->Name;
	if (id == p4led4) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4led4))->Name;
	if (id == p4abar1) return ((HMI_Arc_Data*)lv_obj_get_user_data(p4abar1))->Name;
	if (id == p4abar2) return ((HMI_Arc_Data*)lv_obj_get_user_data(p4abar2))->Name;
	if (id == p5) return ((HMI_SlidePage_Data*)lv_obj_get_user_data(p5))->Name;
	if (id == p5rect1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p5rect1))->Name;
	if (id == p5l1) return ((HMI_Label_Data*)lv_obj_get_user_data(p5l1))->Name;
	if (id == p5b1) return ((HMI_Button_Data*)lv_obj_get_user_data(p5b1))->Name;
	if (id == p5b2) return ((HMI_Button_Data*)lv_obj_get_user_data(p5b2))->Name;
	if (id == p5rect2) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p5rect2))->Name;
	if (id == p5rect3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p5rect3))->Name;
	if (id == p5rect5) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p5rect5))->Name;
	if (id == p5rect8) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p5rect8))->Name;
	if (id == p5l2) return ((HMI_Label_Data*)lv_obj_get_user_data(p5l2))->Name;
	if (id == p5l6) return ((HMI_Label_Data*)lv_obj_get_user_data(p5l6))->Name;
	if (id == p5l7) return ((HMI_Label_Data*)lv_obj_get_user_data(p5l7))->Name;
	if (id == p5tl1->getObj()) return ((HMI_TextList_Data*)lv_obj_get_user_data(p5tl1->getObj()))->Name;
	if (id == p5tbl1) return ((HMI_Table_Data*)lv_obj_get_user_data(p5tbl1))->Name;
	if (id == p5ch1) return ((HMI_Chart_Data*)lv_obj_get_user_data(p5ch1))->Name;
	if (id == p5osc1->getObj()) return ((HMI_Oscilloscope_Data*)lv_obj_get_user_data(p5osc1->getObj()))->Name;
	if (id == p5l8) return ((HMI_Label_Data*)lv_obj_get_user_data(p5l8))->Name;
	if (id == p6) return ((HMI_SlidePage_Data*)lv_obj_get_user_data(p6))->Name;
	if (id == p6rect1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect1))->Name;
	if (id == p6l1) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l1))->Name;
	if (id == p6b1) return ((HMI_Button_Data*)lv_obj_get_user_data(p6b1))->Name;
	if (id == p6b2) return ((HMI_Button_Data*)lv_obj_get_user_data(p6b2))->Name;
	if (id == p6rect2) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect2))->Name;
	if (id == p6l2) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l2))->Name;
	if (id == p6rect3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect3))->Name;
	if (id == p6l3) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l3))->Name;
	if (id == p6tin1) return ((HMI_TextInput_Data*)lv_obj_get_user_data(p6tin1))->Name;
	if (id == p6nin1) return ((HMI_NumberInput_Data*)lv_obj_get_user_data(p6nin1))->Name;
	if (id == p6nin2) return ((HMI_NumberInput_Data*)lv_obj_get_user_data(p6nin2))->Name;
	if (id == p6rect4) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect4))->Name;
	if (id == p6rect5) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect5))->Name;
	if (id == p6l4) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l4))->Name;
	if (id == p6l5) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l5))->Name;
	if (id == p6tin2) return ((HMI_TextInput_Data*)lv_obj_get_user_data(p6tin2))->Name;
	if (id == p6din1) return ((HMI_DateInput_Data*)lv_obj_get_user_data(p6din1))->Name;
	if (id == p6din2) return ((HMI_DateInput_Data*)lv_obj_get_user_data(p6din2))->Name;
	if (id == p6sp2) return ((HMI_Spinbox_Data*)lv_obj_get_user_data(p6sp2))->Name;
	if (id == p6sp4) return ((HMI_Spinbox_Data*)lv_obj_get_user_data(p6sp4))->Name;
	if (id == p7) return ((HMI_SlidePage_Data*)lv_obj_get_user_data(p7))->Name;
	if (id == p7rect1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p7rect1))->Name;
	if (id == p7l1) return ((HMI_Label_Data*)lv_obj_get_user_data(p7l1))->Name;
	if (id == p7b1) return ((HMI_Button_Data*)lv_obj_get_user_data(p7b1))->Name;
	if (id == p7b2) return ((HMI_Button_Data*)lv_obj_get_user_data(p7b2))->Name;
	if (id == p7rect2) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p7rect2))->Name;
	if (id == p7l2) return ((HMI_Label_Data*)lv_obj_get_user_data(p7l2))->Name;
	if (id == p7rect3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p7rect3))->Name;
	if (id == p7l3) return ((HMI_Label_Data*)lv_obj_get_user_data(p7l3))->Name;
	if (id == p7rect4) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p7rect4))->Name;
	if (id == p7l4) return ((HMI_Label_Data*)lv_obj_get_user_data(p7l4))->Name;
	if (id == p7ddl1) return ((HMI_DropDownList_Data*)lv_obj_get_user_data(p7ddl1))->Name;
	if (id == p7ddl2) return ((HMI_DropDownList_Data*)lv_obj_get_user_data(p7ddl2))->Name;
	if (id == p7li1) return ((HMI_List_Data*)lv_obj_get_user_data(p7li1))->Name;
	if (id == p7ro1) return ((HMI_Roller_Data*)lv_obj_get_user_data(p7ro1))->Name;
	if (id == p7ro2) return ((HMI_Roller_Data*)lv_obj_get_user_data(p7ro2))->Name;
	if (id == p7rect6) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p7rect6))->Name;
	if (id == p7l6) return ((HMI_Label_Data*)lv_obj_get_user_data(p7l6))->Name;
	if (id == p7fb1->getObj()) return ((HMI_FileBrowser_Data*)get_list_user_data(p7fb1->getObj()))->Name;
	return NULL;
}

int MyHMI::GET_SN(int p)
{
	return p < 1 || p > 999 ? 0 : p;
}

int MyHMI::GET_SN(int p, HMI_SN obj, int id)
{
	if (p < 1 || p > 999 || id < 0) return 0;
	return (id * 100000) + ((int)obj % 100 * 1000) + p;
}

void MyHMI::gotoPage(lv_obj_t* pageId)
{
	static lv_point_t valid_pos;
	valid_pos.x = -1;
	while (pageId != NULL)
	{
		pageId = lv_obj_get_child(tileview, pageId);
		valid_pos.x++;
	}
	lv_tileview_set_valid_positions(tileview, &valid_pos, 1);
}

lv_obj_t* MyHMI::getCurrentPage()
{
	return currentPage;
}

int MyHMI::getCurrentPageNumber()
{
	return ID_TO_SN(getCurrentPage());
}

void MyHMI::setLanguage(int lang_id)
{
	if (lang_id < 1 || lang_id > 9) return;
	_LangID = lang_id - 1;
	LabelRefreshText(p1l1);
	ButtonRefreshText(p1b1);
	ButtonRefreshText(p1b2);
	LabelRefreshText(p1l2);
	LabelRefreshText(p1l3);
	LabelRefreshText(p1l4);
	LabelRefreshText(p1l5);
	LabelRefreshText(p1l6);
	LabelRefreshText(p1l7);
	LabelRefreshText(p1l8);
	LabelRefreshText(p1l9);
	NumberRefreshText(p1num1);
	NumberRefreshText(p1num2);
	LabelRefreshText(p2l1);
	ButtonRefreshText(p2b1);
	ButtonRefreshText(p2b2);
	LabelRefreshText(p2l2);
	LabelRefreshText(p2l5);
	LabelRefreshText(p2l6);
	LabelRefreshText(p2l7);
	ButtonRefreshText(p2b3);
	ToggleButtonRefreshText(p2tb1);
	ButtonRefreshText(p2b4);
	ToggleButtonRefreshText(p2tb2);
	ButtonRefreshText(p2b5);
	ButtonRefreshText(p2b6);
	ToggleButtonRefreshText(p2tb3);
	ToggleButtonRefreshText(p2tb4);
	LabelRefreshText(p3l1);
	ButtonRefreshText(p3b1);
	ButtonRefreshText(p3b2);
	LabelRefreshText(p3l9);
	LabelRefreshText(p3l8);
	LabelRefreshText(p3l2);
	LabelRefreshText(p3l6);
	LabelRefreshText(p3l7);
	SliderRefreshText(p3sl2);
	SliderRefreshText(p3sl3);
	ArcSliderRefreshText(p3asl1);
	ArcSliderRefreshText(p3asl2);
	CheckboxRefreshText(p3cb1);
	CheckboxRefreshText(p3cb2);
	CheckboxRefreshText(p3cb3);
	CheckboxRefreshText(p3cb4);
	LabelRefreshText(p4l1);
	ButtonRefreshText(p4b1);
	ButtonRefreshText(p4b2);
	LabelRefreshText(p4l2);
	LabelRefreshText(p4l3);
	LabelRefreshText(p4l4);
	LabelRefreshText(p4l6);
	LabelRefreshText(p4l7);
	LabelRefreshText(p4l8);
	BarRefreshText(p4bar2);
	BarRefreshText(p4bar3);
	ArcBarRefreshText(p4abar1);
	ArcBarRefreshText(p4abar2);
	LabelRefreshText(p5l1);
	ButtonRefreshText(p5b1);
	ButtonRefreshText(p5b2);
	LabelRefreshText(p5l2);
	LabelRefreshText(p5l6);
	LabelRefreshText(p5l7);
	TableRefreshText(p5tbl1);
	ChartRefreshText(p5ch1);
	LabelRefreshText(p5l8);
	LabelRefreshText(p6l1);
	ButtonRefreshText(p6b1);
	ButtonRefreshText(p6b2);
	LabelRefreshText(p6l2);
	LabelRefreshText(p6l3);
	LabelRefreshText(p6l4);
	LabelRefreshText(p6l5);
	LabelRefreshText(p7l1);
	ButtonRefreshText(p7b1);
	ButtonRefreshText(p7b2);
	LabelRefreshText(p7l2);
	LabelRefreshText(p7l3);
	LabelRefreshText(p7l4);
	DropDownListRefreshText(p7ddl1);
	DropDownListRefreshText(p7ddl2);
	ListRefreshText(p7li1);
	RollerRefreshText(p7ro1);
	RollerRefreshText(p7ro2);
	LabelRefreshText(p7l6);
}

void MyHMI::setEnable(lv_obj_t* obj, bool _Enable)
{
	void (*enable_cb)(lv_obj_t*, lv_state_t) = _Enable ? lv_obj_clear_state : lv_obj_add_state;
	lv_obj_t* child = NULL;
	if (_ISOBJ(obj, HMI_B) || _ISOBJ(obj, HMI_TB))
	{
		lv_btn_set_state(obj, lv_btn_get_state(obj) == LV_BTN_STATE_CHECKED_RELEASED || lv_btn_get_state(obj) == LV_BTN_STATE_CHECKED_PRESSED || lv_btn_get_state(obj) == LV_BTN_STATE_CHECKED_DISABLED ? (_Enable ? LV_BTN_STATE_CHECKED_RELEASED : LV_BTN_STATE_CHECKED_DISABLED) : (_Enable ? LV_BTN_STATE_RELEASED : LV_BTN_STATE_DISABLED));
	}
	else if (_ISOBJ(obj, HMI_CB))
	{
		lv_btn_set_state(obj, lv_btn_get_state(obj) == LV_BTN_STATE_CHECKED_RELEASED || lv_btn_get_state(obj) == LV_BTN_STATE_CHECKED_PRESSED || lv_btn_get_state(obj) == LV_BTN_STATE_CHECKED_DISABLED ? (_Enable ? LV_BTN_STATE_CHECKED_RELEASED : LV_BTN_STATE_CHECKED_DISABLED) : (_Enable ? LV_BTN_STATE_RELEASED : LV_BTN_STATE_DISABLED));
		lv_checkbox_ext_t * ext = (lv_checkbox_ext_t*)lv_obj_get_ext_attr(obj);
		enable_cb(ext->bullet, LV_STATE_DISABLED);
		enable_cb(ext->label, LV_STATE_DISABLED);
	}
	else if (_ISOBJ(obj, HMI_TIN) || _ISOBJ(obj, HMI_NIN) || _ISOBJ(obj, HMI_DIN) || _ISOBJ(obj, HMI_SP) || _ISOBJ(obj, HMI_RO))
	{
		while (child = lv_obj_get_child(obj, child))
			enable_cb(child, LV_STATE_DISABLED);
	}
	else if (_ISOBJ(obj, HMI_LI) || _ISOBJ(obj, HMI_FB))
	{
		while (child = lv_obj_get_child(obj, child))
			enable_cb(child, LV_STATE_DISABLED);
		child = NULL;
		while (child = lv_list_get_next_btn(obj, child))
			enable_cb(child, LV_STATE_DISABLED);
	}
	enable_cb(obj, LV_STATE_DISABLED);
}

void MyHMI::playSound(int soundNo)
{
	switch (soundNo)
	{
	}
}

void MyHMI::playSound(char* description)
{
}

void MyHMI::stopSound(int soundNo)
{
	switch (soundNo)
	{
	}
}

void MyHMI::stopSound(char* description)
{
}

int MyHMI::showMessageBox(char* title, char* btn0, char* btn1, char* btn2)
{
	if (kb != NULL)
		lv_event_send(kb, LV_EVENT_CANCEL, NULL);
	int result = -1;
	static const char* mbox_btns[4];
	mbox_btns[0] = btn0 ? btn0 : "OK";
	mbox_btns[1] = btn0 && btn1 ? btn1 : "";
	mbox_btns[2] = btn1 && btn2 ? btn2 : "";
	mbox_btns[3] = "";
	if (strlen(mbox_btns[0]) == 0) return -1;

	lv_obj_set_click(lv_layer_top(), true);
	lv_obj_set_style_local_bg_color(lv_layer_top(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
	lv_obj_set_style_local_bg_opa(lv_layer_top(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
	if (popUpMsgBox == NULL)
		popUpMsgBox = lv_msgbox_create(lv_layer_top(), NULL);
	lv_msgbox_set_text(popUpMsgBox, title);
	lv_msgbox_add_btns(popUpMsgBox, mbox_btns);
	lv_obj_set_event_cb(popUpMsgBox, popUpMsgBoxEventCallback);

	setMessageBoxWidth(popUpMsgWidth);
	lv_obj_align(popUpMsgBox, NULL, LV_ALIGN_CENTER, 0, 0);

	if (coroutineState != IN_HELPER)
	{
		popUpBlock = true;
		while (popUpBlock)
			helperYield();
		result = lv_msgbox_get_active_btn(popUpMsgBox);

		lv_obj_set_hidden(popUpMsgBox, true);
		lv_obj_del(popUpMsgBox);
		popUpMsgBox = NULL;
		lv_obj_set_click(lv_layer_top(), false);
		lv_obj_set_style_local_bg_opa(lv_layer_top(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	}
	else
		popUpBlock = false;
	return result;
}

void MyHMI::setMessageBoxWidth(MBOX_WIDTH width)
{
	popUpMsgWidth = width;
	if (popUpMsgBox == NULL) return;
	switch (popUpMsgWidth)
	{
		case MBOX_NORMAL: lv_obj_set_width(popUpMsgBox, get_x86_hor_resolution() * 0.4); break;
		case MBOX_LARGER: lv_obj_set_width(popUpMsgBox, get_x86_hor_resolution() * 0.6); break;
		case MBOX_LARGEST: lv_obj_set_width(popUpMsgBox, get_x86_hor_resolution() * 0.8); break;
	}
}

void MyHMI::showAlarmBanner(char* msg, int bank)
{
	if (msg == NULL) return;
	if (bank < 0 || bank > ALARM_BANNER_BANK_SIZE) return;

	if (ab == NULL)
	{
		ab = lv_cont_create(lv_layer_top(), NULL);
		lv_obj_set_width(ab, get_x86_hor_resolution());
		lv_obj_set_style_local_border_width(ab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
		lv_obj_set_style_local_radius(ab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
		lv_obj_set_style_local_bg_opa(ab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
		lv_obj_set_style_local_bg_color(ab, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFBDC04));

		abLabel = lv_label_create(ab, NULL);
		lv_label_set_long_mode(abLabel, LV_LABEL_LONG_SROLL_CIRC);
		lv_label_set_align(abLabel, LV_LABEL_ALIGN_CENTER);
		lv_obj_set_style_local_text_color(abLabel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x3B3E42));
		lv_obj_set_width(abLabel, get_x86_hor_resolution());

		char* buf = envPathCombine("IRSPATH", "IMAGE/blockade.png");
		lv_obj_t* img1 = lv_img_create(ab, NULL);
		lv_img_set_src(img1, buf);
		lv_obj_set_width(img1, get_x86_hor_resolution());

		lv_obj_t* img2 = lv_img_create(ab, NULL);
		lv_img_set_src(img2, buf);
		lv_obj_set_width(img2, get_x86_hor_resolution());

		free(buf);

		lv_obj_set_height(ab, lv_obj_get_height(abLabel) + lv_obj_get_height(img1) + lv_obj_get_height(img2) + 30);
		lv_obj_align(abLabel, ab, LV_ALIGN_CENTER, 0, 0);
		lv_obj_align(img1, ab, LV_ALIGN_IN_TOP_MID, 0, 0);
		lv_obj_align(img2, ab, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
		setAlarmBannerPosition(abPos);
	}

	if (abBank[bank] != NULL)
	{
		if (strcmp(msg, abBank[bank]) == 0) return;
		free(abBank[bank]);
	}
	abBank[bank] = (char*)malloc(sizeof(char) * (strlen(msg) + 1));
	strcpy(abBank[bank], msg);
	alarmBannerRefreshText();
}

void MyHMI::deleteAlarmBanner(int bank)
{
	int i;
	if (bank >= 0 && bank < ALARM_BANNER_BANK_SIZE)
	{
		if (abBank[bank] == NULL) return;
		free(abBank[bank]);
		abBank[bank] = NULL;
		for (i = 0; i < ALARM_BANNER_BANK_SIZE; i++)
			if (abBank[i]) break;
		if (i == ALARM_BANNER_BANK_SIZE)
		{
			lv_obj_del(ab);
			ab = NULL;
		}
		else
			alarmBannerRefreshText();
	}
	else if (bank == -1)
	{
		for (i = 0; i < ALARM_BANNER_BANK_SIZE; i++)
			if (abBank[i])
			{
				free(abBank[i]);
				abBank[i] = NULL;
			}
		if (ab) lv_obj_del(ab);
		ab = NULL;
	}
}

void MyHMI::setAlarmBannerSpacing(int space)
{
	if (space < 0) return;
	abBankSpacing = space;
	alarmBannerRefreshText();
}

void MyHMI::setAlarmBannerPosition(ABANNER_POS pos)
{
	abPos = pos;
	if (ab == NULL) return;
	switch (abPos)
	{
	case ABANNER_TOP:
		lv_obj_align(ab, lv_layer_top(), LV_ALIGN_IN_TOP_MID, 0, 0);
		break;
	case ABANNER_CENTER:
		lv_obj_align(ab, lv_layer_top(), LV_ALIGN_CENTER, 0, 0);
		break;
	case ABANNER_BOTTOM:
		lv_obj_align(ab, lv_layer_top(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
		break;
	}
}

/* Commands Function */
void MyHMI::enablePageTimer(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_PAGE)) return;
	HMI_SlidePage_Data* d = (HMI_SlidePage_Data*)lv_obj_get_user_data(id);
	d->_t = timer_NowTime();
	d->_en = true;
	d->TimerEnable = true;
}

void MyHMI::enablePageTimer(int sn)
{
	return enablePageTimer(SN_TO_ID(sn));
}

void MyHMI::enablePageTimer(char* name)
{
	return enablePageTimer(NAME_TO_ID(name));
}

void MyHMI::disablePageTimer(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_PAGE)) return;
	HMI_SlidePage_Data* d = (HMI_SlidePage_Data*)lv_obj_get_user_data(id);
	d->_en = false;
	d->TimerEnable = false;
}

void MyHMI::disablePageTimer(int sn)
{
	return disablePageTimer(SN_TO_ID(sn));
}

void MyHMI::disablePageTimer(char* name)
{
	return disablePageTimer(NAME_TO_ID(name));
}

void MyHMI::setArcBarValue(lv_obj_t* id, double value)
{
	if (!_ISOBJ(id, HMI_ABAR)) return;
	HMI_Arc_Data* d = (HMI_Arc_Data*)lv_obj_get_user_data(id);
	if (value > d->MaxValue)
		d->Value = d->MaxValue;
	else if (value < d->MinValue)
		d->Value = d->MinValue;
	else
		d->Value = value;
	lv_arc_set_value(id, (d->Value - d->MinValue) / (d->MaxValue - d->MinValue) * (lv_arc_get_max_value(id) - lv_arc_get_min_value(id)) + lv_arc_get_min_value(id));
	lv_event_send(id, LV_EVENT_VALUE_CHANGED, NULL);
}

void MyHMI::setArcBarValue(int sn, double value)
{
	return setArcBarValue(SN_TO_ID(sn), value);
}

void MyHMI::setArcBarValue(char* name, double value)
{
	return setArcBarValue(NAME_TO_ID(name), value);
}

void MyHMI::setArcSliderValue(lv_obj_t* id, int value)
{
	if (!_ISOBJ(id, HMI_ASL)) return;
	HMI_Arc_Data* d = (HMI_Arc_Data*)lv_obj_get_user_data(id);
	lv_arc_set_value(id, (int16_t)value);
	lv_event_send(id, LV_EVENT_VALUE_CHANGED, NULL);
}

void MyHMI::setArcSliderValue(int sn, int value)
{
	return setArcSliderValue(SN_TO_ID(sn), value);
}

void MyHMI::setArcSliderValue(char* name, int value)
{
	return setArcSliderValue(NAME_TO_ID(name), value);
}

void MyHMI::setBarValue(lv_obj_t* id, double value)
{
	if (!_ISOBJ(id, HMI_BAR)) return;
	HMI_Bar_Data* d = (HMI_Bar_Data*)lv_obj_get_user_data(id);
	if (value > d->MaxValue)
		d->Value = d->MaxValue;
	else if (value < d->MinValue)
		d->Value = d->MinValue;
	else
		d->Value = value;
	lv_bar_set_value(id, (d->Value - d->MinValue) / (d->MaxValue - d->MinValue) * (lv_bar_get_max_value(id) - lv_bar_get_min_value(id)) + lv_bar_get_min_value(id), LV_ANIM_OFF);
	lv_event_send(id, LV_EVENT_VALUE_CHANGED, NULL);
}

void MyHMI::setBarValue(int sn, double value)
{
	return setBarValue(SN_TO_ID(sn), value);
}

void MyHMI::setBarValue(char* name, double value)
{
	return setBarValue(NAME_TO_ID(name), value);
}

void MyHMI::setToggleButtonState(lv_obj_t* id, bool checked)
{
	if (!_ISOBJ(id, HMI_TB)) return;
	lv_btn_set_state(id, lv_obj_get_state(id, LV_OBJ_PART_MAIN) & LV_STATE_DISABLED ? (checked ? LV_BTN_STATE_CHECKED_DISABLED : LV_BTN_STATE_DISABLED) : (checked ? LV_BTN_STATE_CHECKED_RELEASED : LV_BTN_STATE_RELEASED));
	lv_event_send(id, LV_EVENT_VALUE_CHANGED, NULL);
}

void MyHMI::setToggleButtonState(int sn, bool checked)
{
	return setToggleButtonState(SN_TO_ID(sn), checked);
}

void MyHMI::setToggleButtonState(char* name, bool checked)
{
	return setToggleButtonState(NAME_TO_ID(name), checked);
}

void MyHMI::setCheckboxState(lv_obj_t* id, bool checked)
{
	if (!_ISOBJ(id, HMI_CB)) return;
	lv_checkbox_set_checked(id, checked);
	lv_event_send(id, LV_EVENT_VALUE_CHANGED, NULL);
}

void MyHMI::setCheckboxState(int sn, bool checked)
{
	return setCheckboxState(SN_TO_ID(sn), checked);
}

void MyHMI::setCheckboxState(char* name, bool checked)
{
	return setCheckboxState(NAME_TO_ID(name), checked);
}

void MyHMI::setChartValue(lv_obj_t* id, int channel, int index, int value)
{
	if (!_ISOBJ(id, HMI_CH)) return;
	lv_chart_series_t* i;
	int cnt = channel;
	HMI_Chart_Data* d = (HMI_Chart_Data*)lv_obj_get_user_data(id);
	if (channel < 1 || channel > d->ChannelCount + 1) return;
	lv_ll_t * ll_p = &((lv_chart_ext_t*)lv_obj_get_ext_attr(id))->series_ll;
	for (i = (lv_chart_series_t*)_lv_ll_get_tail(ll_p); cnt > 1; cnt--, i = (lv_chart_series_t*)_lv_ll_get_prev(ll_p, i));
	lv_chart_set_point_id(id, i, value, index);
	lv_chart_refresh(id);
}

void MyHMI::setChartValue(int sn, int channel, int index, int value)
{
	return setChartValue(SN_TO_ID(sn), channel, index, value);
}

void MyHMI::setChartValue(char* name, int channel, int index, int value)
{
	return setChartValue(NAME_TO_ID(name), channel, index, value);
}

void MyHMI::setChartValues(lv_obj_t* id, int channel, int* values)
{
	if (!_ISOBJ(id, HMI_CH)) return;
	lv_chart_series_t* i;
	int cnt = channel;
	HMI_Chart_Data* d = (HMI_Chart_Data*)lv_obj_get_user_data(id);
	if (channel < 1 || channel > d->ChannelCount + 1) return;
	lv_ll_t * ll_p = &((lv_chart_ext_t*)lv_obj_get_ext_attr(id))->series_ll;
	for (i = (lv_chart_series_t*)_lv_ll_get_tail(ll_p); cnt > 1; cnt--, i = (lv_chart_series_t*)_lv_ll_get_prev(ll_p, i));
	lv_chart_set_points(id, i, (lv_coord_t*)values);
	lv_chart_refresh(id);
}

void MyHMI::setChartValues(int sn, int channel, int* values)
{
	return setChartValues(SN_TO_ID(sn), channel, values);
}

void MyHMI::setChartValues(char* name, int channel, int* values)
{
	return setChartValues(NAME_TO_ID(name), channel, values);
}

void MyHMI::disableChartChannel(lv_obj_t* id, int channel)
{
	if (!_ISOBJ(id, HMI_CH)) return;
	lv_chart_series_t* i;
	int cnt = channel;
	HMI_Chart_Data* d = (HMI_Chart_Data*)lv_obj_get_user_data(id);
	if (channel < 1 || channel > d->ChannelCount + 1) return;
	lv_ll_t * ll_p = &((lv_chart_ext_t*)lv_obj_get_ext_attr(id))->series_ll;
	for (i = (lv_chart_series_t*)_lv_ll_get_tail(ll_p); cnt > 1; cnt--, i = (lv_chart_series_t*)_lv_ll_get_prev(ll_p, i));
	lv_chart_hide_series(id, i, true);
}

void MyHMI::disableChartChannel(int sn, int channel)
{
	return disableChartChannel(SN_TO_ID(sn), channel);
}

void MyHMI::disableChartChannel(char* name, int channel)
{
	return disableChartChannel(NAME_TO_ID(name), channel);
}

void MyHMI::enableChartChannel(lv_obj_t* id, int channel)
{
	if (!_ISOBJ(id, HMI_CH)) return;
	lv_chart_series_t* i;
	int cnt = channel;
	HMI_Chart_Data* d = (HMI_Chart_Data*)lv_obj_get_user_data(id);
	if (channel < 1 || channel > d->ChannelCount + 1) return;
	lv_ll_t * ll_p = &((lv_chart_ext_t*)lv_obj_get_ext_attr(id))->series_ll;
	for (i = (lv_chart_series_t*)_lv_ll_get_tail(ll_p); cnt > 1; cnt--, i = (lv_chart_series_t*)_lv_ll_get_prev(ll_p, i));
	lv_chart_hide_series(id, i, false);
}

void MyHMI::enableChartChannel(int sn, int channel)
{
	return enableChartChannel(SN_TO_ID(sn), channel);
}

void MyHMI::enableChartChannel(char* name, int channel)
{
	return enableChartChannel(NAME_TO_ID(name), channel);
}

void MyHMI::setGaugeValue(lv_obj_t* id, int value)
{
	setGaugeValue(id, value, 1);
}

void MyHMI::setGaugeValue(int sn, int value)
{
	return setGaugeValue(SN_TO_ID(sn), value);
}

void MyHMI::setGaugeValue(char* name, int value)
{
	return setGaugeValue(NAME_TO_ID(name), value);
}

void MyHMI::setGaugeValue(lv_obj_t* id, int value, int needle)
{
	if (!_ISOBJ(id, HMI_GA)) return;
	if (needle < 1 || needle > lv_gauge_get_needle_count(id) + 1) return;
	lv_gauge_set_value(id, needle - 1, value);
}

void MyHMI::setGaugeValue(int sn, int value, int needle)
{
	return setGaugeValue(SN_TO_ID(sn), value, needle);
}

void MyHMI::setGaugeValue(char* name, int value, int needle)
{
	return setGaugeValue(NAME_TO_ID(name), value, needle);
}

void MyHMI::setImageHidden(lv_obj_t* id, bool hidden)
{
	if (!_ISOBJ(id, HMI_IMG)) return;
	lv_obj_set_hidden(id, hidden);
}

void MyHMI::setImageHidden(int sn, bool hidden)
{
	return setImageHidden(SN_TO_ID(sn), hidden);
}

void MyHMI::setImageHidden(char* name, bool hidden)
{
	return setImageHidden(NAME_TO_ID(name), hidden);
}

void MyHMI::setImageRotate(lv_obj_t* id, double v)
{
	if (!_ISOBJ(id, HMI_IMG)) return;
	setImgObjAngle(id, v);
}

void MyHMI::setImageRotate(int sn, double v)
{
	return setImageRotate(SN_TO_ID(sn), v);
}

void MyHMI::setImageRotate(char* name, double v)
{
	return setImageRotate(NAME_TO_ID(name), v);
}

void MyHMI::setImageZoom(lv_obj_t* id, double v)
{
	if (!_ISOBJ(id, HMI_IMG)) return;
	setImgObjZoom(id, v);
}

void MyHMI::setImageZoom(int sn, double v)
{
	return setImageZoom(SN_TO_ID(sn), v);
}

void MyHMI::setImageZoom(char* name, double v)
{
	return setImageZoom(NAME_TO_ID(name), v);
}

void MyHMI::setImagePos(lv_obj_t* id, int x, int y)
{
	if (!_ISOBJ(id, HMI_IMG)) return;
	lv_obj_set_pos(id, (int32_t)x, (int32_t)y);
}

void MyHMI::setImagePos(int sn, int x, int y)
{
	return setImagePos(SN_TO_ID(sn), x, y);
}

void MyHMI::setImagePos(char* name, int x, int y)
{
	return setImagePos(NAME_TO_ID(name), x, y);
}

void MyHMI::setImageX(lv_obj_t* id, int x)
{
	if (!_ISOBJ(id, HMI_IMG)) return;
	lv_obj_set_x(id, (int32_t)x);
}

void MyHMI::setImageX(int sn, int x)
{
	return setImageX(SN_TO_ID(sn), x);
}

void MyHMI::setImageX(char* name, int x)
{
	return setImageX(NAME_TO_ID(name), x);
}

void MyHMI::setImageY(lv_obj_t* id, int y)
{
	if (!_ISOBJ(id, HMI_IMG)) return;
	lv_obj_set_y(id, (int32_t)y);
}

void MyHMI::setImageY(int sn, int y)
{
	return setImageY(SN_TO_ID(sn), y);
}

void MyHMI::setImageY(char* name, int y)
{
	return setImageY(NAME_TO_ID(name), y);
}

void MyHMI::setLabelText(lv_obj_t* id, char* text)
{
	if (!_ISOBJ(id, HMI_L)) return;
	if (text == NULL) return;
	HMI_Label_Data* d = (HMI_Label_Data*)lv_obj_get_user_data(id);
	if (!strcmp(d->_textlist[_LangID], text)) return;
	free(d->_textlist[_LangID]);
	d->_textlist[_LangID] = strdup(text);
	LabelRefreshText(id);
}

void MyHMI::setLabelText(int sn, char* text)
{
	return setLabelText(SN_TO_ID(sn), text);
}

void MyHMI::setLabelText(char* name, char* text)
{
	return setLabelText(NAME_TO_ID(name), text);
}

void MyHMI::setLabelText(lv_obj_t* id, char* text, int langID)
{
	if (!_ISOBJ(id, HMI_L)) return;
	if (langID < 1 || langID > 9) return;
	if (text == NULL) return;
	HMI_Label_Data* d = (HMI_Label_Data*)lv_obj_get_user_data(id);
	if (!strcmp(d->_textlist[langID-1], text)) return;
	free(d->_textlist[langID-1]);
	d->_textlist[langID-1] = strdup(text);
	LabelRefreshText(id);
}

void MyHMI::setLabelText(int sn, char* text, int langID)
{
	return setLabelText(SN_TO_ID(sn), text, langID);
}

void MyHMI::setLabelText(char* name, char* text, int langID)
{
	return setLabelText(NAME_TO_ID(name), text, langID);
}

void MyHMI::setNumberValue(lv_obj_t* id, double value)
{
	if (!_ISOBJ(id, HMI_NUM)) return;
	((HMI_Number_Data*)lv_obj_get_user_data(id))->Value = value;
	NumberRefreshText(id);
}

void MyHMI::setNumberValue(int sn, double value)
{
	return setNumberValue(SN_TO_ID(sn), value);
}

void MyHMI::setNumberValue(char* name, double value)
{
	return setNumberValue(NAME_TO_ID(name), value);
}

void MyHMI::setLedBright(lv_obj_t* id, int bright)
{
	if (!_ISOBJ(id, HMI_LED)) return;
	if (bright > 100)
		lv_led_set_bright(id, 255);
	else
		lv_led_set_bright(id, (int16_t)(bright * 135 / 100 + 120));
}

void MyHMI::setLedBright(int sn, int bright)
{
	return setLedBright(SN_TO_ID(sn), bright);
}

void MyHMI::setLedBright(char* name, int bright)
{
	return setLedBright(NAME_TO_ID(name), bright);
}

void MyHMI::setSliderValue(lv_obj_t* id, int value)
{
	if (!_ISOBJ(id, HMI_SL)) return;
	lv_slider_set_value(id, (int16_t)value, LV_ANIM_OFF);
	lv_event_send(id, LV_EVENT_VALUE_CHANGED, NULL);
}

void MyHMI::setSliderValue(int sn, int value)
{
	return setSliderValue(SN_TO_ID(sn), value);
}

void MyHMI::setSliderValue(char* name, int value)
{
	return setSliderValue(NAME_TO_ID(name), value);
}

void MyHMI::setSpinboxValue(lv_obj_t* id, double value)
{
	if (!_ISOBJ(id, HMI_SP)) return;
	HMI_Spinbox_Data* d = (HMI_Spinbox_Data*)lv_obj_get_user_data(id);
	if (2147483647 / pow(10, d->Decimal) < value || -2147483648 / pow(10, d->Decimal) > value) return;
	lv_spinbox_set_value(lv_obj_get_child_back(id, NULL), value * pow(10, d->Decimal));
}

void MyHMI::setSpinboxValue(int sn, double value)
{
	return setSpinboxValue(SN_TO_ID(sn), value);
}

void MyHMI::setSpinboxValue(char* name, double value)
{
	return setSpinboxValue(NAME_TO_ID(name), value);
}

void MyHMI::setSwitchState(lv_obj_t* id, bool checked)
{
	if (!_ISOBJ(id, HMI_SW)) return;
	if (checked)
		lv_switch_on(id, LV_ANIM_OFF);
	else
		lv_switch_off(id, LV_ANIM_OFF);
	lv_event_send(id, LV_EVENT_VALUE_CHANGED, NULL);
}

void MyHMI::setSwitchState(int sn, bool checked)
{
	return setSwitchState(SN_TO_ID(sn), checked);
}

void MyHMI::setSwitchState(char* name, bool checked)
{
	return setSwitchState(NAME_TO_ID(name), checked);
}

void MyHMI::setTableCellText(lv_obj_t* id, int x, int y, char* text)
{
	if (!_ISOBJ(id, HMI_TBL)) return;
	lv_table_set_cell_value(id, (uint16_t)y, (uint16_t)x, text);
}

void MyHMI::setTableCellText(int sn, int x, int y, char* text)
{
	return setTableCellText(SN_TO_ID(sn), x, y, text);
}

void MyHMI::setTableCellText(char* name, int x, int y, char* text)
{
	return setTableCellText(NAME_TO_ID(name), x, y, text);
}

void MyHMI::setTableCellValue(lv_obj_t* id, int x, int y, int value)
{
	if (!_ISOBJ(id, HMI_TBL)) return;
	lv_table_set_cell_value_fmt(id, (uint16_t)y, (uint16_t)x, "%d", value);
}

void MyHMI::setTableCellValue(int sn, int x, int y, int value)
{
	return setTableCellValue(SN_TO_ID(sn), x, y, value);
}

void MyHMI::setTableCellValue(char* name, int x, int y, int value)
{
	return setTableCellValue(NAME_TO_ID(name), x, y, value);
}

void MyHMI::setTableCellValue(lv_obj_t* id, int x, int y, double value)
{
	if (!_ISOBJ(id, HMI_TBL)) return;
	lv_table_set_cell_value_fmt(id, (uint16_t)y, (uint16_t)x, "%f", value);
}

void MyHMI::setTableCellValue(int sn, int x, int y, double value)
{
	return setTableCellValue(SN_TO_ID(sn), x, y, value);
}

void MyHMI::setTableCellValue(char* name, int x, int y, double value)
{
	return setTableCellValue(NAME_TO_ID(name), x, y, value);
}

void MyHMI::setTableAlign(lv_obj_t* id, int x, int y, lv_label_align_t align)
{
	if (!_ISOBJ(id, HMI_TBL)) return;
	lv_table_set_cell_align(id, (uint16_t)y, (uint16_t)x, align);
}

void MyHMI::setTableAlign(int sn, int x, int y, lv_label_align_t align)
{
	return setTableAlign(SN_TO_ID(sn), x, y, align);
}

void MyHMI::setTableAlign(char* name, int x, int y, lv_label_align_t align)
{
	return setTableAlign(NAME_TO_ID(name), x, y, align);
}

void MyHMI::setTableAlign(lv_obj_t* id, int x1, int y1, int x2, int y2, lv_label_align_t align)
{
	if (!_ISOBJ(id, HMI_TBL)) return;
	if (x1 < 0 || y1 < 0) return;
	for (int i = x1; i < lv_table_get_col_cnt(id) && i <= x2; i++)
		for (int j = y1; j < lv_table_get_row_cnt(id) && j <= y2; j++)
			lv_table_set_cell_align(id, j, i, align);
}

void MyHMI::setTableAlign(int sn, int x1, int y1, int x2, int y2, lv_label_align_t align)
{
	return setTableAlign(SN_TO_ID(sn), x1, y1, x2, y2, align);
}

void MyHMI::setTableAlign(char* name, int x1, int y1, int x2, int y2, lv_label_align_t align)
{
	return setTableAlign(NAME_TO_ID(name), x1, y1, x2, y2, align);
}

void MyHMI::setTableRowAlign(lv_obj_t* id, int row, lv_label_align_t align)
{
	if (!_ISOBJ(id, HMI_TBL)) return;
	for (int i = 0; i < lv_table_get_col_cnt(id); i++)
	lv_table_set_cell_align(id, (uint16_t)row, i, align);
}

void MyHMI::setTableRowAlign(int sn, int row, lv_label_align_t align)
{
	return setTableRowAlign(SN_TO_ID(sn), row, align);
}

void MyHMI::setTableRowAlign(char* name, int row, lv_label_align_t align)
{
	return setTableRowAlign(NAME_TO_ID(name), row, align);
}

void MyHMI::setTableColAlign(lv_obj_t* id, int col, lv_label_align_t align)
{
	if (!_ISOBJ(id, HMI_TBL)) return;
	for (int i = 0; i < lv_table_get_row_cnt(id); i++)
	lv_table_set_cell_align(id, i, (uint16_t)col, align);
}

void MyHMI::setTableColAlign(int sn, int col, lv_label_align_t align)
{
	return setTableColAlign(SN_TO_ID(sn), col, align);
}

void MyHMI::setTableColAlign(char* name, int col, lv_label_align_t align)
{
	return setTableColAlign(NAME_TO_ID(name), col, align);
}

void MyHMI::setTableCellStyle(lv_obj_t* id, int x, int y, int styleID)
{
	if (!_ISOBJ(id, HMI_TBL)) return;
	if (styleID < LV_TABLE_PART_CELL1 || styleID > LV_TABLE_PART_CELL16) return;
	lv_table_set_cell_type(id, (uint16_t)y, (uint16_t)x, styleID);
}

void MyHMI::setTableCellStyle(int sn, int x, int y, int styleID)
{
	return setTableCellStyle(SN_TO_ID(sn), x, y, styleID);
}

void MyHMI::setTableCellStyle(char* name, int x, int y, int styleID)
{
	return setTableCellStyle(NAME_TO_ID(name), x, y, styleID);
}

void MyHMI::setTableCellStyle(lv_obj_t* id, int x1, int y1, int x2, int y2, int styleID)
{
	if (!_ISOBJ(id, HMI_TBL)) return;
	if (x1 < 0 || y1 < 0) return;
	if (styleID < LV_TABLE_PART_CELL1 || styleID > LV_TABLE_PART_CELL16) return;
	for (int i = x1; i < lv_table_get_col_cnt(id) && i <= x2; i++)
		for (int j = y1; j < lv_table_get_row_cnt(id) && j <= y2; j++)
			lv_table_set_cell_type(id, j, i, styleID);
}

void MyHMI::setTableCellStyle(int sn, int x1, int y1, int x2, int y2, int styleID)
{
	return setTableCellStyle(SN_TO_ID(sn), x1, y1, x2, y2, styleID);
}

void MyHMI::setTableCellStyle(char* name, int x1, int y1, int x2, int y2, int styleID)
{
	return setTableCellStyle(NAME_TO_ID(name), x1, y1, x2, y2, styleID);
}

void MyHMI::setTableRowStyle(lv_obj_t* id, int row, int styleID)
{
	if (!_ISOBJ(id, HMI_TBL)) return;
	if (styleID < LV_TABLE_PART_CELL1 || styleID > LV_TABLE_PART_CELL16) return;
	for (int i = 0; i < lv_table_get_col_cnt(id); i++)
	lv_table_set_cell_type(id, (uint16_t)row, i, styleID);
}

void MyHMI::setTableRowStyle(int sn, int row, int styleID)
{
	return setTableRowStyle(SN_TO_ID(sn), row, styleID);
}

void MyHMI::setTableRowStyle(char* name, int row, int styleID)
{
	return setTableRowStyle(NAME_TO_ID(name), row, styleID);
}

void MyHMI::setTableColStyle(lv_obj_t* id, int col, int styleID)
{
	if (!_ISOBJ(id, HMI_TBL)) return;
	if (styleID < LV_TABLE_PART_CELL1 || styleID > LV_TABLE_PART_CELL16) return;
	for (int i = 0; i < lv_table_get_row_cnt(id); i++)
	lv_table_set_cell_type(id, i, (uint16_t)col, styleID);
}

void MyHMI::setTableColStyle(int sn, int col, int styleID)
{
	return setTableColStyle(SN_TO_ID(sn), col, styleID);
}

void MyHMI::setTableColStyle(char* name, int col, int styleID)
{
	return setTableColStyle(NAME_TO_ID(name), col, styleID);
}

void MyHMI::setTableStyleTextColor(lv_obj_t* id, int styleID, int color)
{
	if (!_ISOBJ(id, HMI_TBL)) return;
	if (styleID < LV_TABLE_PART_CELL1 || styleID > LV_TABLE_PART_CELL16) return;
	lv_obj_set_style_local_text_color(id, styleID, LV_STATE_DEFAULT, lv_color_hex((uint32_t)color));
}

void MyHMI::setTableStyleTextColor(int sn, int styleID, int color)
{
	return setTableStyleTextColor(SN_TO_ID(sn), styleID, color);
}

void MyHMI::setTableStyleTextColor(char* name, int styleID, int color)
{
	return setTableStyleTextColor(NAME_TO_ID(name), styleID, color);
}

void MyHMI::setTableStyleBgColor(lv_obj_t* id, int styleID, int color)
{
	if (!_ISOBJ(id, HMI_TBL)) return;
	if (styleID < LV_TABLE_PART_CELL1 || styleID > LV_TABLE_PART_CELL16) return;
	lv_obj_set_style_local_bg_color(id, styleID, LV_STATE_DEFAULT, lv_color_hex((uint32_t)color));
}

void MyHMI::setTableStyleBgColor(int sn, int styleID, int color)
{
	return setTableStyleBgColor(SN_TO_ID(sn), styleID, color);
}

void MyHMI::setTableStyleBgColor(char* name, int styleID, int color)
{
	return setTableStyleBgColor(NAME_TO_ID(name), styleID, color);
}

void MyHMI::setTextInputText(lv_obj_t* id, char* text)
{
	if (!_ISOBJ(id, HMI_TIN)) return;
	lv_textarea_set_text(id, text);
}

void MyHMI::setTextInputText(int sn, char* text)
{
	return setTextInputText(SN_TO_ID(sn), text);
}

void MyHMI::setTextInputText(char* name, char* text)
{
	return setTextInputText(NAME_TO_ID(name), text);
}

void MyHMI::setQrCodeLinkSource(lv_obj_t* id, char* source)
{
	if (!_ISOBJ(id, HMI_QR)) return;
	lv_qrcode_update(id, source, strlen(source));
}

void MyHMI::setQrCodeLinkSource(int sn, char* source)
{
	return setQrCodeLinkSource(SN_TO_ID(sn), source);
}

void MyHMI::setQrCodeLinkSource(char* name, char* source)
{
	return setQrCodeLinkSource(NAME_TO_ID(name), source);
}

void MyHMI::setImageToggleButtonState(lv_obj_t* id, bool checked)
{
	if (!_ISOBJ(id, HMI_ITB)) return;
	lv_imgbtn_set_state(id, lv_obj_get_state(id, LV_OBJ_PART_MAIN) & LV_STATE_DISABLED ? (checked ? LV_BTN_STATE_CHECKED_DISABLED : LV_BTN_STATE_DISABLED) : (checked ? LV_BTN_STATE_CHECKED_RELEASED : LV_BTN_STATE_RELEASED));
	lv_event_send(id, LV_EVENT_VALUE_CHANGED, NULL);
}

void MyHMI::setImageToggleButtonState(int sn, bool checked)
{
	return setImageToggleButtonState(SN_TO_ID(sn), checked);
}

void MyHMI::setImageToggleButtonState(char* name, bool checked)
{
	return setImageToggleButtonState(NAME_TO_ID(name), checked);
}

void MyHMI::addOscSample(lv_obj_t* id, double value)
{
	if (!_ISOBJ(id, HMI_OSC)) return;
	((HMI_Oscilloscope_Data*)lv_obj_get_user_data(id))->_osc->addSample(value);
}

void MyHMI::addOscSample(int sn, double value)
{
	return addOscSample(SN_TO_ID(sn), value);
}

void MyHMI::addOscSample(char* name, double value)
{
	return addOscSample(NAME_TO_ID(name), value);
}

void MyHMI::addOscSample(Oscilloscope* id, double value)
{
	return addOscSample(id->getObj(), value);
}

void MyHMI::setOscTriggerMode(lv_obj_t* id, OSC_TRIGGER_MODE mode)
{
	if (!_ISOBJ(id, HMI_OSC)) return;
	((HMI_Oscilloscope_Data*)lv_obj_get_user_data(id))->_osc->setTriggerMode((OSC_TRIGGER_MODE)mode);
}

void MyHMI::setOscTriggerMode(int sn, OSC_TRIGGER_MODE mode)
{
	return setOscTriggerMode(SN_TO_ID(sn), mode);
}

void MyHMI::setOscTriggerMode(char* name, OSC_TRIGGER_MODE mode)
{
	return setOscTriggerMode(NAME_TO_ID(name), mode);
}

void MyHMI::setOscTriggerMode(Oscilloscope* id, OSC_TRIGGER_MODE mode)
{
	return setOscTriggerMode(id->getObj(), mode);
}

void MyHMI::setOscTriggerLevel(lv_obj_t* id, double value)
{
	if (!_ISOBJ(id, HMI_OSC)) return;
	((HMI_Oscilloscope_Data*)lv_obj_get_user_data(id))->_osc->setTriggerLevel(value);
}

void MyHMI::setOscTriggerLevel(int sn, double value)
{
	return setOscTriggerLevel(SN_TO_ID(sn), value);
}

void MyHMI::setOscTriggerLevel(char* name, double value)
{
	return setOscTriggerLevel(NAME_TO_ID(name), value);
}

void MyHMI::setOscTriggerLevel(Oscilloscope* id, double value)
{
	return setOscTriggerLevel(id->getObj(), value);
}

void MyHMI::setOscTriggerOffset(lv_obj_t* id, long value)
{
	if (!_ISOBJ(id, HMI_OSC)) return;
	((HMI_Oscilloscope_Data*)lv_obj_get_user_data(id))->_osc->setOffset(value);
}

void MyHMI::setOscTriggerOffset(int sn, long value)
{
	return setOscTriggerOffset(SN_TO_ID(sn), value);
}

void MyHMI::setOscTriggerOffset(char* name, long value)
{
	return setOscTriggerOffset(NAME_TO_ID(name), value);
}

void MyHMI::setOscTriggerOffset(Oscilloscope* id, long value)
{
	return setOscTriggerOffset(id->getObj(), value);
}

void MyHMI::setOscTimeDiv(lv_obj_t* id, unsigned long value)
{
	if (!_ISOBJ(id, HMI_OSC)) return;
	((HMI_Oscilloscope_Data*)lv_obj_get_user_data(id))->_osc->setTimeDiv(value);
}

void MyHMI::setOscTimeDiv(int sn, unsigned long value)
{
	return setOscTimeDiv(SN_TO_ID(sn), value);
}

void MyHMI::setOscTimeDiv(char* name, unsigned long value)
{
	return setOscTimeDiv(NAME_TO_ID(name), value);
}

void MyHMI::setOscTimeDiv(Oscilloscope* id, unsigned long value)
{
	return setOscTimeDiv(id->getObj(), value);
}

void MyHMI::setOscYRange(lv_obj_t* id, double min, double max)
{
	if (!_ISOBJ(id, HMI_OSC)) return;
	((HMI_Oscilloscope_Data*)lv_obj_get_user_data(id))->_osc->setYRange(min, max);
}

void MyHMI::setOscYRange(int sn, double min, double max)
{
	return setOscYRange(SN_TO_ID(sn), min, max);
}

void MyHMI::setOscYRange(char* name, double min, double max)
{
	return setOscYRange(NAME_TO_ID(name), min, max);
}

void MyHMI::setOscYRange(Oscilloscope* id, double min, double max)
{
	return setOscYRange(id->getObj(), min, max);
}

void MyHMI::setOscYDiv(lv_obj_t* id, double value)
{
	if (!_ISOBJ(id, HMI_OSC)) return;
	double _min = ((HMI_Oscilloscope_Data*)lv_obj_get_user_data(id))->_osc->getMinValue();
	double _max = _min + value * (((HMI_Oscilloscope_Data*)lv_obj_get_user_data(id))->_osc->getYDivisionLines() + 1);
	setOscYRange(id, _min, _max);
}

void MyHMI::setOscYDiv(int sn, double value)
{
	return setOscYDiv(SN_TO_ID(sn), value);
}

void MyHMI::setOscYDiv(char* name, double value)
{
	return setOscYDiv(NAME_TO_ID(name), value);
}

void MyHMI::setOscYDiv(Oscilloscope* id, double value)
{
	return setOscYDiv(id->getObj(), value);
}

void MyHMI::setNumberInputValue(lv_obj_t* id, double value)
{
	if (!_ISOBJ(id, HMI_NIN)) return;
	char buf[21];
	char buf2[6] = "%.0lf";
	buf2[2] += ((HMI_NumberInput_Data*)lv_obj_get_user_data(id))->ValueType * 6;
	snprintf(buf, 20, buf2, value);
	lv_textarea_set_text(id, buf);
}

void MyHMI::setNumberInputValue(int sn, double value)
{
	return setNumberInputValue(SN_TO_ID(sn), value);
}

void MyHMI::setNumberInputValue(char* name, double value)
{
	return setNumberInputValue(NAME_TO_ID(name), value);
}

void MyHMI::setDateInput(lv_obj_t* id, int year, int month, int day)
{
	if (!_ISOBJ(id, HMI_DIN)) return;
	if (year > 9999 || month < 1 || month > 12 || day < 1 || day > 31) return;
	if (((month < 7 && month % 2 == 0) || (month > 8 && month % 2 == 1)) && day > 30)
		return;
	else if (month == 2 && day > 29)
		return;
	else if (month == 2 && !((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) && day > 28)
		return;
	HMI_DateInput_Data* d = (HMI_DateInput_Data*)lv_obj_get_user_data(id);
	d->DefaultYear = (uint16_t)year;
	d->DefaultMonth = (uint16_t)month;
	d->DefaultDay = (uint16_t)day;
	DateInputRefreshText(id);
}

void MyHMI::setDateInput(int sn, int year, int month, int day)
{
	return setDateInput(SN_TO_ID(sn), year, month, day);
}

void MyHMI::setDateInput(char* name, int year, int month, int day)
{
	return setDateInput(NAME_TO_ID(name), year, month, day);
}

void MyHMI::setImageBarValue(lv_obj_t* id, double value)
{
	if (!_ISOBJ(id, HMI_IBAR)) return;
	HMI_ImageBar_Data* d = (HMI_ImageBar_Data*)lv_obj_get_user_data(id);
	if (value > d->MaxValue)
		d->Value = d->MaxValue;
	else if (value < d->MinValue)
		d->Value = d->MinValue;
	else
		d->Value = value;
	lv_bar_set_value(id, (d->Value - d->MinValue) / (d->MaxValue - d->MinValue) * (lv_bar_get_max_value(id) - lv_bar_get_min_value(id)) + lv_bar_get_min_value(id), LV_ANIM_OFF);
	lv_event_send(id, LV_EVENT_VALUE_CHANGED, NULL);
}

void MyHMI::setImageBarValue(int sn, double value)
{
	return setImageBarValue(SN_TO_ID(sn), value);
}

void MyHMI::setImageBarValue(char* name, double value)
{
	return setImageBarValue(NAME_TO_ID(name), value);
}

void MyHMI::setImageSliderValue(lv_obj_t* id, int value)
{
	if (!_ISOBJ(id, HMI_ISL)) return;
	lv_slider_set_value(id, (int16_t)value, LV_ANIM_OFF);
	lv_event_send(id, LV_EVENT_VALUE_CHANGED, NULL);
}

void MyHMI::setImageSliderValue(int sn, int value)
{
	return setImageSliderValue(SN_TO_ID(sn), value);
}

void MyHMI::setImageSliderValue(char* name, int value)
{
	return setImageSliderValue(NAME_TO_ID(name), value);
}

void MyHMI::setImageGaugeValue(lv_obj_t* id, int value)
{
	if (!_ISOBJ(id, HMI_IGA)) return;
	HMI_Gauge_Data* d = (HMI_Gauge_Data*)lv_obj_get_user_data(id);
	d->NeedleValue1 = value;
}

void MyHMI::setImageGaugeValue(int sn, int value)
{
	return setImageGaugeValue(SN_TO_ID(sn), value);
}

void MyHMI::setImageGaugeValue(char* name, int value)
{
	return setImageGaugeValue(NAME_TO_ID(name), value);
}

void MyHMI::setFileBrowserFilter(lv_obj_t* id, char* filter, int filterIndex)
{
	if (!_ISOBJ(id, HMI_FB)) return;
	((HMI_FileBrowser_Data*)get_list_user_data(id))->_fb->setFilter(filter, filterIndex);
}

void MyHMI::setFileBrowserFilter(int sn, char* filter, int filterIndex)
{
	return setFileBrowserFilter(SN_TO_ID(sn), filter, filterIndex);
}

void MyHMI::setFileBrowserFilter(char* name, char* filter, int filterIndex)
{
	return setFileBrowserFilter(NAME_TO_ID(name), filter, filterIndex);
}

void MyHMI::setFileBrowserFilter(FileBrowser* id, char* filter, int filterIndex)
{
	return setFileBrowserFilter(id->getObj(), filter, filterIndex);
}

void MyHMI::setFileBrowserFilter(lv_obj_t* id, char* filter)
{
	setFileBrowserFilter(id, filter, 0);
}

void MyHMI::setFileBrowserFilter(int sn, char* filter)
{
	return setFileBrowserFilter(SN_TO_ID(sn), filter);
}

void MyHMI::setFileBrowserFilter(char* name, char* filter)
{
	return setFileBrowserFilter(NAME_TO_ID(name), filter);
}

void MyHMI::setFileBrowserFilter(FileBrowser* id, char* filter)
{
	return setFileBrowserFilter(id->getObj(), filter);
}

void MyHMI::deleteFileBrowserFilter(lv_obj_t* id, int filterIndex)
{
	if (!_ISOBJ(id, HMI_FB)) return;
	((HMI_FileBrowser_Data*)get_list_user_data(id))->_fb->removeFilter(filterIndex);
}

void MyHMI::deleteFileBrowserFilter(int sn, int filterIndex)
{
	return deleteFileBrowserFilter(SN_TO_ID(sn), filterIndex);
}

void MyHMI::deleteFileBrowserFilter(char* name, int filterIndex)
{
	return deleteFileBrowserFilter(NAME_TO_ID(name), filterIndex);
}

void MyHMI::deleteFileBrowserFilter(FileBrowser* id, int filterIndex)
{
	return deleteFileBrowserFilter(id->getObj(), filterIndex);
}

void MyHMI::deleteFileBrowserFilter(lv_obj_t* id)
{
	deleteFileBrowserFilter(id, -1);
}

void MyHMI::deleteFileBrowserFilter(int sn)
{
	return deleteFileBrowserFilter(SN_TO_ID(sn));
}

void MyHMI::deleteFileBrowserFilter(char* name)
{
	return deleteFileBrowserFilter(NAME_TO_ID(name));
}

void MyHMI::deleteFileBrowserFilter(FileBrowser* id)
{
	return deleteFileBrowserFilter(id->getObj());
}

void MyHMI::setFileBrowserDisk(lv_obj_t* id, int disk)
{
	if (!_ISOBJ(id, HMI_FB)) return;
	((HMI_FileBrowser_Data*)get_list_user_data(id))->_fb->setDisk(disk);
}

void MyHMI::setFileBrowserDisk(int sn, int disk)
{
	return setFileBrowserDisk(SN_TO_ID(sn), disk);
}

void MyHMI::setFileBrowserDisk(char* name, int disk)
{
	return setFileBrowserDisk(NAME_TO_ID(name), disk);
}

void MyHMI::setFileBrowserDisk(FileBrowser* id, int disk)
{
	return setFileBrowserDisk(id->getObj(), disk);
}

void MyHMI::setTextListLine(lv_obj_t* id, int lineNumber, char* text)
{
	if (!_ISOBJ(id, HMI_TL)) return;
	((HMI_TextList_Data*)lv_obj_get_user_data(id))->_tl->print(lineNumber, text);
}

void MyHMI::setTextListLine(int sn, int lineNumber, char* text)
{
	return setTextListLine(SN_TO_ID(sn), lineNumber, text);
}

void MyHMI::setTextListLine(char* name, int lineNumber, char* text)
{
	return setTextListLine(NAME_TO_ID(name), lineNumber, text);
}

void MyHMI::setTextListLine(TextList* id, int lineNumber, char* text)
{
	return setTextListLine(id->getObj(), lineNumber, text);
}

void MyHMI::importToTextList(lv_obj_t* id, char* path)
{
	if (!_ISOBJ(id, HMI_TL)) return;
	((HMI_TextList_Data*)lv_obj_get_user_data(id))->_tl->openFile(path);
}

void MyHMI::importToTextList(int sn, char* path)
{
	return importToTextList(SN_TO_ID(sn), path);
}

void MyHMI::importToTextList(char* name, char* path)
{
	return importToTextList(NAME_TO_ID(name), path);
}

void MyHMI::importToTextList(TextList* id, char* path)
{
	return importToTextList(id->getObj(), path);
}

void MyHMI::importToTextList(lv_obj_t* id, char* path, int disk)
{
	if (!_ISOBJ(id, HMI_TL)) return;
	((HMI_TextList_Data*)lv_obj_get_user_data(id))->_tl->openFile(disk, path);
}

void MyHMI::importToTextList(int sn, char* path, int disk)
{
	return importToTextList(SN_TO_ID(sn), path, disk);
}

void MyHMI::importToTextList(char* name, char* path, int disk)
{
	return importToTextList(NAME_TO_ID(name), path, disk);
}

void MyHMI::importToTextList(TextList* id, char* path, int disk)
{
	return importToTextList(id->getObj(), path, disk);
}

void MyHMI::clearTextList(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_TL)) return;
	((HMI_TextList_Data*)lv_obj_get_user_data(id))->_tl->clear();
}

void MyHMI::clearTextList(int sn)
{
	return clearTextList(SN_TO_ID(sn));
}

void MyHMI::clearTextList(char* name)
{
	return clearTextList(NAME_TO_ID(name));
}

void MyHMI::clearTextList(TextList* id)
{
	return clearTextList(id->getObj());
}

void MyHMI::focusTextListLine(lv_obj_t* id, int lineNumber)
{
	if (!_ISOBJ(id, HMI_TL)) return;
	((HMI_TextList_Data*)lv_obj_get_user_data(id))->_tl->focus(lineNumber, true);
	((HMI_TextList_Data*)lv_obj_get_user_data(id))->_tl->highlight(lineNumber);
}

void MyHMI::focusTextListLine(int sn, int lineNumber)
{
	return focusTextListLine(SN_TO_ID(sn), lineNumber);
}

void MyHMI::focusTextListLine(char* name, int lineNumber)
{
	return focusTextListLine(NAME_TO_ID(name), lineNumber);
}

void MyHMI::focusTextListLine(TextList* id, int lineNumber)
{
	return focusTextListLine(id->getObj(), lineNumber);
}

void MyHMI::setTextListViewRange(lv_obj_t* id, int startLine, int endLine)
{
	if (!_ISOBJ(id, HMI_TL)) return;
	((HMI_TextList_Data*)lv_obj_get_user_data(id))->_tl->setViewRange(startLine, endLine);
}

void MyHMI::setTextListViewRange(int sn, int startLine, int endLine)
{
	return setTextListViewRange(SN_TO_ID(sn), startLine, endLine);
}

void MyHMI::setTextListViewRange(char* name, int startLine, int endLine)
{
	return setTextListViewRange(NAME_TO_ID(name), startLine, endLine);
}

void MyHMI::setTextListViewRange(TextList* id, int startLine, int endLine)
{
	return setTextListViewRange(id->getObj(), startLine, endLine);
}


/* States Function */
double MyHMI::getArcBarValue(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_ABAR)) return 0;
	HMI_Arc_Data* d = (HMI_Arc_Data*)lv_obj_get_user_data(id);
	return d->Value;
}

double MyHMI::getArcBarValue(int sn)
{
	return getArcBarValue(SN_TO_ID(sn));
}

double MyHMI::getArcBarValue(char* name)
{
	return getArcBarValue(NAME_TO_ID(name));
}

int MyHMI::getArcSliderValue(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_ASL)) return 0;
	return (int16_t)lv_arc_get_value(id);
}

int MyHMI::getArcSliderValue(int sn)
{
	return getArcSliderValue(SN_TO_ID(sn));
}

int MyHMI::getArcSliderValue(char* name)
{
	return getArcSliderValue(NAME_TO_ID(name));
}

double MyHMI::getBarValue(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_BAR)) return 0;
	HMI_Bar_Data* d = (HMI_Bar_Data*)lv_obj_get_user_data(id);
	return d->Value;
}

double MyHMI::getBarValue(int sn)
{
	return getBarValue(SN_TO_ID(sn));
}

double MyHMI::getBarValue(char* name)
{
	return getBarValue(NAME_TO_ID(name));
}

bool MyHMI::getToggleButtonState(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_TB)) return 0;
	return lv_btn_get_state(id) == LV_BTN_STATE_CHECKED_RELEASED || lv_btn_get_state(id) == LV_BTN_STATE_CHECKED_PRESSED || lv_btn_get_state(id) == LV_BTN_STATE_CHECKED_DISABLED;
}

bool MyHMI::getToggleButtonState(int sn)
{
	return getToggleButtonState(SN_TO_ID(sn));
}

bool MyHMI::getToggleButtonState(char* name)
{
	return getToggleButtonState(NAME_TO_ID(name));
}

bool MyHMI::getCheckboxState(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_CB)) return 0;
	return lv_checkbox_is_checked(id);
}

bool MyHMI::getCheckboxState(int sn)
{
	return getCheckboxState(SN_TO_ID(sn));
}

bool MyHMI::getCheckboxState(char* name)
{
	return getCheckboxState(NAME_TO_ID(name));
}

int MyHMI::getDropDownListSelectedItem(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_DDL)) return -1;
	return (int)lv_dropdown_get_selected(id);
}

int MyHMI::getDropDownListSelectedItem(int sn)
{
	return getDropDownListSelectedItem(SN_TO_ID(sn));
}

int MyHMI::getDropDownListSelectedItem(char* name)
{
	return getDropDownListSelectedItem(NAME_TO_ID(name));
}

int MyHMI::getGaugeValue(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_GA)) return 0;
	return lv_gauge_get_value(id, 0);
}

int MyHMI::getGaugeValue(int sn)
{
	return getGaugeValue(SN_TO_ID(sn));
}

int MyHMI::getGaugeValue(char* name)
{
	return getGaugeValue(NAME_TO_ID(name));
}

int MyHMI::getGaugeValue(lv_obj_t* id, int needle)
{
	if (!_ISOBJ(id, HMI_GA)) return 0;
	return lv_gauge_get_value(id, needle - 1);
}

int MyHMI::getGaugeValue(int sn, int needle)
{
	return getGaugeValue(SN_TO_ID(sn), needle);
}

int MyHMI::getGaugeValue(char* name, int needle)
{
	return getGaugeValue(NAME_TO_ID(name), needle);
}

int MyHMI::getImageX(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_IMG)) return 0;
	return (int)lv_obj_get_x(id);
}

int MyHMI::getImageX(int sn)
{
	return getImageX(SN_TO_ID(sn));
}

int MyHMI::getImageX(char* name)
{
	return getImageX(NAME_TO_ID(name));
}

int MyHMI::getImageY(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_IMG)) return 0;
	return (int)lv_obj_get_y(id);
}

int MyHMI::getImageY(int sn)
{
	return getImageY(SN_TO_ID(sn));
}

int MyHMI::getImageY(char* name)
{
	return getImageY(NAME_TO_ID(name));
}

double MyHMI::getNumberValue(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_NUM)) return 0;
	return ((HMI_Number_Data*)lv_obj_get_user_data(id))->Value;
}

double MyHMI::getNumberValue(int sn)
{
	return getNumberValue(SN_TO_ID(sn));
}

double MyHMI::getNumberValue(char* name)
{
	return getNumberValue(NAME_TO_ID(name));
}

int MyHMI::getListSelectedItem(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_LI)) return -1;
	lv_obj_t* btn = lv_list_get_btn_selected(id);
	if (btn == NULL) return -1;
	return (int)lv_list_get_btn_index(id, btn);
}

int MyHMI::getListSelectedItem(int sn)
{
	return getListSelectedItem(SN_TO_ID(sn));
}

int MyHMI::getListSelectedItem(char* name)
{
	return getListSelectedItem(NAME_TO_ID(name));
}

int MyHMI::getRollerSelectedItem(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_RO)) return -1;
	return (int)lv_roller_get_selected(id);
}

int MyHMI::getRollerSelectedItem(int sn)
{
	return getRollerSelectedItem(SN_TO_ID(sn));
}

int MyHMI::getRollerSelectedItem(char* name)
{
	return getRollerSelectedItem(NAME_TO_ID(name));
}

int MyHMI::getSliderValue(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_SL)) return 0;
	return (int)lv_slider_get_value(id);
}

int MyHMI::getSliderValue(int sn)
{
	return getSliderValue(SN_TO_ID(sn));
}

int MyHMI::getSliderValue(char* name)
{
	return getSliderValue(NAME_TO_ID(name));
}

double MyHMI::getSpinboxValue(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_SP)) return 0;
	HMI_Spinbox_Data* d = (HMI_Spinbox_Data*)lv_obj_get_user_data(id);
	return (double)lv_spinbox_get_value(lv_obj_get_child_back(id, NULL)) / pow(10, d->Decimal);
}

double MyHMI::getSpinboxValue(int sn)
{
	return getSpinboxValue(SN_TO_ID(sn));
}

double MyHMI::getSpinboxValue(char* name)
{
	return getSpinboxValue(NAME_TO_ID(name));
}

int MyHMI::getSpinboxInteger(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_SP)) return 0;
	HMI_Spinbox_Data* d = (HMI_Spinbox_Data*)lv_obj_get_user_data(id);
	return floor(lv_spinbox_get_value(lv_obj_get_child_back(id, NULL)) / pow(10, d->Decimal));
}

int MyHMI::getSpinboxInteger(int sn)
{
	return getSpinboxInteger(SN_TO_ID(sn));
}

int MyHMI::getSpinboxInteger(char* name)
{
	return getSpinboxInteger(NAME_TO_ID(name));
}

bool MyHMI::getSwitchState(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_SW)) return 0;
	return lv_switch_get_state(id);
}

bool MyHMI::getSwitchState(int sn)
{
	return getSwitchState(SN_TO_ID(sn));
}

bool MyHMI::getSwitchState(char* name)
{
	return getSwitchState(NAME_TO_ID(name));
}

char* MyHMI::getTextInputText(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_TIN)) return 0;
	HMI_TextInput_Data* d = (HMI_TextInput_Data*)lv_obj_get_user_data(id);
	strcpy(d->_buffer, lv_textarea_get_text(id));
	return d->_buffer;
}

char* MyHMI::getTextInputText(int sn)
{
	return getTextInputText(SN_TO_ID(sn));
}

char* MyHMI::getTextInputText(char* name)
{
	return getTextInputText(NAME_TO_ID(name));
}

bool MyHMI::getImageToggleButtonState(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_ITB)) return 0;
	return lv_imgbtn_get_state(id) == LV_BTN_STATE_CHECKED_RELEASED || lv_imgbtn_get_state(id) == LV_BTN_STATE_CHECKED_PRESSED || lv_imgbtn_get_state(id) == LV_BTN_STATE_CHECKED_DISABLED;
}

bool MyHMI::getImageToggleButtonState(int sn)
{
	return getImageToggleButtonState(SN_TO_ID(sn));
}

bool MyHMI::getImageToggleButtonState(char* name)
{
	return getImageToggleButtonState(NAME_TO_ID(name));
}

double MyHMI::getNumberInputValue(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_NIN)) return 0;
	return strtod(lv_textarea_get_text(id), NULL);
}

double MyHMI::getNumberInputValue(int sn)
{
	return getNumberInputValue(SN_TO_ID(sn));
}

double MyHMI::getNumberInputValue(char* name)
{
	return getNumberInputValue(NAME_TO_ID(name));
}

int MyHMI::getNumberInputInteger(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_NIN)) return 0;
	return floor(getNumberInputValue(id));
}

int MyHMI::getNumberInputInteger(int sn)
{
	return getNumberInputInteger(SN_TO_ID(sn));
}

int MyHMI::getNumberInputInteger(char* name)
{
	return getNumberInputInteger(NAME_TO_ID(name));
}

int MyHMI::getDateInputYear(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_DIN)) return 0;
	return ((HMI_DateInput_Data*)lv_obj_get_user_data(id))->DefaultYear;
}

int MyHMI::getDateInputYear(int sn)
{
	return getDateInputYear(SN_TO_ID(sn));
}

int MyHMI::getDateInputYear(char* name)
{
	return getDateInputYear(NAME_TO_ID(name));
}

int MyHMI::getDateInputMonth(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_DIN)) return 0;
	return ((HMI_DateInput_Data*)lv_obj_get_user_data(id))->DefaultMonth;
}

int MyHMI::getDateInputMonth(int sn)
{
	return getDateInputMonth(SN_TO_ID(sn));
}

int MyHMI::getDateInputMonth(char* name)
{
	return getDateInputMonth(NAME_TO_ID(name));
}

int MyHMI::getDateInputDay(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_DIN)) return 0;
	return ((HMI_DateInput_Data*)lv_obj_get_user_data(id))->DefaultDay;
}

int MyHMI::getDateInputDay(int sn)
{
	return getDateInputDay(SN_TO_ID(sn));
}

int MyHMI::getDateInputDay(char* name)
{
	return getDateInputDay(NAME_TO_ID(name));
}

double MyHMI::getImageBarValue(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_IBAR)) return 0;
	HMI_ImageBar_Data* d = (HMI_ImageBar_Data*)lv_obj_get_user_data(id);
	return d->Value;
}

double MyHMI::getImageBarValue(int sn)
{
	return getImageBarValue(SN_TO_ID(sn));
}

double MyHMI::getImageBarValue(char* name)
{
	return getImageBarValue(NAME_TO_ID(name));
}

int MyHMI::getImageSliderValue(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_ISL)) return 0;
	return (int16_t)lv_slider_get_value(id);
}

int MyHMI::getImageSliderValue(int sn)
{
	return getImageSliderValue(SN_TO_ID(sn));
}

int MyHMI::getImageSliderValue(char* name)
{
	return getImageSliderValue(NAME_TO_ID(name));
}

int MyHMI::getImageGaugeValue(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_IGA)) return 0;
	HMI_Gauge_Data* d = (HMI_Gauge_Data*)lv_obj_get_user_data(id);
	return d->NeedleValue1;
}

int MyHMI::getImageGaugeValue(int sn)
{
	return getImageGaugeValue(SN_TO_ID(sn));
}

int MyHMI::getImageGaugeValue(char* name)
{
	return getImageGaugeValue(NAME_TO_ID(name));
}

char* MyHMI::getFileBrowserFolder(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_FB)) return NULL;
	return (char*)((HMI_FileBrowser_Data*)get_list_user_data(id))->_fb->getCurrentFolderPath();
}

char* MyHMI::getFileBrowserFolder(int sn)
{
	return getFileBrowserFolder(SN_TO_ID(sn));
}

char* MyHMI::getFileBrowserFolder(char* name)
{
	return getFileBrowserFolder(NAME_TO_ID(name));
}

char* MyHMI::getFileBrowserFolder(FileBrowser* id)
{
	return getFileBrowserFolder(id->getObj());
}

char* MyHMI::getFileBrowserFilePath(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_FB)) return NULL;
	return (char*)((HMI_FileBrowser_Data*)get_list_user_data(id))->_fb->getSelectedFilePath();
}

char* MyHMI::getFileBrowserFilePath(int sn)
{
	return getFileBrowserFilePath(SN_TO_ID(sn));
}

char* MyHMI::getFileBrowserFilePath(char* name)
{
	return getFileBrowserFilePath(NAME_TO_ID(name));
}

char* MyHMI::getFileBrowserFilePath(FileBrowser* id)
{
	return getFileBrowserFilePath(id->getObj());
}

char* MyHMI::getTextListLine(lv_obj_t* id, int lineNumber)
{
	if (!_ISOBJ(id, HMI_TL)) return NULL;
	return (char*)((HMI_TextList_Data*)lv_obj_get_user_data(id))->_tl->getLine((uint32_t)lineNumber);
}

char* MyHMI::getTextListLine(int sn, int lineNumber)
{
	return getTextListLine(SN_TO_ID(sn), lineNumber);
}

char* MyHMI::getTextListLine(char* name, int lineNumber)
{
	return getTextListLine(NAME_TO_ID(name), lineNumber);
}

char* MyHMI::getTextListLine(TextList* id, int lineNumber)
{
	return getTextListLine(id->getObj(), lineNumber);
}

char* MyHMI::getTextListLine(lv_obj_t* id, int lineNumber, char* buf)
{
	if (!_ISOBJ(id, HMI_TL)) return NULL;
	return (char*)((HMI_TextList_Data*)lv_obj_get_user_data(id))->_tl->getLine((uint32_t)lineNumber, buf);
}

char* MyHMI::getTextListLine(int sn, int lineNumber, char* buf)
{
	return getTextListLine(SN_TO_ID(sn), lineNumber, buf);
}

char* MyHMI::getTextListLine(char* name, int lineNumber, char* buf)
{
	return getTextListLine(NAME_TO_ID(name), lineNumber, buf);
}

char* MyHMI::getTextListLine(TextList* id, int lineNumber, char* buf)
{
	return getTextListLine(id->getObj(), lineNumber, buf);
}

int MyHMI::getTextListViewRangeStart(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_TL)) return NULL;
	return (int)((HMI_TextList_Data*)lv_obj_get_user_data(id))->_tl->getViewRangeStart();
}

int MyHMI::getTextListViewRangeStart(int sn)
{
	return getTextListViewRangeStart(SN_TO_ID(sn));
}

int MyHMI::getTextListViewRangeStart(char* name)
{
	return getTextListViewRangeStart(NAME_TO_ID(name));
}

int MyHMI::getTextListViewRangeStart(TextList* id)
{
	return getTextListViewRangeStart(id->getObj());
}

int MyHMI::getTextListViewRangeEnd(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_TL)) return NULL;
	return (int)((HMI_TextList_Data*)lv_obj_get_user_data(id))->_tl->getViewRangeEnd();
}

int MyHMI::getTextListViewRangeEnd(int sn)
{
	return getTextListViewRangeEnd(SN_TO_ID(sn));
}

int MyHMI::getTextListViewRangeEnd(char* name)
{
	return getTextListViewRangeEnd(NAME_TO_ID(name));
}

int MyHMI::getTextListViewRangeEnd(TextList* id)
{
	return getTextListViewRangeEnd(id->getObj());
}


/* Events Function */
bool MyHMI::pageEnter(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_PAGE))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::pageEnter(int sn)
{
	return pageEnter(SN_TO_ID(sn));
}

bool MyHMI::pageEnter(char* name)
{
	return pageEnter(NAME_TO_ID(name));
}

bool MyHMI::pageLeave(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_PAGE))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 1)
			return true;
	return false;
}

bool MyHMI::pageLeave(int sn)
{
	return pageLeave(SN_TO_ID(sn));
}

bool MyHMI::pageLeave(char* name)
{
	return pageLeave(NAME_TO_ID(name));
}

bool MyHMI::pageTimerEventTriggered(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_PAGE))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 2)
			return true;
	return false;
}

bool MyHMI::pageTimerEventTriggered(int sn)
{
	return pageTimerEventTriggered(SN_TO_ID(sn));
}

bool MyHMI::pageTimerEventTriggered(char* name)
{
	return pageTimerEventTriggered(NAME_TO_ID(name));
}

bool MyHMI::arcSliderDragging(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_ASL))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::arcSliderDragging(int sn)
{
	return arcSliderDragging(SN_TO_ID(sn));
}

bool MyHMI::arcSliderDragging(char* name)
{
	return arcSliderDragging(NAME_TO_ID(name));
}

bool MyHMI::arcSliderDropped(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_ASL))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 1)
			return true;
	return false;
}

bool MyHMI::arcSliderDropped(int sn)
{
	return arcSliderDropped(SN_TO_ID(sn));
}

bool MyHMI::arcSliderDropped(char* name)
{
	return arcSliderDropped(NAME_TO_ID(name));
}

bool MyHMI::buttonClicked(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_B))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::buttonClicked(int sn)
{
	return buttonClicked(SN_TO_ID(sn));
}

bool MyHMI::buttonClicked(char* name)
{
	return buttonClicked(NAME_TO_ID(name));
}

bool MyHMI::buttonPressing(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_B))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 1)
			return true;
	return false;
}

bool MyHMI::buttonPressing(int sn)
{
	return buttonPressing(SN_TO_ID(sn));
}

bool MyHMI::buttonPressing(char* name)
{
	return buttonPressing(NAME_TO_ID(name));
}

bool MyHMI::toggleButtonChecked(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_TB))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::toggleButtonChecked(int sn)
{
	return toggleButtonChecked(SN_TO_ID(sn));
}

bool MyHMI::toggleButtonChecked(char* name)
{
	return toggleButtonChecked(NAME_TO_ID(name));
}

bool MyHMI::toggleButtonUnchecked(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_TB))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 1)
			return true;
	return false;
}

bool MyHMI::toggleButtonUnchecked(int sn)
{
	return toggleButtonUnchecked(SN_TO_ID(sn));
}

bool MyHMI::toggleButtonUnchecked(char* name)
{
	return toggleButtonUnchecked(NAME_TO_ID(name));
}

bool MyHMI::checkboxChecked(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_CB))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::checkboxChecked(int sn)
{
	return checkboxChecked(SN_TO_ID(sn));
}

bool MyHMI::checkboxChecked(char* name)
{
	return checkboxChecked(NAME_TO_ID(name));
}

bool MyHMI::checkboxUnchecked(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_CB))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 1)
			return true;
	return false;
}

bool MyHMI::checkboxUnchecked(int sn)
{
	return checkboxUnchecked(SN_TO_ID(sn));
}

bool MyHMI::checkboxUnchecked(char* name)
{
	return checkboxUnchecked(NAME_TO_ID(name));
}

bool MyHMI::dropDownListItemSelected(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_DDL))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::dropDownListItemSelected(int sn)
{
	return dropDownListItemSelected(SN_TO_ID(sn));
}

bool MyHMI::dropDownListItemSelected(char* name)
{
	return dropDownListItemSelected(NAME_TO_ID(name));
}

bool MyHMI::imageButtonClicked(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_IB))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::imageButtonClicked(int sn)
{
	return imageButtonClicked(SN_TO_ID(sn));
}

bool MyHMI::imageButtonClicked(char* name)
{
	return imageButtonClicked(NAME_TO_ID(name));
}

bool MyHMI::imageButtonPressing(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_IB))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 1)
			return true;
	return false;
}

bool MyHMI::imageButtonPressing(int sn)
{
	return imageButtonPressing(SN_TO_ID(sn));
}

bool MyHMI::imageButtonPressing(char* name)
{
	return imageButtonPressing(NAME_TO_ID(name));
}

bool MyHMI::listItemSelected(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_LI))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::listItemSelected(int sn)
{
	return listItemSelected(SN_TO_ID(sn));
}

bool MyHMI::listItemSelected(char* name)
{
	return listItemSelected(NAME_TO_ID(name));
}

bool MyHMI::rollerItemSelected(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_RO))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::rollerItemSelected(int sn)
{
	return rollerItemSelected(SN_TO_ID(sn));
}

bool MyHMI::rollerItemSelected(char* name)
{
	return rollerItemSelected(NAME_TO_ID(name));
}

bool MyHMI::sliderDragging(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_SL))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::sliderDragging(int sn)
{
	return sliderDragging(SN_TO_ID(sn));
}

bool MyHMI::sliderDragging(char* name)
{
	return sliderDragging(NAME_TO_ID(name));
}

bool MyHMI::sliderDropped(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_SL))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 1)
			return true;
	return false;
}

bool MyHMI::sliderDropped(int sn)
{
	return sliderDropped(SN_TO_ID(sn));
}

bool MyHMI::sliderDropped(char* name)
{
	return sliderDropped(NAME_TO_ID(name));
}

bool MyHMI::switchOn(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_SW))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::switchOn(int sn)
{
	return switchOn(SN_TO_ID(sn));
}

bool MyHMI::switchOn(char* name)
{
	return switchOn(NAME_TO_ID(name));
}

bool MyHMI::switchOff(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_SW))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 1)
			return true;
	return false;
}

bool MyHMI::switchOff(int sn)
{
	return switchOff(SN_TO_ID(sn));
}

bool MyHMI::switchOff(char* name)
{
	return switchOff(NAME_TO_ID(name));
}

bool MyHMI::textInputApplied(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_TIN))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::textInputApplied(int sn)
{
	return textInputApplied(SN_TO_ID(sn));
}

bool MyHMI::textInputApplied(char* name)
{
	return textInputApplied(NAME_TO_ID(name));
}

bool MyHMI::textInputCanceled(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_TIN))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 1)
			return true;
	return false;
}

bool MyHMI::textInputCanceled(int sn)
{
	return textInputCanceled(SN_TO_ID(sn));
}

bool MyHMI::textInputCanceled(char* name)
{
	return textInputCanceled(NAME_TO_ID(name));
}

bool MyHMI::imageToggleButtonChecked(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_ITB))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::imageToggleButtonChecked(int sn)
{
	return imageToggleButtonChecked(SN_TO_ID(sn));
}

bool MyHMI::imageToggleButtonChecked(char* name)
{
	return imageToggleButtonChecked(NAME_TO_ID(name));
}

bool MyHMI::imageToggleButtonUnchecked(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_ITB))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 1)
			return true;
	return false;
}

bool MyHMI::imageToggleButtonUnchecked(int sn)
{
	return imageToggleButtonUnchecked(SN_TO_ID(sn));
}

bool MyHMI::imageToggleButtonUnchecked(char* name)
{
	return imageToggleButtonUnchecked(NAME_TO_ID(name));
}

bool MyHMI::numberInputApplied(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_NIN))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::numberInputApplied(int sn)
{
	return numberInputApplied(SN_TO_ID(sn));
}

bool MyHMI::numberInputApplied(char* name)
{
	return numberInputApplied(NAME_TO_ID(name));
}

bool MyHMI::numberInputCanceled(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_NIN))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 1)
			return true;
	return false;
}

bool MyHMI::numberInputCanceled(int sn)
{
	return numberInputCanceled(SN_TO_ID(sn));
}

bool MyHMI::numberInputCanceled(char* name)
{
	return numberInputCanceled(NAME_TO_ID(name));
}

bool MyHMI::imageSliderDragging(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_ISL))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 0)
			return true;
	return false;
}

bool MyHMI::imageSliderDragging(int sn)
{
	return imageSliderDragging(SN_TO_ID(sn));
}

bool MyHMI::imageSliderDragging(char* name)
{
	return imageSliderDragging(NAME_TO_ID(name));
}

bool MyHMI::imageSliderDropped(lv_obj_t* id)
{
	if (!_ISOBJ(id, HMI_ISL))
		return false;
	if(popOutEvent != NULL)
		if(popOutEvent->obj == id && popOutEvent->eventId == 1)
			return true;
	return false;
}

bool MyHMI::imageSliderDropped(int sn)
{
	return imageSliderDropped(SN_TO_ID(sn));
}

bool MyHMI::imageSliderDropped(char* name)
{
	return imageSliderDropped(NAME_TO_ID(name));
}


bool MyHMI::eventPop()
{
	popOutEvent = eventQueuePop();
	return popOutEvent->obj != NULL;
}

void widgetsInit()
{
	themeFonts[0] = (lv_font_t*)malloc(sizeof(lv_font_t));
	lv_freetype_font_init(themeFonts[0], envPathCombine("IRSPATH", "Font/microhei.ttc"), 18);
	themeFonts[1] = (lv_font_t*)malloc(sizeof(lv_font_t));
	lv_freetype_font_init(themeFonts[1], envPathCombine("IRSPATH", "Font/microhei.ttc"), 34);
	themeFonts[2] = (lv_font_t*)malloc(sizeof(lv_font_t));
	lv_freetype_font_init(themeFonts[2], envPathCombine("IRSPATH", "Font/microhei.ttc"), 14);
	themeFonts[3] = (lv_font_t*)malloc(sizeof(lv_font_t));
	lv_freetype_font_init(themeFonts[3], envPathCombine("IRSPATH", "Font/microhei.ttc"), 18);
	themeFonts[4] = (lv_font_t*)malloc(sizeof(lv_font_t));
	lv_freetype_font_init(themeFonts[4], envPathCombine("IRSPATH", "Font/microhei.ttc"), 28);
	themeFonts[5] = &lv_font_montserrat_40;
	lv_theme_material86_init(themePalette, themeFonts);
	static HMI_SlidePage_Data p1_Data = {1, 500, true, "My Page p1"};
	static HMI_BasicObj_Data p1rect1_Data = {"My Rectangle p1rect1"};
	static HMI_Label_Data p1l1_Data = {"My Label p1l1", {"Primitive widgets", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Button_Data p1b1_Data = {"My Button p1b1", LV_SYMBOL_PREV, {"", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p1b2_Data = {"My Button p1b2", LV_SYMBOL_NEXT, {"", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_BasicObj_Data p1rect2_Data = {"My Rectangle p1rect2"};
	static HMI_BasicObj_Data p1rect3_Data = {"My Rectangle p1rect3"};
	static HMI_BasicObj_Data p1rect4_Data = {"My Rectangle p1rect4"};
	static HMI_BasicObj_Data p1rect5_Data = {"My Rectangle p1rect5"};
	static HMI_BasicObj_Data p1rect6_Data = {"My Rectangle p1rect6"};
	static HMI_BasicObj_Data p1rect7_Data = {"My Rectangle p1rect7"};
	static HMI_Label_Data p1l2_Data = {"My Label p1l2", {"Label", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p1l3_Data = {"My Label p1l3", {"Number", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p1l4_Data = {"My Label p1l4", {"Rectangle", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p1l5_Data = {"My Label p1l5", {"Circle", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p1l6_Data = {"My Label p1l6", {"Image", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p1l7_Data = {"My Label p1l7", {"QrCode", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p1l8_Data = {"My Label p1l8", {"Regular size", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p1l9_Data = {"My Label p1l9", {"Large size", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Number_Data p1num1_Data = {"My Number p1num1", {"", "", "", "", "", "", "", "", ""}, {" °C", "", "", "", "", "", "", "", ""}, 0, 100};
	static HMI_Number_Data p1num2_Data = {"My Number p1num2", {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}, 6, 3.141592};
	static HMI_BasicObj_Data p1rect8_Data = {"My Rectangle p1rect8"};
	static HMI_BasicObj_Data p1cir1_Data = {"My Circle p1cir1"};
	static HMI_BasicObj_Data p1qr1_Data = {"My QrCode p1qr1"};
	static HMI_BasicObj_Data p1rect9_Data = {"My Rectangle p1rect9"};
	static HMI_BasicObj_Data p1rect10_Data = {"My Rectangle p1rect10"};
	static HMI_BasicObj_Data p1cir2_Data = {"My Circle p1cir2"};
	static HMI_BasicObj_Data p1cir3_Data = {"My Circle p1cir3"};
	static HMI_Image_Data p1img1_Data = {"My Image p1img1", envPathCombine("UIRSPATH", "MyHMI/Image/limg3.png")};
	static HMI_SlidePage_Data p2_Data = {0, 100, false, "My Page p2"};
	static HMI_BasicObj_Data p2rect1_Data = {"My Rectangle p2rect1"};
	static HMI_Label_Data p2l1_Data = {"My Label p2l1", {"Control widgets (1/2)", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Button_Data p2b1_Data = {"My Button p2b1", LV_SYMBOL_PREV, {"", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p2b2_Data = {"My Button p2b2", LV_SYMBOL_NEXT, {"", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_BasicObj_Data p2rect2_Data = {"My Rectangle p2rect2"};
	static HMI_BasicObj_Data p2rect3_Data = {"My Rectangle p2rect3"};
	static HMI_BasicObj_Data p2rect4_Data = {"My Rectangle p2rect4"};
	static HMI_BasicObj_Data p2rect5_Data = {"My Rectangle p2rect5"};
	static HMI_Label_Data p2l2_Data = {"My Label p2l2", {"ImageToggleButton", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p2l5_Data = {"My Label p2l5", {"ImageButton", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p2l6_Data = {"My Label p2l6", {"ToggleButton", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p2l7_Data = {"My Label p2l7", {"Button", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Button_Data p2b3_Data = {"My Button p2b3", "", {"Button", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_ToggleButton_Data p2tb1_Data = {"My ToggleButton p2tb1", "", {"Unchecked", "OFF2", "OFF3", "OFF4", "OFF5", "OFF6", "OFF7", "OFF8", "OFF9"}, {"Checked", "ON2", "ON3", "ON4", "ON5", "ON6", "ON7", "ON8", "ON9"}};
	static HMI_ImageButton_Data p2ib1_Data = {"My ImageButton p2ib1", NULL};
	static HMI_ImageToggleButton_Data p2itb1_Data = {"My ImageToggleButton p2itb1", NULL, NULL};
	static HMI_Button_Data p2b4_Data = {"My Button p2b4", LV_SYMBOL_POWER, {"Power", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_ToggleButton_Data p2tb2_Data = {"My ToggleButton p2tb2", LV_SYMBOL_POWER, {"Power OFF", "OFF2", "OFF3", "OFF4", "OFF5", "OFF6", "OFF7", "OFF8", "OFF9"}, {"Power ON", "ON2", "ON3", "ON4", "ON5", "ON6", "ON7", "ON8", "ON9"}};
	static HMI_ImageButton_Data p2ib2_Data = {"My ImageButton p2ib2", envPathCombine("UIRSPATH", "MyHMI/Image/limg1.png")};
	static HMI_ImageToggleButton_Data p2itb2_Data = {"My ImageToggleButton p2itb2", envPathCombine("UIRSPATH", "MyHMI/Image/limg1.png"), envPathCombine("UIRSPATH", "MyHMI/Image/limg2.png")};
	static HMI_Button_Data p2b5_Data = {"My Button p2b5", LV_SYMBOL_SETTINGS, {"Config", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p2b6_Data = {"My Button p2b6", LV_SYMBOL_OK, {"OK", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_ToggleButton_Data p2tb3_Data = {"My ToggleButton p2tb3", LV_SYMBOL_CHARGE, {"Discharge", "OFF2", "OFF3", "OFF4", "OFF5", "OFF6", "OFF7", "OFF8", "OFF9"}, {"Charge", "ON2", "ON3", "ON4", "ON5", "ON6", "ON7", "ON8", "ON9"}};
	static HMI_ToggleButton_Data p2tb4_Data = {"My ToggleButton p2tb4", LV_SYMBOL_WIFI, {"Disconnect", "OFF2", "OFF3", "OFF4", "OFF5", "OFF6", "OFF7", "OFF8", "OFF9"}, {"Connect", "ON2", "ON3", "ON4", "ON5", "ON6", "ON7", "ON8", "ON9"}};
	static HMI_ImageToggleButton_Data p2itb3_Data = {"My ImageToggleButton p2itb3", NULL, NULL};
	static HMI_ImageToggleButton_Data p2itb4_Data = {"My ImageToggleButton p2itb4", envPathCombine("UIRSPATH", "MyHMI/Image/limg1.png"), envPathCombine("UIRSPATH", "MyHMI/Image/limg2.png")};
	static HMI_ImageButton_Data p2ib3_Data = {"My ImageButton p2ib3", NULL};
	static HMI_ImageButton_Data p2ib4_Data = {"My ImageButton p2ib4", envPathCombine("UIRSPATH", "MyHMI/Image/limg2.png")};
	static HMI_SlidePage_Data p3_Data = {0, 100, false, "My Page p3"};
	static HMI_BasicObj_Data p3rect1_Data = {"My Rectangle p3rect1"};
	static HMI_Label_Data p3l1_Data = {"My Label p3l1", {"Control widgets (2/2)", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Button_Data p3b1_Data = {"My Button p3b1", LV_SYMBOL_PREV, {"", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p3b2_Data = {"My Button p3b2", LV_SYMBOL_NEXT, {"", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_BasicObj_Data p3rect2_Data = {"My Rectangle p3rect2"};
	static HMI_BasicObj_Data p3rect3_Data = {"My Rectangle p3rect3"};
	static HMI_BasicObj_Data p3rect9_Data = {"My Rectangle p3rect9"};
	static HMI_Label_Data p3l9_Data = {"My Label p3l9", {"ImageSlider", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_BasicObj_Data p3rect5_Data = {"My Rectangle p3rect5"};
	static HMI_BasicObj_Data p3rect8_Data = {"My Rectangle p3rect8"};
	static HMI_Label_Data p3l8_Data = {"My Label p3l8", {"Slider", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p3l2_Data = {"My Label p3l2", {"ArcSlider", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p3l6_Data = {"My Label p3l6", {"Checkbox", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p3l7_Data = {"My Label p3l7", {"Switch", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Slider_Data p3sl2_Data = {"My Slider p3sl2", 5, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Slider_Data p3sl3_Data = {"My Slider p3sl3", 3, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_ImageSlider_Data p3isl1_Data = {"My ImageSlider p3isl1", NULL, NULL, NULL, 0};
	static HMI_Arc_Data p3asl1_Data = {"My ArcSlider p3asl1", 0, 100, 0, 20, 0, {"", "", "", "", "", "", "", "", ""}, 2};
	static HMI_Arc_Data p3asl2_Data = {"My ArcSlider p3asl2", 0, 100, 0, 50, 1, {"", "", "", "", "", "", "", "", ""}, 2};
	static HMI_BasicObj_Data p3sw1_Data = {"My Switch p3sw1"};
	static HMI_BasicObj_Data p3sw2_Data = {"My Switch p3sw2"};
	static HMI_Checkbox_Data p3cb1_Data = {"My Checkbox p3cb1", {"Check box1", "Check box2", "Check box3", "Check box4", "Check box5", "Check box6", "Check box7", "Check box8", "Check box9"}};
	static HMI_Checkbox_Data p3cb2_Data = {"My Checkbox p3cb2", {"Check box2", "Check box2", "Check box3", "Check box4", "Check box5", "Check box6", "Check box7", "Check box8", "Check box9"}};
	static HMI_Checkbox_Data p3cb3_Data = {"My Checkbox p3cb3", {"Check box3", "Check box2", "Check box3", "Check box4", "Check box5", "Check box6", "Check box7", "Check box8", "Check box9"}};
	static HMI_Checkbox_Data p3cb4_Data = {"My Checkbox p3cb4", {"Check box4", "Check box2", "Check box3", "Check box4", "Check box5", "Check box6", "Check box7", "Check box8", "Check box9"}};
	static HMI_BasicObj_Data p3sw3_Data = {"My Switch p3sw3"};
	static HMI_BasicObj_Data p3sw4_Data = {"My Switch p3sw4"};
	static HMI_BasicObj_Data p3sw5_Data = {"My Switch p3sw5"};
	static HMI_BasicObj_Data p3sw6_Data = {"My Switch p3sw6"};
	static HMI_BasicObj_Data p3sw7_Data = {"My Switch p3sw7"};
	static HMI_BasicObj_Data p3sw8_Data = {"My Switch p3sw8"};
	static HMI_SlidePage_Data p4_Data = {1, 50, true, "My Page p4"};
	static HMI_BasicObj_Data p4rect1_Data = {"My Rectangle p4rect1"};
	static HMI_Label_Data p4l1_Data = {"My Label p4l1", {"Visualization widgets (1/2)", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Button_Data p4b1_Data = {"My Button p4b1", LV_SYMBOL_PREV, {"", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p4b2_Data = {"My Button p4b2", LV_SYMBOL_NEXT, {"", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_BasicObj_Data p4rect2_Data = {"My Rectangle p4rect2"};
	static HMI_BasicObj_Data p4rect3_Data = {"My Rectangle p4rect3"};
	static HMI_BasicObj_Data p4rect5_Data = {"My Rectangle p4rect5"};
	static HMI_BasicObj_Data p4rect6_Data = {"My Rectangle p4rect6"};
	static HMI_BasicObj_Data p4rect7_Data = {"My Rectangle p4rect7"};
	static HMI_Label_Data p4l2_Data = {"My Label p4l2", {"Gauge", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p4l3_Data = {"My Label p4l3", {"ImageGauge", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p4l4_Data = {"My Label p4l4", {"LED", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p4l6_Data = {"My Label p4l6", {"ArcBar", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p4l7_Data = {"My Label p4l7", {"Bar", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_BasicObj_Data p4rect8_Data = {"My Rectangle p4rect8"};
	static HMI_Label_Data p4l8_Data = {"My Label p4l8", {"ImageBar", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_ImageBar_Data p4ibar1_Data = {"My ImageBar p4ibar1", 20, 0, 14, NULL, NULL, 8};
	static HMI_Bar_Data p4bar2_Data = {"My Bar p4bar2", 0, 100, 0, 50, 1, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Bar_Data p4bar3_Data = {"My Bar p4bar3", 0, 100, 0, 80, 3, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Gauge_Data p4ga1_Data = {"My Gauge p4ga1", 0, 100, 20, 30, -1, -1, -1, -1};
	static HMI_ImageGauge_Data p4iga1_Data = {"My ImageGauge p4iga1", 0, NULL};
	static HMI_BasicObj_Data p4led1_Data = {"My LED p4led1"};
	static HMI_BasicObj_Data p4led2_Data = {"My LED p4led2"};
	static HMI_BasicObj_Data p4led3_Data = {"My LED p4led3"};
	static HMI_BasicObj_Data p4led4_Data = {"My LED p4led4"};
	static HMI_Arc_Data p4abar1_Data = {"My ArcBar p4abar1", 0, 100, 0, 0, 0, {"", "", "", "", "", "", "", "", ""}, 2};
	static HMI_Arc_Data p4abar2_Data = {"My ArcBar p4abar2", 0, 100, 0, 0, 1, {"", "", "", "", "", "", "", "", ""}, 2};
	static HMI_SlidePage_Data p5_Data = {1, 10, true, "My Page p5"};
	static HMI_BasicObj_Data p5rect1_Data = {"My Rectangle p5rect1"};
	static HMI_Label_Data p5l1_Data = {"My Label p5l1", {"Visualization widgets (2/2)", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Button_Data p5b1_Data = {"My Button p5b1", LV_SYMBOL_PREV, {"", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p5b2_Data = {"My Button p5b2", LV_SYMBOL_NEXT, {"", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_BasicObj_Data p5rect2_Data = {"My Rectangle p5rect2"};
	static HMI_BasicObj_Data p5rect3_Data = {"My Rectangle p5rect3"};
	static HMI_BasicObj_Data p5rect5_Data = {"My Rectangle p5rect5"};
	static HMI_BasicObj_Data p5rect8_Data = {"My Rectangle p5rect8"};
	static HMI_Label_Data p5l2_Data = {"My Label p5l2", {"Chart", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p5l6_Data = {"My Label p5l6", {"TextList", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p5l7_Data = {"My Label p5l7", {"Table", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_TextList_Data p5tl1_Data = {"My TextList p5tl1"};
	static HMI_Table_Data p5tbl1_Data = {"My Table p5tbl1", 81, 72, 72, 72, 72, 56, 39, 39, 39, 39, 130, -1, -1, 1, {",\nSpring,\nSummer,\nAutumn,\nWinter", "(Auto)", "(Auto)", "(Auto)", "(Auto)", "(Auto)", "(Auto)", "(Auto)", "(Auto)"}, -1, -1, 1, {",\nApple,\nBanana,\nGuava,\nOrange", "(Auto)", "(Auto)", "(Auto)", "(Auto)", "(Auto)", "(Auto)", "(Auto)", "(Auto)"}, 5};
	static HMI_Chart_Data p5ch1_Data = {"My Chart p5ch1", 11, 100, 0, 1, -1, -1, -1, -1, 0, 0, 1, {"0\n5\n10", "0\n2.5\n5\n7.5\n10", "0\n2.5\n5\n7.5\n10", "0\n2.5\n5\n7.5\n10", "0\n2.5\n5\n7.5\n10", "0\n2.5\n5\n7.5\n10", "0\n2.5\n5\n7.5\n10", "0\n2.5\n5\n7.5\n10", "0\n2.5\n5\n7.5\n10"}, {"100\n75\n50\n25\n0", "100\n75\n50\n25\n0", "100\n75\n50\n25\n0", "100\n75\n50\n25\n0", "100\n75\n50\n25\n0", "100\n75\n50\n25\n0", "100\n75\n50\n25\n0", "100\n75\n50\n25\n0", "100\n75\n50\n25\n0"}};
	static HMI_Oscilloscope_Data p5osc1_Data = {"My Oscilloscope p5osc1", 1, 10000, 10000, 250000, -1, 0, 0, 10, -10};
	static HMI_Label_Data p5l8_Data = {"My Label p5l8", {"Oscilloscope", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_SlidePage_Data p6_Data = {0, 100, false, "My Page p6"};
	static HMI_BasicObj_Data p6rect1_Data = {"My Rectangle p6rect1"};
	static HMI_Label_Data p6l1_Data = {"My Label p6l1", {"Input widgets", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Button_Data p6b1_Data = {"My Button p6b1", LV_SYMBOL_PREV, {"", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p6b2_Data = {"My Button p6b2", LV_SYMBOL_NEXT, {"", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_BasicObj_Data p6rect2_Data = {"My Rectangle p6rect2"};
	static HMI_Label_Data p6l2_Data = {"My Label p6l2", {"TextInput", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_BasicObj_Data p6rect3_Data = {"My Rectangle p6rect3"};
	static HMI_Label_Data p6l3_Data = {"My Label p6l3", {"NumberInput", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_TextInput_Data p6tin1_Data = {"My TextInput p6tin1"};
	static HMI_NumberInput_Data p6nin1_Data = {"My NumberInput p6nin1", 0};
	static HMI_NumberInput_Data p6nin2_Data = {"My NumberInput p6nin2", 1};
	static HMI_BasicObj_Data p6rect4_Data = {"My Rectangle p6rect4"};
	static HMI_BasicObj_Data p6rect5_Data = {"My Rectangle p6rect5"};
	static HMI_Label_Data p6l4_Data = {"My Label p6l4", {"DateInput", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p6l5_Data = {"My Label p6l5", {"Spinbox", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_TextInput_Data p6tin2_Data = {"My TextInput p6tin2"};
	static HMI_DateInput_Data p6din1_Data = {"My DateInput p6din1", 0, 2023, 10, 23};
	static HMI_DateInput_Data p6din2_Data = {"My DateInput p6din2", 4, 2023, 10, 23};
	static HMI_Spinbox_Data p6sp2_Data = {"My Spinbox p6sp2", 3, 0};
	static HMI_Spinbox_Data p6sp4_Data = {"My Spinbox p6sp4", 5, 3};
	static HMI_SlidePage_Data p7_Data = {0, 100, false, "My Page p7"};
	static HMI_BasicObj_Data p7rect1_Data = {"My Rectangle p7rect1"};
	static HMI_Label_Data p7l1_Data = {"My Label p7l1", {"Menu widgets", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Button_Data p7b1_Data = {"My Button p7b1", LV_SYMBOL_PREV, {"", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p7b2_Data = {"My Button p7b2", LV_SYMBOL_NEXT, {"", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_BasicObj_Data p7rect2_Data = {"My Rectangle p7rect2"};
	static HMI_Label_Data p7l2_Data = {"My Label p7l2", {"DropDownList", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_BasicObj_Data p7rect3_Data = {"My Rectangle p7rect3"};
	static HMI_Label_Data p7l3_Data = {"My Label p7l3", {"List", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_BasicObj_Data p7rect4_Data = {"My Rectangle p7rect4"};
	static HMI_Label_Data p7l4_Data = {"My Label p7l4", {"Roller", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_DropDownList_Data p7ddl1_Data = {"My DropDownList p7ddl1", {"Item A\nItem B\nItem C", "Item A2\nItem B2\n", "Item A3\nItem B3\n", "Item A4\nItem B4\n", "Item A5\nItem B5\n", "Item A6\nItem B6\n", "Item A7\nItem B7\n", "Item A8\nItem B8\n", "Item A9\nItem B9\n"}};
	static HMI_DropDownList_Data p7ddl2_Data = {"My DropDownList p7ddl2", {"Apple\nBanana\nGuava\nOrange", "Item A2\nItem B2\n\n", "Item A3\nItem B3\n\n", "Item A4\nItem B4\n\n", "Item A5\nItem B5\n\n", "Item A6\nItem B6\n\n", "Item A7\nItem B7\n\n", "Item A8\nItem B8\n\n", "Item A9\nItem B9\n\n"}};
	static HMI_List_Data p7li1_Data = {"My List p7li1", 3, {"Item A", "Item B", "Item C", "Item D", "Item E", "Item F", "Item G", "Item H", "Item I", "Item J", "Item K", "Item L", "Item M", "Item N", "Item O", "Item P", "Item A2", "Item B2", "Item C2", "Item D2", "Item E2", "Item F2", "Item G2", "Item H2", "Item I2", "Item J2", "Item K2", "Item L2", "Item M2", "Item N2", "Item O2", "Item P2", "Item A3", "Item B3", "Item C3", "Item D3", "Item E3", "Item F3", "Item G3", "Item H3", "Item I3", "Item J3", "Item K3", "Item L3", "Item M3", "Item N3", "Item O3", "Item P3", "Item A4", "Item B4", "Item C4", "Item D4", "Item E4", "Item F4", "Item G4", "Item H4", "Item I4", "Item J4", "Item K4", "Item L4", "Item M4", "Item N4", "Item O4", "Item P4", "Item A5", "Item B5", "Item C5", "Item D5", "Item E5", "Item F5", "Item G5", "Item H5", "Item I5", "Item J5", "Item K5", "Item L5", "Item M5", "Item N5", "Item O5", "Item P5", "Item A6", "Item B6", "Item C6", "Item D6", "Item E6", "Item F6", "Item G6", "Item H6", "Item I6", "Item J6", "Item K6", "Item L6", "Item M6", "Item N6", "Item O6", "Item P6", "Item A7", "Item B7", "Item C7", "Item D7", "Item E7", "Item F7", "Item G7", "Item H7", "Item I7", "Item J7", "Item K7", "Item L7", "Item M7", "Item N7", "Item O7", "Item P7", "Item A8", "Item B8", "Item C8", "Item D8", "Item E8", "Item F8", "Item G8", "Item H8", "Item I8", "Item J8", "Item K8", "Item L8", "Item M8", "Item N8", "Item O8", "Item P8", "Item A9", "Item B9", "Item C9", "Item D9", "Item E9", "Item F9", "Item G9", "Item H9", "Item I9", "Item J9", "Item K9", "Item L9", "Item M9", "Item N9", "Item O9", "Item P9"}, {LV_SYMBOL_AUDIO, LV_SYMBOL_VIDEO, LV_SYMBOL_LIST, LV_SYMBOL_OK, LV_SYMBOL_CLOSE, LV_SYMBOL_POWER, LV_SYMBOL_SETTINGS, LV_SYMBOL_HOME, LV_SYMBOL_DOWNLOAD, LV_SYMBOL_DRIVE, LV_SYMBOL_REFRESH, LV_SYMBOL_MUTE, LV_SYMBOL_VOLUME_MID, LV_SYMBOL_VOLUME_MAX, LV_SYMBOL_IMAGE, LV_SYMBOL_EDIT}};
	static HMI_Roller_Data p7ro1_Data = {"My Roller p7ro1", {"Item A\nItem B", "Item A2\nItem B2", "Item A3\nItem B3", "Item A4\nItem B4", "Item A5\nItem B5", "Item A6\nItem B6", "Item A7\nItem B7", "Item A8\nItem B8", "Item A9\nItem B9"}, 0};
	static HMI_Roller_Data p7ro2_Data = {"My Roller p7ro2", {"Apple\nBanana\nGuava\nOrange", "Item A2\nItem B2\n\n", "Item A3\nItem B3\n\n", "Item A4\nItem B4\n\n", "Item A5\nItem B5\n\n", "Item A6\nItem B6\n\n", "Item A7\nItem B7\n\n", "Item A8\nItem B8\n\n", "Item A9\nItem B9\n\n"}, 1};
	static HMI_BasicObj_Data p7rect6_Data = {"My Rectangle p7rect6"};
	static HMI_Label_Data p7l6_Data = {"My Label p7l6", {"FileBrowser", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_FileBrowser_Data p7fb1_Data = {"My FileBrowser p7fb1"};
	tileview = lv_tileview_create(lv_scr_act(), NULL);
	lv_page_set_scrlbar_mode(tileview, LV_SCRLBAR_MODE_OFF);
	lv_obj_set_style_local_bg_opa(tileview, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
	lv_obj_set_style_local_border_width(tileview, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 0);
	p1 = SlidePageInit(tileview, NULL, &p1_Data);
	lv_obj_set_pos(p1, LV_HOR_RES * 0, 0);
	p1rect1 = RectangleInit(p1, -10, -10, 820, 80, -2, false, 28, -1, 100, &p1rect1_Data);
	p1l1 = LabelInit(p1, -10, 14, 820, 44, -2, true, 27, 2, -1, 5, &p1l1_Data);
	p1b1 = ButtonInit(p1, 0, 0, 80, 70, -1, true, 26, 2, -1, 5, &p1b1_Data);
	p1b2 = ButtonInit(p1, 720, 0, 80, 70, -1, true, 25, 2, -1, 5, &p1b2_Data);
	p1rect2 = RectangleInit(p1, 10, 80, 250, 190, -2, false, 24, -1, 100, &p1rect2_Data);
	p1rect3 = RectangleInit(p1, 275, 80, 250, 190, -2, false, 23, -1, 100, &p1rect3_Data);
	p1rect4 = RectangleInit(p1, 542, 81, 250, 190, -2, false, 22, -1, 100, &p1rect4_Data);
	p1rect5 = RectangleInit(p1, 10, 280, 250, 190, -2, false, 21, -1, 100, &p1rect5_Data);
	p1rect6 = RectangleInit(p1, 275, 280, 250, 190, -2, false, 20, -1, 100, &p1rect6_Data);
	p1rect7 = RectangleInit(p1, 540, 280, 250, 190, -2, false, 19, -1, 100, &p1rect7_Data);
	p1l2 = LabelInit(p1, 20, 85, 45, 21, -2, true, 18, 0, 0x9A9A9A, -1, &p1l2_Data);
	p1l3 = LabelInit(p1, 285, 85, 68, 21, -2, true, 17, 0, 0x9A9A9A, -1, &p1l3_Data);
	p1l4 = LabelInit(p1, 550, 85, 81, 21, -2, true, 16, 0, 0x9A9A9A, -1, &p1l4_Data);
	p1l5 = LabelInit(p1, 20, 285, 46, 21, -2, true, 15, 0, 0x9A9A9A, -1, &p1l5_Data);
	p1l6 = LabelInit(p1, 285, 285, 52, 21, -2, true, 14, 0, 0x9A9A9A, -1, &p1l6_Data);
	p1l7 = LabelInit(p1, 550, 285, 62, 21, -2, true, 13, 0, 0x9A9A9A, -1, &p1l7_Data);
	p1l8 = LabelInit(p1, 85, 135, 101, 21, -2, true, 12, 0, -1, 0, &p1l8_Data);
	p1l9 = LabelInit(p1, 60, 182, 151, 40, -2, true, 11, 0, -1, 1, &p1l9_Data);
	p1num1 = NumberInit(p1, 373, 135, 54, 21, -2, true, 10, 0, -1, -1, &p1num1_Data);
	p1num2 = NumberInit(p1, 329, 182, 142, 40, -2, true, 9, 0, -1, 1, &p1num2_Data);
	p1rect8 = RectangleInit(p1, 621, 125, 100, 80, -2, false, 8, 0xFF0000, 50, &p1rect8_Data);
	p1cir1 = CircleInit(p1, 84, 304, 100, 100, -2, false, 7, 0xFF0000, 50, &p1cir1_Data);
	p1qr1 = QrCodeInit(p1, 595, 313, 140, 140, -2, true, 6, "https://www.icop.com.tw/", &p1qr1_Data);
	p1rect9 = RectangleInit(p1, 648, 155, 100, 80, -2, false, 5, 0x0000FF, 50, &p1rect9_Data);
	p1rect10 = RectangleInit(p1, 589, 165, 100, 80, -2, false, 4, 0x00FF00, 50, &p1rect10_Data);
	p1cir2 = CircleInit(p1, 53, 355, 100, 100, -2, false, 3, 0x00FF00, 50, &p1cir2_Data);
	p1cir3 = CircleInit(p1, 116, 355, 100, 100, -2, false, 2, 0x0000FF, 50, &p1cir3_Data);
	p1img1 = ImageInit(p1, 300, 275, 200, 200, -2, true, 1, 100, false, &p1img1_Data);

	p2 = SlidePageInit(tileview, NULL, &p2_Data);
	lv_obj_set_pos(p2, LV_HOR_RES * 1, 0);
	p2rect1 = RectangleInit(p2, -10, -10, 820, 80, -2, false, 28, -1, 100, &p2rect1_Data);
	p2l1 = LabelInit(p2, -10, 14, 820, 44, -2, true, 27, 2, -1, 5, &p2l1_Data);
	p2b1 = ButtonInit(p2, 0, 0, 80, 70, -1, true, 26, 2, -1, 5, &p2b1_Data);
	p2b2 = ButtonInit(p2, 720, 0, 80, 70, -1, true, 25, 2, -1, 5, &p2b2_Data);
	p2rect2 = RectangleInit(p2, 10, 80, 385, 190, -2, false, 24, -1, 100, &p2rect2_Data);
	p2rect3 = RectangleInit(p2, 405, 80, 385, 190, -2, false, 23, -1, 100, &p2rect3_Data);
	p2rect4 = RectangleInit(p2, 10, 280, 385, 190, -2, false, 22, -1, 100, &p2rect4_Data);
	p2rect5 = RectangleInit(p2, 405, 280, 385, 190, -2, false, 21, -1, 100, &p2rect5_Data);
	p2l2 = LabelInit(p2, 415, 285, 161, 21, -2, true, 20, 0, 0x9A9A9A, -1, &p2l2_Data);
	p2l5 = LabelInit(p2, 20, 285, 107, 21, -2, true, 19, 0, 0x9A9A9A, -1, &p2l5_Data);
	p2l6 = LabelInit(p2, 415, 85, 109, 21, -2, true, 18, 0, 0x9A9A9A, -1, &p2l6_Data);
	p2l7 = LabelInit(p2, 20, 85, 55, 21, -2, true, 17, 0, 0x9A9A9A, -1, &p2l7_Data);
	p2b3 = ButtonInit(p2, 55, 125, 130, 43, 0, true, 16, 0, -1, -1, &p2b3_Data);
	p2tb1 = ToggleButtonInit(p2, 455, 125, 130, 43, 0, true, 15, -1, -1, -1, true, &p2tb1_Data);
	p2ib1 = ImageButtonInit(p2, 46, 348, 99, 67, -2, true, 14, 0, 100, &p2ib1_Data);
	p2itb1 = ImageToggleButtonInit(p2, 466, 335, 50, 97, -2, true, 13, 0, 100, true, &p2itb1_Data);
	p2b4 = ButtonInit(p2, 55, 200, 130, 43, 1, true, 12, 1, -1, -1, &p2b4_Data);
	p2tb2 = ToggleButtonInit(p2, 455, 200, 130, 43, 1, true, 11, -1, -1, -1, false, &p2tb2_Data);
	p2ib2 = ImageButtonInit(p2, 312, 333, 35, 35, -2, true, 10, 0, 100, &p2ib2_Data);
	p2itb2 = ImageToggleButtonInit(p2, 702, 333, 35, 35, -2, true, 9, 0, 100, false, &p2itb2_Data);
	p2b5 = ButtonInit(p2, 220, 125, 130, 43, 2, true, 8, 0, -1, -1, &p2b5_Data);
	p2b6 = ButtonInit(p2, 220, 200, 130, 43, 3, true, 7, 1, -1, -1, &p2b6_Data);
	p2tb3 = ToggleButtonInit(p2, 620, 125, 130, 43, 2, true, 6, -1, -1, -1, true, &p2tb3_Data);
	p2tb4 = ToggleButtonInit(p2, 620, 200, 130, 43, 3, true, 5, -1, -1, -1, false, &p2tb4_Data);
	p2itb3 = ImageToggleButtonInit(p2, 580, 335, 50, 97, -2, true, 4, 1, 100, false, &p2itb3_Data);
	p2itb4 = ImageToggleButtonInit(p2, 702, 399, 35, 35, -2, true, 3, 1, 100, true, &p2itb4_Data);
	p2ib3 = ImageButtonInit(p2, 174, 348, 99, 67, -2, true, 2, 1, 100, &p2ib3_Data);
	p2ib4 = ImageButtonInit(p2, 312, 399, 35, 35, -2, true, 1, 1, 100, &p2ib4_Data);

	p3 = SlidePageInit(tileview, NULL, &p3_Data);
	lv_obj_set_pos(p3, LV_HOR_RES * 2, 0);
	p3rect1 = RectangleInit(p3, -10, -10, 820, 80, -2, false, 31, -1, 100, &p3rect1_Data);
	p3l1 = LabelInit(p3, -10, 14, 820, 44, -2, true, 30, 2, -1, 5, &p3l1_Data);
	p3b1 = ButtonInit(p3, 0, 0, 80, 70, -1, true, 29, 2, -1, 5, &p3b1_Data);
	p3b2 = ButtonInit(p3, 720, 0, 80, 70, -1, true, 28, 2, -1, 5, &p3b2_Data);
	p3rect2 = RectangleInit(p3, 10, 80, 385, 190, -2, false, 27, -1, 100, &p3rect2_Data);
	p3rect3 = RectangleInit(p3, 405, 80, 385, 190, -2, false, 26, -1, 100, &p3rect3_Data);
	p3rect9 = RectangleInit(p3, 10, 380, 385, 90, -2, false, 25, -1, 100, &p3rect9_Data);
	p3l9 = LabelInit(p3, 20, 387, 99, 21, -2, true, 24, 0, 0x9A9A9A, -1, &p3l9_Data);
	p3rect5 = RectangleInit(p3, 405, 280, 385, 190, -2, false, 23, -1, 100, &p3rect5_Data);
	p3rect8 = RectangleInit(p3, 10, 280, 385, 90, -2, false, 22, -1, 100, &p3rect8_Data);
	p3l8 = LabelInit(p3, 20, 285, 47, 21, -2, true, 21, 0, 0x9A9A9A, -1, &p3l8_Data);
	p3l2 = LabelInit(p3, 415, 285, 73, 21, -2, true, 20, 0, 0x9A9A9A, -1, &p3l2_Data);
	p3l6 = LabelInit(p3, 415, 85, 79, 21, -2, true, 19, 0, 0x9A9A9A, -1, &p3l6_Data);
	p3l7 = LabelInit(p3, 20, 85, 52, 21, -2, true, 18, 0, 0x9A9A9A, -1, &p3l7_Data);
	p3sl2 = SliderInit(p3, 77, 318, 250, 8, -1, true, 17, 100, 0, 50, -1, -1, &p3sl2_Data);
	p3sl3 = SliderInit(p3, 77, 349, 250, 8, 3, true, 16, 100, 0, 30, -1, -1, &p3sl3_Data);
	p3isl1 = ImageSliderInit(p3, 55, 416, 301, 34, -2, true, 15, 100, 0, 50, -1, -1, &p3isl1_Data);
	p3asl1 = ArcSliderInit(p3, 418, 291, 180, 180, -1, true, 14, 135, 45, -1, -1, -1, 2, &p3asl1_Data);
	p3asl2 = ArcSliderInit(p3, 597, 291, 180, 180, 3, true, 13, 45, 270, -1, -1, -1, 2, &p3asl2_Data);
	p3sw1 = SwitchInit(p3, 66, 138, 49, 28, 0, true, 12, false, &p3sw1_Data);
	p3sw2 = SwitchInit(p3, 66, 207, 49, 28, 0, true, 11, true, &p3sw2_Data);
	p3cb1 = CheckboxInit(p3, 444, 138, 127, 25, 0, true, 10, false, -1, -1, &p3cb1_Data);
	p3cb2 = CheckboxInit(p3, 444, 205, 127, 25, 1, true, 9, true, -1, -1, &p3cb2_Data);
	p3cb3 = CheckboxInit(p3, 619, 138, 127, 25, 2, true, 8, false, -1, -1, &p3cb3_Data);
	p3cb4 = CheckboxInit(p3, 619, 205, 127, 25, 3, true, 7, true, -1, -1, &p3cb4_Data);
	p3sw3 = SwitchInit(p3, 138, 207, 49, 28, 1, true, 6, true, &p3sw3_Data);
	p3sw4 = SwitchInit(p3, 138, 138, 49, 28, 1, true, 5, false, &p3sw4_Data);
	p3sw5 = SwitchInit(p3, 209, 207, 49, 28, 2, true, 4, true, &p3sw5_Data);
	p3sw6 = SwitchInit(p3, 209, 138, 49, 28, 2, true, 3, false, &p3sw6_Data);
	p3sw7 = SwitchInit(p3, 281, 207, 49, 28, 3, true, 2, true, &p3sw7_Data);
	p3sw8 = SwitchInit(p3, 281, 138, 49, 28, 3, true, 1, false, &p3sw8_Data);

	p4 = SlidePageInit(tileview, NULL, &p4_Data);
	lv_obj_set_pos(p4, LV_HOR_RES * 3, 0);
	p4rect1 = RectangleInit(p4, -10, -10, 820, 80, -2, false, 27, -1, 100, &p4rect1_Data);
	p4l1 = LabelInit(p4, -10, 14, 820, 44, -2, true, 26, 2, -1, 5, &p4l1_Data);
	p4b1 = ButtonInit(p4, 0, 0, 80, 70, -1, true, 25, 2, -1, 5, &p4b1_Data);
	p4b2 = ButtonInit(p4, 720, 0, 80, 70, -1, true, 24, 2, -1, 5, &p4b2_Data);
	p4rect2 = RectangleInit(p4, 10, 80, 385, 90, -2, false, 23, -1, 100, &p4rect2_Data);
	p4rect3 = RectangleInit(p4, 405, 80, 385, 190, -2, false, 22, -1, 100, &p4rect3_Data);
	p4rect5 = RectangleInit(p4, 10, 280, 250, 190, -2, false, 21, -1, 100, &p4rect5_Data);
	p4rect6 = RectangleInit(p4, 275, 280, 250, 190, -2, false, 20, -1, 100, &p4rect6_Data);
	p4rect7 = RectangleInit(p4, 540, 280, 250, 190, -2, false, 19, -1, 100, &p4rect7_Data);
	p4l2 = LabelInit(p4, 20, 285, 53, 21, -2, true, 18, 0, 0x9A9A9A, -1, &p4l2_Data);
	p4l3 = LabelInit(p4, 285, 285, 105, 21, -2, true, 17, 0, 0x9A9A9A, -1, &p4l3_Data);
	p4l4 = LabelInit(p4, 550, 285, 31, 21, -2, true, 16, 0, 0x9A9A9A, -1, &p4l4_Data);
	p4l6 = LabelInit(p4, 415, 85, 54, 21, -2, true, 15, 0, 0x9A9A9A, -1, &p4l6_Data);
	p4l7 = LabelInit(p4, 20, 85, 28, 21, -2, true, 14, 0, 0x9A9A9A, -1, &p4l7_Data);
	p4rect8 = RectangleInit(p4, 10, 180, 385, 90, -2, false, 13, -1, 100, &p4rect8_Data);
	p4l8 = LabelInit(p4, 20, 187, 80, 21, -2, true, 12, 0, 0x9A9A9A, -1, &p4l8_Data);
	p4ibar1 = ImageBarInit(p4, 46, 220, 309, 30, -2, true, 11, -1, -1, &p4ibar1_Data);
	p4bar2 = BarInit(p4, 72, 116, 260, 13, -1, true, 10, -1, -1, &p4bar2_Data);
	p4bar3 = BarInit(p4, 72, 144, 260, 13, 3, true, 9, -1, -1, &p4bar3_Data);
	p4ga1 = GaugeInit(p4, 45, 285, 180, 180, -1, true, 8, 1, 100, 0, 2, 2, 135, 45, 5, 3, 80, true, &p4ga1_Data);
	p4iga1 = ImageGaugeInit(p4, 302, 298, 195, 195, -2, true, 7, 0, 100, 0, 2, 2, 135, 45, 5, 3, 80, false, true, &p4iga1_Data);
	p4led1 = LEDInit(p4, 558, 354, 43, 43, 0, true, 6, 100, &p4led1_Data);
	p4led2 = LEDInit(p4, 617, 354, 43, 43, 1, true, 5, 100, &p4led2_Data);
	p4led3 = LEDInit(p4, 676, 354, 43, 43, 2, true, 4, 100, &p4led3_Data);
	p4led4 = LEDInit(p4, 735, 354, 43, 43, 3, true, 3, 100, &p4led4_Data);
	p4abar1 = ArcBarInit(p4, 410, 90, 180, 180, 0, true, 2, 135, 45, -1, -1, -1, 2, &p4abar1_Data);
	p4abar2 = ArcBarInit(p4, 599, 90, 180, 180, 3, true, 1, 90, 0, -1, -1, -1, 2, &p4abar2_Data);

	p5 = SlidePageInit(tileview, NULL, &p5_Data);
	lv_obj_set_pos(p5, LV_HOR_RES * 4, 0);
	p5rect1 = RectangleInit(p5, -10, -10, 820, 80, -2, false, 16, -1, 100, &p5rect1_Data);
	p5l1 = LabelInit(p5, -10, 14, 820, 44, -2, true, 15, 2, -1, 5, &p5l1_Data);
	p5b1 = ButtonInit(p5, 0, 0, 80, 70, -1, true, 14, 2, -1, 5, &p5b1_Data);
	p5b2 = ButtonInit(p5, 720, 0, 80, 70, -1, true, 13, 2, -1, 5, &p5b2_Data);
	p5rect2 = RectangleInit(p5, 10, 80, 385, 190, -2, false, 12, -1, 100, &p5rect2_Data);
	p5rect3 = RectangleInit(p5, 405, 80, 385, 190, -2, false, 11, -1, 100, &p5rect3_Data);
	p5rect5 = RectangleInit(p5, 10, 280, 385, 190, -2, false, 10, -1, 100, &p5rect5_Data);
	p5rect8 = RectangleInit(p5, 405, 280, 385, 190, -2, false, 9, -1, 100, &p5rect8_Data);
	p5l2 = LabelInit(p5, 20, 285, 45, 21, -2, true, 8, 0, 0x9A9A9A, -1, &p5l2_Data);
	p5l6 = LabelInit(p5, 416, 85, 63, 21, -2, true, 7, 0, 0x9A9A9A, -1, &p5l6_Data);
	p5l7 = LabelInit(p5, 20, 85, 45, 21, -2, true, 6, 0, 0x9A9A9A, -1, &p5l7_Data);
	p5tl1 = TextListInit(p5, 413, 107, 370, 160, -2, true, 5, 0, 1, 2, 1, 0, -1, -1, -1, -1, -1, -1, -1, &p5tl1_Data);
	p5tbl1 = TableInit(p5, 18, 108, 370, 156, -2, true, 4, 5, 1, 5, 3, -1, -1, 3, -1, false, 100, &p5tbl1_Data);
	p5ch1 = ChartInit(p5, 18, 305, 370, 160, -2, true, 3, 0, -1, 1, 3, 2, 0, 0, 0, &p5ch1_Data);
	p5osc1 = OscilloscopeInit(p5, 413, 305, 370, 160, -2, true, 2, 0, 1, 1, -1, 3, 3, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, &p5osc1_Data);
	p5l8 = LabelInit(p5, 416, 285, 103, 21, -2, true, 1, 0, 0x9A9A9A, -1, &p5l8_Data);

	p6 = SlidePageInit(tileview, NULL, &p6_Data);
	lv_obj_set_pos(p6, LV_HOR_RES * 5, 0);
	p6rect1 = RectangleInit(p6, -10, -10, 820, 80, -2, false, 20, -1, 100, &p6rect1_Data);
	p6l1 = LabelInit(p6, -10, 14, 820, 44, -2, true, 19, 2, -1, 5, &p6l1_Data);
	p6b1 = ButtonInit(p6, 0, 0, 80, 70, -1, true, 18, 2, -1, 5, &p6b1_Data);
	p6b2 = ButtonInit(p6, 720, 0, 80, 70, -1, true, 17, 2, -1, 5, &p6b2_Data);
	p6rect2 = RectangleInit(p6, 10, 80, 385, 190, -2, false, 16, -1, 100, &p6rect2_Data);
	p6l2 = LabelInit(p6, 20, 85, 79, 21, -2, true, 15, 0, 0x9A9A9A, -1, &p6l2_Data);
	p6rect3 = RectangleInit(p6, 405, 80, 385, 190, -2, false, 14, -1, 100, &p6rect3_Data);
	p6l3 = LabelInit(p6, 415, 85, 113, 21, -2, true, 13, 0, 0x9A9A9A, -1, &p6l3_Data);
	p6tin1 = TextInputInit(p6, 72, 130, 260, 37, -2, true, 12, -1, &p6tin1_Data);
	p6nin1 = NumberInputInit(p6, 467, 130, 260, 37, -2, true, 11, -1, &p6nin1_Data);
	p6nin2 = NumberInputInit(p6, 467, 188, 260, 56, -2, true, 10, 1, &p6nin2_Data);
	p6rect4 = RectangleInit(p6, 10, 280, 385, 190, -2, false, 9, -1, 100, &p6rect4_Data);
	p6rect5 = RectangleInit(p6, 405, 280, 385, 190, -2, false, 8, -1, 100, &p6rect5_Data);
	p6l4 = LabelInit(p6, 20, 285, 83, 21, -2, true, 7, 0, 0x9A9A9A, -1, &p6l4_Data);
	p6l5 = LabelInit(p6, 415, 285, 66, 21, -2, true, 6, 0, 0x9A9A9A, -1, &p6l5_Data);
	p6tin2 = TextInputInit(p6, 72, 188, 260, 56, -2, true, 5, 1, &p6tin2_Data);
	p6din1 = DateInputInit(p6, 72, 328, 260, 37, -2, true, 4, -1, &p6din1_Data);
	p6din2 = DateInputInit(p6, 72, 391, 260, 56, -2, true, 3, 1, &p6din2_Data);
	p6sp2 = SpinboxInit(p6, 497, 328, 200, 37, -2, true, 2, 1, 0, -1, &p6sp2_Data);
	p6sp4 = SpinboxInit(p6, 455, 399, 284, 37, -2, true, 1, 3, 0, -1, &p6sp4_Data);

	p7 = SlidePageInit(tileview, NULL, &p7_Data);
	lv_obj_set_pos(p7, LV_HOR_RES * 6, 0);
	p7rect1 = RectangleInit(p7, -10, -10, 820, 80, -2, false, 18, -1, 100, &p7rect1_Data);
	p7l1 = LabelInit(p7, -10, 14, 820, 44, -2, true, 17, 2, -1, 5, &p7l1_Data);
	p7b1 = ButtonInit(p7, 0, 0, 80, 70, -1, true, 16, 2, -1, 5, &p7b1_Data);
	p7b2 = ButtonInit(p7, 720, 0, 80, 70, -1, true, 15, 2, -1, 5, &p7b2_Data);
	p7rect2 = RectangleInit(p7, 10, 80, 385, 190, -2, false, 14, -1, 100, &p7rect2_Data);
	p7l2 = LabelInit(p7, 20, 85, 115, 21, -2, true, 13, 0, 0x9A9A9A, -1, &p7l2_Data);
	p7rect3 = RectangleInit(p7, 405, 80, 385, 190, -2, false, 12, -1, 100, &p7rect3_Data);
	p7l3 = LabelInit(p7, 415, 85, 29, 21, -2, true, 11, 0, 0x9A9A9A, -1, &p7l3_Data);
	p7rect4 = RectangleInit(p7, 10, 280, 385, 190, -2, false, 10, -1, 100, &p7rect4_Data);
	p7l4 = LabelInit(p7, 20, 285, 48, 21, -2, true, 9, 0, 0x9A9A9A, -1, &p7l4_Data);
	p7ddl1 = DropDownListInit(p7, 120, 129, 150, 37, -1, true, 8, -1, &p7ddl1_Data);
	p7ddl2 = DropDownListInit(p7, 120, 198, 150, 37, 3, true, 7, -1, &p7ddl2_Data);
	p7li1 = ListInit(p7, 467, 105, 260, 160, -1, true, 6, 3, -1, &p7li1_Data);
	p7ro1 = RollerInit(p7, 62, 321, 106, 123, -1, true, 5, -1, &p7ro1_Data);
	p7ro2 = RollerInit(p7, 231, 321, 115, 123, 3, true, 4, -1, &p7ro2_Data);
	p7rect6 = RectangleInit(p7, 405, 280, 385, 190, -2, false, 3, -1, 100, &p7rect6_Data);
	p7l6 = LabelInit(p7, 415, 285, 96, 21, -2, true, 2, 0, 0x9A9A9A, -1, &p7l6_Data);
	p7fb1 = FileBrowserInit(p7, 467, 305, 260, 160, -1, true, 1, 2, -1, &p7fb1_Data);

	currentPage = p1;
	static lv_point_t valid_pos;
	valid_pos.x = 0;
	lv_tileview_set_valid_positions(tileview, &valid_pos, 1);
	lv_obj_set_event_cb(tileview, tileviewEventCallback);
	lv_obj_set_event_cb(p1, p1EventCallback);
	lv_obj_set_event_cb(p1b1, p1b1EventCallback);
	lv_obj_set_event_cb(p1b2, p1b2EventCallback);
	lv_obj_set_event_cb(p2, p2EventCallback);
	lv_obj_set_event_cb(p2b1, p2b1EventCallback);
	lv_obj_set_event_cb(p2b2, p2b2EventCallback);
	lv_obj_set_event_cb(p2b3, p2b3EventCallback);
	lv_obj_set_event_cb(p2tb1, p2tb1EventCallback);
	lv_obj_set_event_cb(p2ib1, p2ib1EventCallback);
	lv_obj_set_event_cb(p2itb1, p2itb1EventCallback);
	lv_obj_set_event_cb(p2b4, p2b4EventCallback);
	lv_obj_set_event_cb(p2tb2, p2tb2EventCallback);
	lv_obj_set_event_cb(p2ib2, p2ib2EventCallback);
	lv_obj_set_event_cb(p2itb2, p2itb2EventCallback);
	lv_obj_set_event_cb(p2b5, p2b5EventCallback);
	lv_obj_set_event_cb(p2b6, p2b6EventCallback);
	lv_obj_set_event_cb(p2tb3, p2tb3EventCallback);
	lv_obj_set_event_cb(p2tb4, p2tb4EventCallback);
	lv_obj_set_event_cb(p2itb3, p2itb3EventCallback);
	lv_obj_set_event_cb(p2itb4, p2itb4EventCallback);
	lv_obj_set_event_cb(p2ib3, p2ib3EventCallback);
	lv_obj_set_event_cb(p2ib4, p2ib4EventCallback);
	lv_obj_set_event_cb(p3, p3EventCallback);
	lv_obj_set_event_cb(p3b1, p3b1EventCallback);
	lv_obj_set_event_cb(p3b2, p3b2EventCallback);
	lv_obj_set_event_cb(p3sl2, p3sl2EventCallback);
	lv_obj_set_event_cb(p3sl3, p3sl3EventCallback);
	lv_obj_set_event_cb(p3isl1, p3isl1EventCallback);
	lv_obj_set_event_cb(p3asl1, p3asl1EventCallback);
	lv_obj_set_event_cb(p3asl2, p3asl2EventCallback);
	lv_obj_set_event_cb(p3sw1, p3sw1EventCallback);
	lv_obj_set_event_cb(p3sw2, p3sw2EventCallback);
	lv_obj_set_event_cb(p3cb1, p3cb1EventCallback);
	lv_obj_set_event_cb(p3cb2, p3cb2EventCallback);
	lv_obj_set_event_cb(p3cb3, p3cb3EventCallback);
	lv_obj_set_event_cb(p3cb4, p3cb4EventCallback);
	lv_obj_set_event_cb(p3sw3, p3sw3EventCallback);
	lv_obj_set_event_cb(p3sw4, p3sw4EventCallback);
	lv_obj_set_event_cb(p3sw5, p3sw5EventCallback);
	lv_obj_set_event_cb(p3sw6, p3sw6EventCallback);
	lv_obj_set_event_cb(p3sw7, p3sw7EventCallback);
	lv_obj_set_event_cb(p3sw8, p3sw8EventCallback);
	lv_obj_set_event_cb(p4, p4EventCallback);
	lv_obj_set_event_cb(p4b1, p4b1EventCallback);
	lv_obj_set_event_cb(p4b2, p4b2EventCallback);
	lv_obj_set_event_cb(p5, p5EventCallback);
	lv_obj_set_event_cb(p5b1, p5b1EventCallback);
	lv_obj_set_event_cb(p5b2, p5b2EventCallback);
	lv_obj_set_event_cb(p6, p6EventCallback);
	lv_obj_set_event_cb(p6b1, p6b1EventCallback);
	lv_obj_set_event_cb(p6b2, p6b2EventCallback);
	lv_obj_set_event_cb(p6tin1, p6tin1EventCallback);
	lv_obj_set_event_cb(p6nin1, p6nin1EventCallback);
	lv_obj_set_event_cb(p6nin2, p6nin2EventCallback);
	lv_obj_set_event_cb(p6tin2, p6tin2EventCallback);
	lv_obj_set_event_cb(p6din1, p6din1EventCallback);
	lv_obj_set_event_cb(p6din2, p6din2EventCallback);
	lv_obj_set_event_cb(p7, p7EventCallback);
	lv_obj_set_event_cb(p7b1, p7b1EventCallback);
	lv_obj_set_event_cb(p7b2, p7b2EventCallback);
	lv_obj_set_event_cb(p7ddl1, p7ddl1EventCallback);
	lv_obj_set_event_cb(p7ddl2, p7ddl2EventCallback);
	lv_obj_set_event_cb(p7li1, p7li1EventCallback);
	lv_obj_set_event_cb(p7ro1, p7ro1EventCallback);
	lv_obj_set_event_cb(p7ro2, p7ro2EventCallback);
	EventNode startUp = {currentPage, 0};	lv_event_send(currentPage, LV_EVENT_VALUE_CHANGED, &startUp);}

void audioInit()
{
}

void audioLoad()
{
}

void pageTimerUpdate()
{
	if (currentPage == NULL) return;
	EventNode timerEN = {currentPage, 2};
	HMI_SlidePage_Data* d = (HMI_SlidePage_Data*)lv_obj_get_user_data(currentPage);
	if (!d->_en) return;
	if (d->_t + d->TimerInterval > timer_NowTime()) return;
	if (d->TimerMode == 0) d->_en = false;
	lv_event_send(currentPage, LV_EVENT_VALUE_CHANGED, &timerEN);
	d->_t = timer_NowTime();
}

void imageGaugeUpdate()
{
	if (currentPage == NULL) return;
	HMI_SlidePage_Data* d = (HMI_SlidePage_Data*)lv_obj_get_user_data(currentPage);
	if (d->_igat + 100 > timer_NowTime()) return;
	lv_obj_t* child = lv_obj_get_child(currentPage, NULL);
	while (child != NULL)
	{
		if (Hmi.ID_TO_SN(child) / 1000 % 100 == HMI_IGA)
			lv_gauge_set_value(child, 0, ((HMI_ImageGauge_Data*)lv_obj_get_user_data(child))->NeedleValue1);
		child = lv_obj_get_child(currentPage, child);
	}
	d->_igat = timer_NowTime();
}

void audioUpdate()
{
}

void oscUpdate()
{
	if (currentPage == p5 && p5osc1->refreshTimeUp()) p5osc1->refresh();
}

void consoleUpdate()
{
}

void hmi_task_handler()
{
	pageTimerUpdate();
	imageGaugeUpdate();
	audioUpdate();
	oscUpdate();
	consoleUpdate();
	lv_task_handler();
	helperYield();
}

EventNode* eventQueuePop()
{
	int current = eqHead;
	if (eqHead == eqRail)
	{
		eventQueue[eqHead].obj = NULL;
		eventQueue[eqHead].eventId = -1;
	}
	else
		eqHead = eqHead + 1 < EVENT_QUEUE_SIZE ? eqHead + 1 : 0;
	return &eventQueue[current];
}

bool eventQueuePush(EventNode e)
{
	int next = eqRail + 1 < EVENT_QUEUE_SIZE ? eqRail + 1 : 0;
	if (next == eqHead)
		return false;
	eventQueue[eqRail] = e;
	eqRail = next;
	return true;
}

void alarmBannerRefreshText()
{
	int i, len, space = 0;
	char* buf, * p;
	lv_point_t size = { 0, 0 };
	lv_draw_label_dsc_t label_dsc;
	if (ab == NULL) return;
	for (i = 0, len = 0; i < ALARM_BANNER_BANK_SIZE; i++)
		if (abBank[i])
			len += strlen(abBank[i]) + abBankSpacing;

	buf = (char*)malloc(sizeof(char) * (len + 1));
	for (i = 0, p = buf; i < ALARM_BANNER_BANK_SIZE; i++)
		if (abBank[i])
		{
			space = p == buf ? 0 : abBankSpacing;
			memset(p, ' ', space);
			p += space;
			strcpy(p, abBank[i]);
			p += strlen(abBank[i]);
		}
	*p = '\0';
	if (space > LV_LABEL_WAIT_CHAR_COUNT)
	{
		lv_draw_label_dsc_init(&label_dsc);
		lv_obj_init_draw_label_dsc(abLabel, LV_CHART_PART_BG, &label_dsc);
		_lv_txt_get_size(&size, buf, label_dsc.font, label_dsc.letter_space, label_dsc.line_space, LV_COORD_MAX, LV_TXT_FLAG_CENTER);
		if (size.x > lv_obj_get_width(abLabel))
		{
			space = abBankSpacing - LV_LABEL_WAIT_CHAR_COUNT;
			memset(p, ' ', space);
			p += space;
			*p = '\0';
		}
	}
	lv_label_set_text(abLabel, buf);
	free(buf);
}

char* envPathCombine(char* env, char* path)
{
	char* buf = (char*)malloc(sizeof(char) * (strlen(getenv(env)) + strlen(path) + 1));
	sprintf(buf, "%s%s", getenv(env), path);
	return buf;
}

/* Widgets init function */
lv_obj_t* SlidePageInit(lv_obj_t* parent, char* _Wallpaper, HMI_SlidePage_Data* data)
{
	lv_obj_t* obj;
	if(_Wallpaper == NULL)
	{
		obj = lv_obj_create(parent, NULL);
		lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_border_width(obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 0);
	}
	else
	{
		obj = lv_img_create(parent, NULL);
		lv_img_set_src(obj, _Wallpaper);
		free(_Wallpaper);
	}
	lv_obj_set_size(obj, LV_HOR_RES, LV_VER_RES);
	lv_tileview_add_element(parent, obj);
	if (data != NULL)
	{
		data->_t = timer_NowTime();
		data->_en = data->TimerEnable;
		lv_obj_set_user_data(obj, data);
	}
	return obj;
}

void BasicObjInit(lv_obj_t* obj, lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, HMI_BasicObj_Data* data)
{
	lv_obj_set_pos(obj, _X, _Y);
	lv_obj_set_size(obj, _Width, _Height);
	set_color_id(obj, _Color);
	void (*enable_cb)(lv_obj_t*, lv_state_t) = _Enable ? lv_obj_clear_state : lv_obj_add_state;
	enable_cb(obj, LV_STATE_DISABLED);
	if (data != NULL)
		lv_obj_set_user_data(obj, data);
}

lv_obj_t* ArcInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                  uint16_t _AngleStart, uint16_t _AngleEnd, double _TextColor, int8_t _TextFont, double _BgColor, int8_t _BarThickness, HMI_Arc_Data* data)
{
	lv_obj_t* obj = lv_arc_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	lv_arc_set_bg_start_angle(obj, _AngleStart);
	lv_arc_set_bg_end_angle(obj, _AngleEnd);
	if (_TextColor != -1)
		lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(_TextColor));
		lv_obj_set_style_local_line_width(obj, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 15 + 5 * _BarThickness);
		lv_obj_set_style_local_line_width(obj, LV_ARC_PART_BG, LV_STATE_DEFAULT, 15 + 5 * _BarThickness);
	set_font_id(obj, _TextFont);
	if (_BgColor != -1)
	{
		lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex((uint32_t)_BgColor));
		lv_obj_set_style_local_bg_grad_dir(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
	}
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		if (data->Background == 1)
		{
			lv_obj_set_style_local_radius(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
		}
		else if (data->Background == 2)
		{
			lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
			lv_obj_set_style_local_border_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
			lv_obj_set_style_local_outline_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		}
		if (data->MaxValue - data->MinValue >= 65535 / pow(10, data->Decimal))
			lv_arc_set_range(obj, -32768, 32767);
		else if (data->MaxValue > 32767 / pow(10, data->Decimal))
			lv_arc_set_range(obj, 32767 - (data->MaxValue - data->MinValue) * pow(10, data->Decimal), 32767);
		else if (data->MinValue < -32768 / pow(10, data->Decimal))
			lv_arc_set_range(obj, -32768, -32768 + (data->MaxValue - data->MinValue) * pow(10, data->Decimal));
		else
			lv_arc_set_range(obj ,data->MinValue * pow(10, data->Decimal), data->MaxValue * pow(10, data->Decimal));
		lv_arc_set_value(obj, (data->Value - data->MinValue) / (data->MaxValue - data->MinValue) * (lv_arc_get_max_value(obj) - lv_arc_get_min_value(obj)) + lv_arc_get_min_value(obj));
	}
	return obj;
}

lv_obj_t* ArcBarInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     uint16_t _AngleStart, uint16_t _AngleEnd, double _TextColor, int8_t _TextFont, double _BgColor, int8_t _BarThickness, HMI_Arc_Data* data)
{
	lv_obj_t* obj = ArcInit(parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, _AngleStart, _AngleEnd, _TextColor, _TextFont, _BgColor, _BarThickness, data);
	lv_obj_set_event_cb(obj, ArcBarValueChange);
	ArcBarRefreshText(obj);
	return obj;
}

void ArcBarValueChange(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
		ArcBarRefreshText(o);
}

void ArcBarRefreshText(lv_obj_t* o)
{
	HMI_Arc_Data* d = (HMI_Arc_Data*)lv_obj_get_user_data(o);
	if (d->ShowValue)
	{
		char* buf = (char*)lv_obj_get_style_value_str(o, LV_OBJ_PART_MAIN);
		int len = strlen(d->SuffixList[_LangID]) + 7;
		if(buf == NULL)
			buf = (char*)malloc(len * sizeof(char));
		char s[10] = "%.0lf%s";
		s[2] += d->Decimal;
		sprintf(buf, s, d->Value, d->SuffixList[_LangID]);
		lv_obj_set_style_local_value_str(o, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, buf);
	}
}

lv_obj_t* ArcSliderInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                        uint16_t _AngleStart, uint16_t _AngleEnd, double _TextColor, int8_t _TextFont, double _BgColor, int8_t _BarThickness, HMI_Arc_Data* data)
{
	lv_obj_t* obj = ArcInit(parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, _AngleStart, _AngleEnd, _TextColor, _TextFont, _BgColor, _BarThickness, data);
	lv_arc_set_adjustable(obj, true);
	ArcSliderRefreshText(obj);
	return obj;
}

void ArcSliderRefreshText(lv_obj_t* o)
{
	HMI_Arc_Data* d = (HMI_Arc_Data*)lv_obj_get_user_data(o);
	if (d->ShowValue)
	{
		char* buf = (char*)lv_obj_get_style_value_str(o, LV_OBJ_PART_MAIN);
		int len = strlen(d->SuffixList[_LangID]) + 7;
		if(buf == NULL)
			buf = (char*)malloc(len * sizeof(char));
		sprintf(buf, "%d%s", lv_arc_get_value(o), d->SuffixList[_LangID]);
		lv_obj_set_style_local_value_str(o, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, buf);
	}
}

lv_obj_t* BarInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                  double _TextColor, int8_t _TextFont, HMI_Bar_Data* data)
{
	lv_obj_t* obj = lv_bar_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	if (_TextColor != -1)
		lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(_TextColor));
	set_font_id(obj, _TextFont);
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		lv_obj_set_event_cb(obj, BarValueChange);
		if(data->ShowValue == 1)
		{
			lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_MID);
			lv_obj_set_style_local_value_ofs_y(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, -10);
		}
		else if(data->ShowValue == 2)
		{
			lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
			lv_obj_set_style_local_value_ofs_y(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
		}
		else if(data->ShowValue == 3)
		{
			lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_LEFT_MID);
			lv_obj_set_style_local_value_ofs_x(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, -10);
		}
		else if(data->ShowValue == 4)
		{
			lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_RIGHT_MID);
			lv_obj_set_style_local_value_ofs_x(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
		}
		if (data->MaxValue - data->MinValue >= 65535 / pow(10, data->Decimal))
			lv_bar_set_range(obj, -32768, 32767);
		else if (data->MaxValue > 32767 / pow(10, data->Decimal))
			lv_bar_set_range(obj, 32767 - (data->MaxValue - data->MinValue) * pow(10, data->Decimal), 32767);
		else if (data->MinValue < -32768 / pow(10, data->Decimal))
			lv_bar_set_range(obj, -32768, -32768 + (data->MaxValue - data->MinValue) * pow(10, data->Decimal));
		else
			lv_bar_set_range(obj ,data->MinValue * pow(10, data->Decimal), data->MaxValue * pow(10, data->Decimal));
		lv_bar_set_value(obj, (data->Value - data->MinValue) / (data->MaxValue - data->MinValue) * (lv_bar_get_max_value(obj) - lv_bar_get_min_value(obj)) + lv_bar_get_min_value(obj), LV_ANIM_OFF);
		lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
	}
	return obj;
}

void BarValueChange(lv_obj_t* o, lv_event_t e)
{
	if (e == LV_EVENT_VALUE_CHANGED)
	{
		BarRefreshText(o);
	}
}

void BarRefreshText(lv_obj_t* o)
{
	HMI_Bar_Data* d = (HMI_Bar_Data*)lv_obj_get_user_data(o);
	if (d->ShowValue)
	{
		char* buf = (char*)lv_obj_get_style_value_str(o, LV_OBJ_PART_MAIN);
		int len = strlen(d->PrefixList[_LangID]) + strlen(d->SuffixList[_LangID]) + 7;
		if(buf == NULL)
			buf = (char*)malloc(len * sizeof(char));
		char s[10] = "%s%.0lf%s";
		s[4] += d->Decimal;
		sprintf(buf, s, d->PrefixList[_LangID], d->Value, d->SuffixList[_LangID]);
		lv_obj_set_style_local_value_str(o, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, buf);
	}
}

lv_obj_t* ButtonInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     int8_t _ButtonType, double _TextColor, int8_t _TextFont, HMI_Button_Data* data)
{
	lv_obj_t* label;
	lv_obj_t* obj = lv_btn_create(parent, NULL);
	label = lv_label_create(obj, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	if(_ButtonType == 1)
		lv_btn_set_state(obj, _Enable ? LV_BTN_STATE_CHECKED_RELEASED : LV_BTN_STATE_CHECKED_DISABLED);
	else if(_ButtonType == 2)
	{
		lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_border_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_outline_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_shadow_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
	}
	if (_TextColor != -1)
		lv_obj_set_style_local_text_color(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(_TextColor));
	set_font_id(label, _TextFont);
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		ButtonRefreshText(obj);
	}
	return obj;
}

void ButtonRefreshText(lv_obj_t* o)
{
	HMI_Button_Data* d = (HMI_Button_Data*)lv_obj_get_user_data(o);
	lv_obj_t* label = lv_obj_get_child(o, NULL);
	if (strlen(d->Symbol) != 0)
		lv_label_set_text_fmt(label, "%s %s", d->Symbol, d->TextList[_LangID]);
	else
		lv_label_set_text_fmt(label, "%s", d->TextList[_LangID]);
}

lv_obj_t* ChartInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                    int8_t _ChartType, int8_t _TextFont, uint16_t _DivLineV, uint16_t _DivLineH, int8_t _DivLineType, int8_t _BorderX, int8_t _BorderY, int8_t _BorderThickness, HMI_Chart_Data* data)
{
	lv_obj_t* obj =  lv_chart_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	lv_chart_set_type(obj, _ChartType == 0 ? LV_CHART_TYPE_COLUMN : LV_CHART_TYPE_LINE);
	lv_obj_set_style_local_bg_opa(obj, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, _ChartType == 2 ? LV_OPA_50 : LV_OPA_0);
	lv_obj_set_style_local_bg_grad_dir(obj, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, _ChartType == 2 ? LV_GRAD_DIR_VER : LV_GRAD_DIR_NONE);
	lv_obj_set_style_local_bg_main_stop(obj, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, _ChartType == 2 ? 255 : 0);
	lv_obj_set_style_local_bg_grad_stop(obj, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, 0);
	lv_chart_set_div_line_count(obj, _DivLineH, _DivLineV);
	if (_DivLineType == 0)
		lv_obj_set_style_local_line_width(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, 0);
	else if (_DivLineType == 1)
		lv_obj_set_style_local_line_dash_gap(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, 0);
	else if (_DivLineType == 2)
		lv_obj_set_style_local_line_dash_gap(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, 8);
	set_font_id(obj, _TextFont);
	lv_obj_set_style_local_border_side(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, (_BorderX << 0) | (_BorderY << 2));
	lv_obj_set_style_local_border_width(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, lv_obj_get_style_border_side(obj, LV_CHART_PART_SERIES_BG) > 0 ? _BorderThickness + 1 : 0);
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		lv_chart_set_point_count(obj, data->PointCount);
		lv_chart_set_y_range(obj, LV_CHART_AXIS_PRIMARY_Y, data->MinValue, data->MaxValue);
		if (data->ChannelCount >= 0) lv_chart_add_series(obj, lv_color_hex(data->ChannelColor1 < 0 ? get_theme_color(0, 0) : (uint32_t)data->ChannelColor1));
		if (data->ChannelCount >= 1) lv_chart_add_series(obj, lv_color_hex(data->ChannelColor2 < 0 ? get_theme_color(1, 0) : (uint32_t)data->ChannelColor2));
		if (data->ChannelCount >= 2) lv_chart_add_series(obj, lv_color_hex(data->ChannelColor3 < 0 ? get_theme_color(2, 0) : (uint32_t)data->ChannelColor3));
		if (data->ChannelCount >= 3) lv_chart_add_series(obj, lv_color_hex(data->ChannelColor4 < 0 ? get_theme_color(3, 0) : (uint32_t)data->ChannelColor4));
		ChartRefreshText(obj);
	}
	return obj;
}

void ChartRefreshText(lv_obj_t* o)
{
	HMI_Chart_Data* d = (HMI_Chart_Data*)lv_obj_get_user_data(o);
	bool xTickText = strlen(d->TickTextXList[_LangID]) != 0;
	bool yTickText = strlen(d->TickTextYList[_LangID]) != 0;
	int pad_l = 0, pad_r = 0, pad_t = 0, pad_b = 0;
	lv_point_t size = { 0, 0 };
	lv_draw_label_dsc_t label_dsc;
	lv_draw_label_dsc_init(&label_dsc);
	lv_obj_init_draw_label_dsc(o, LV_CHART_PART_BG, &label_dsc);
	char* buf = strdup(d->TickTextXList[_LangID]);
	for (char* p = strtok(buf, "\n"); p != NULL; p = strtok(NULL, "\n"))
	{
		_lv_txt_get_size(&size, p, label_dsc.font, label_dsc.letter_space, label_dsc.line_space, LV_COORD_MAX, LV_TXT_FLAG_CENTER);
		if (p == buf)
			if (pad_l < size.x / 2) pad_l = size.x / 2;
		if (pad_b < size.y) pad_b = size.y;
	}
	if (xTickText)
		pad_r = size.x / 2;
	free(buf);
	buf = strdup(d->TickTextYList[_LangID]);
	for (char* p = strtok(buf, "\n"); p != NULL; p = strtok(NULL, "\n"))
	{
		_lv_txt_get_size(&size, p, label_dsc.font, label_dsc.letter_space, label_dsc.line_space, LV_COORD_MAX, LV_TXT_FLAG_CENTER);
		if (p == buf) pad_t = size.y / 2;
		if (pad_l < size.x) pad_l = size.x;
	}
	if (yTickText)
		if (pad_b < size.y / 2) pad_b = size.y / 2;
	free(buf);
	int pad = (xTickText ? 2 * (LV_DPI / 10) : 8);
	lv_obj_set_style_local_pad_bottom(o, LV_CHART_PART_BG, LV_STATE_DEFAULT, pad_b + pad);
	lv_obj_set_style_local_pad_top(o, LV_CHART_PART_BG, LV_STATE_DEFAULT, pad_t < pad ? pad : pad_t);
	pad = (yTickText ? 2 * (LV_DPI / 10) : 8);
	lv_obj_set_style_local_pad_left(o, LV_CHART_PART_BG, LV_STATE_DEFAULT, pad_l + pad);
	lv_obj_set_style_local_pad_right(o, LV_CHART_PART_BG, LV_STATE_DEFAULT, pad_r < pad ? pad : pad_r);
	lv_chart_set_x_tick_texts(o, d->TickTextXList[_LangID], d->TickX + 1, LV_CHART_AXIS_DRAW_LAST_TICK);
	lv_chart_set_y_tick_texts(o, d->TickTextYList[_LangID], d->TickY + 1, LV_CHART_AXIS_DRAW_LAST_TICK);
	lv_chart_set_x_tick_length(o, strlen(d->TickTextXList[_LangID]) == 0 ? 0 : d->TickSize * 5, strlen(d->TickTextXList[_LangID]) == 0 ? 0 : d->TickSize * 3);
	lv_chart_set_y_tick_length(o, strlen(d->TickTextYList[_LangID]) == 0 ? 0 : d->TickSize * 5, strlen(d->TickTextYList[_LangID]) == 0 ? 0 : d->TickSize * 3);
	lv_obj_refresh_style(o, LV_OBJ_PART_MAIN, LV_STYLE_PROP_ALL);
}

lv_obj_t* CheckboxInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                       bool _Checked, double _TextColor, int8_t _TextFont, HMI_Checkbox_Data* data)
{
	lv_obj_t* obj =  lv_checkbox_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	lv_obj_t* label = ((lv_checkbox_ext_t*)lv_obj_get_ext_attr(obj))->label;
	lv_checkbox_set_checked(obj, _Checked);
	if(!_Enable)
	{
		lv_btn_set_state(obj, _Checked ? LV_BTN_STATE_CHECKED_DISABLED : LV_BTN_STATE_DISABLED);
		lv_obj_add_state(((lv_checkbox_ext_t*)lv_obj_get_ext_attr(obj))->bullet, LV_STATE_DISABLED);
		lv_obj_add_state(((lv_checkbox_ext_t*)lv_obj_get_ext_attr(obj))->label, LV_STATE_DISABLED);
	}
	if (_TextColor != -1)
		lv_obj_set_style_local_text_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(_TextColor));
	set_font_id(obj, _TextFont);
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		CheckboxRefreshText(obj);
	}
	return obj;
}

void CheckboxRefreshText(lv_obj_t* o)
{
	HMI_Checkbox_Data* d = (HMI_Checkbox_Data*)lv_obj_get_user_data(o);
	lv_obj_t* label = ((lv_checkbox_ext_t*)lv_obj_get_ext_attr(o))->label;
	lv_label_set_text_fmt(label, "%s", d->TextList[_LangID]);
}

lv_obj_t* CircleInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     double _BgColor, uint16_t _Opacity, HMI_BasicObj_Data* data)
{
	lv_obj_t* obj = RectangleInit(parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, _BgColor, _Opacity, data);
		lv_obj_set_style_local_radius(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
	if (_BgColor != -1)
	{
		lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex((uint32_t)_BgColor));
		lv_obj_set_style_local_bg_grad_dir(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
		lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, _Opacity * 255 / 100);
	}
	return obj;
}

lv_obj_t* DateInputInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                        int8_t _TextFont, HMI_DateInput_Data* data)
{
	lv_obj_t* obj = TextAreaInit(parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, _TextFont, NULL);
	lv_textarea_set_accepted_chars(obj, "0123456789/-");
	lv_textarea_set_one_line(obj, true);
	void (*enable_cb)(lv_obj_t*, lv_state_t) = _Enable ? lv_obj_clear_state : lv_obj_add_state;
	lv_obj_t* child = NULL;	while (child = lv_obj_get_child(obj, child))		enable_cb(child, LV_STATE_DISABLED);	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		DateInputRefreshText(obj);
	}
	return obj;
}

void DateInputRefreshText(lv_obj_t* o)
{
	if (o == NULL) return;
	HMI_DateInput_Data* d = (HMI_DateInput_Data*)lv_obj_get_user_data(o);
	char buf[32];
	switch (d->DisplayType)
	{
		case 0: lv_snprintf(buf, sizeof(buf), "%d-%02d-%02d", d->DefaultYear, d->DefaultMonth, d->DefaultDay); break;
		case 1: lv_snprintf(buf, sizeof(buf), "%02d-%02d-%d", d->DefaultMonth, d->DefaultDay, d->DefaultYear); break;
		case 2: lv_snprintf(buf, sizeof(buf), "%02d-%02d-%d", d->DefaultDay, d->DefaultMonth, d->DefaultYear); break;
		case 3: lv_snprintf(buf, sizeof(buf), "%d/%02d/%02d", d->DefaultYear, d->DefaultMonth, d->DefaultDay); break;
		case 4: lv_snprintf(buf, sizeof(buf), "%02d/%02d/%d", d->DefaultMonth, d->DefaultDay, d->DefaultYear); break;
		case 5: lv_snprintf(buf, sizeof(buf), "%02d/%02d/%d", d->DefaultDay, d->DefaultMonth, d->DefaultYear); break;
	}
	lv_textarea_set_text(o, buf);
}

void cal_create(lv_obj_t* o)
{
	if(lv_indev_get_type(lv_indev_get_act()) != LV_INDEV_TYPE_POINTER) return;
	lv_calendar_date_t sel_day;
	HMI_DateInput_Data* d = (HMI_DateInput_Data*)lv_obj_get_user_data(o);
	sel_day.year = d->DefaultYear;
	sel_day.month = d->DefaultMonth;
	sel_day.day = d->DefaultDay;
	lv_obj_clear_state(o, LV_STATE_FOCUSED);
	lv_obj_set_click(lv_layer_top(), true);
	lv_obj_set_event_cb(lv_layer_top(), cal_event_cb);
	lv_obj_set_style_local_bg_color(lv_layer_top(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
	lv_obj_set_style_local_bg_opa(lv_layer_top(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
	cal = lv_calendar_create(lv_layer_top(), NULL);
	lv_obj_set_size(cal, 300, 330);
	lv_calendar_set_showed_date(cal, &sel_day);
	lv_obj_align(cal, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_event_cb(cal, cal_event_cb);
	lv_obj_set_user_data(cal, o);
	lv_obj_add_state(cal, LV_STATE_FOCUSED);
}

void cal_event_cb(lv_obj_t* o, lv_event_t e)
{
	if (o == lv_layer_top() && e == LV_EVENT_PRESSED)
	{
		if (cal) lv_obj_del(cal);
		cal = NULL;
		lv_obj_set_click(lv_layer_top(), false);
		lv_obj_set_event_cb(lv_layer_top(), NULL);
		lv_obj_set_style_local_bg_opa(lv_layer_top(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	}
	else if (o == cal && e == LV_EVENT_VALUE_CHANGED)
	{
		char buf[32];
		lv_obj_t* din = (lv_obj_t *) lv_obj_get_user_data(o);
		HMI_DateInput_Data* d = (HMI_DateInput_Data*)lv_obj_get_user_data(din);
		lv_calendar_date_t *sel_day = lv_calendar_get_pressed_date(o);
		d->DefaultYear = sel_day->year;
		d->DefaultMonth = sel_day->month;
		d->DefaultDay = sel_day->day;
		DateInputRefreshText(din);
		if (cal) lv_obj_del(cal);
		cal = NULL;
		lv_obj_set_click(lv_layer_top(), false);
		lv_obj_set_event_cb(lv_layer_top(), NULL);
		lv_obj_set_style_local_bg_opa(lv_layer_top(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	}
}

lv_obj_t* DropDownListInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                           int8_t _TextFont, HMI_DropDownList_Data* data)
{
	lv_obj_t* obj =  lv_dropdown_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	set_font_id(obj, _TextFont);
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		DropDownListRefreshText(obj);
	}
	return obj;
}

void DropDownListRefreshText(lv_obj_t* o)
{
	HMI_DropDownList_Data* d = (HMI_DropDownList_Data*)lv_obj_get_user_data(o);
	int index = lv_dropdown_get_selected(o);
	lv_dropdown_set_options(o, d->DropDownListText[_LangID]);
	lv_dropdown_set_selected(o, lv_dropdown_get_option_cnt(o) > index ? index : 0);
	lv_obj_refresh_style(o, LV_OBJ_PART_MAIN, LV_STYLE_PROP_ALL);
}

FileBrowser* FileBrowserInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                             int8_t _Spacing, int8_t _TextFont, HMI_FileBrowser_Data* data)
{
	FileBrowser* obj = new FileBrowser();
	lv_obj_t* o = obj->begin(parent, _X, _Y, _Width, _Height, _Color, _TextFont, (_Spacing + 1) * 5, 256, _Enable);
	if (data != NULL)
	{
		data->_fb = obj;
		set_list_user_data(o, data);
	}
	return obj;
}

lv_obj_t* GaugeInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                    int8_t _NeedleCount, int32_t _MaxValue, int32_t _MinValue, int8_t _NeedleThickness, int8_t _CenterPointSize, uint16_t _AngleStart, uint16_t _AngleEnd, int8_t _LabelCnt, int8_t _IntervalTicks, uint16_t _CriticalValue, bool _Background, HMI_Gauge_Data* data)
{
	lv_obj_t* obj = lv_gauge_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	lv_gauge_set_range(obj, _MinValue, _MaxValue);
	switch (_CenterPointSize)
	{
		case 0: lv_obj_set_style_local_size(obj, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, LV_DPX(0)); break;
		case 1: lv_obj_set_style_local_size(obj, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, LV_DPX(15)); break;
		case 2: lv_obj_set_style_local_size(obj, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, LV_DPX(30)); break;
		case 3: lv_obj_set_style_local_size(obj, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, LV_DPX(45)); break;
	}
	lv_gauge_ext_t* ext = (lv_gauge_ext_t*)lv_obj_get_ext_attr(obj);
	lv_color_t* needle_colors = (lv_color_t*)malloc(sizeof(lv_color_t) * 4);
	lv_gauge_set_needle_count(obj, _NeedleCount + 1, needle_colors);
	int lcnt = _LabelCnt > 1 ? _LabelCnt + 1 : 0;
	lv_gauge_set_scale(obj, _AngleEnd > _AngleStart ? _AngleEnd - _AngleStart : 360 - _AngleStart + _AngleEnd, lcnt > 0 ? (_IntervalTicks + 1) * (lcnt - 1) + 1 : 0, lcnt);
	lv_gauge_set_angle_offset(obj, _AngleEnd > _AngleStart ? 90 + _AngleStart + (_AngleEnd - _AngleStart) / 2 : 270 + _AngleEnd + (_AngleStart - _AngleEnd) / 2);
	lv_gauge_set_critical_value(obj, _CriticalValue);
	switch (_NeedleThickness)
	{
		case 0: lv_obj_set_style_local_line_width(obj, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, 2); break;
		case 1: lv_obj_set_style_local_line_width(obj, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, 4); break;
		case 2: lv_obj_set_style_local_line_width(obj, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, 7); break;
		case 3: lv_obj_set_style_local_line_width(obj, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, 10); break;
		case 4: lv_obj_set_style_local_line_width(obj, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, 13); break;
	}
	if (!_Background)
	{
		lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_border_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_outline_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
	}
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		needle_colors[0] = lv_color_hex(data->NeedleColor1 < 0 ? get_theme_color(0, 0) : (uint32_t)data->NeedleColor1);
		needle_colors[1] = lv_color_hex(data->NeedleColor2 < 0 ? get_theme_color(1, 0) : (uint32_t)data->NeedleColor2);
		needle_colors[2] = lv_color_hex(data->NeedleColor3 < 0 ? get_theme_color(2, 0) : (uint32_t)data->NeedleColor3);
		needle_colors[3] = lv_color_hex(data->NeedleColor4 < 0 ? get_theme_color(3, 0) : (uint32_t)data->NeedleColor4);
		lv_gauge_set_value(obj, 0, data->NeedleValue1);
		lv_gauge_set_value(obj, 1, data->NeedleValue2);
		lv_gauge_set_value(obj, 2, data->NeedleValue3);
		lv_gauge_set_value(obj, 3, data->NeedleValue4);
	}
	return obj;
}

lv_obj_t* ImageInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                    uint16_t _Opacity, bool _Hidden, HMI_Image_Data* data)
{
	lv_obj_t* obj = lv_img_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	lv_obj_set_style_local_image_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, _Opacity * 255 / 100);
	lv_obj_set_hidden(obj, _Hidden);
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		lv_img_set_src(obj, data->ImageSource);
	}
	return obj;
}

lv_obj_t* ImageBarInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                       double _TextColor, int8_t _TextFont, HMI_ImageBar_Data* data)
{
	lv_obj_t* obj = BarInit(parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, _TextColor, _TextFont, NULL);
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		if (data->MaxValue - data->MinValue >= 65535)
			lv_bar_set_range(obj, -32768, 32767);
		else if (data->MaxValue > 32767)
			lv_bar_set_range(obj, 32767 - (data->MaxValue - data->MinValue), 32767);
		else if (data->MinValue < -32768)
			lv_bar_set_range(obj, -32768, -32768 + (data->MaxValue - data->MinValue));
		else
			lv_bar_set_range(obj ,data->MinValue , data->MaxValue );
		lv_bar_set_value(obj, (data->Value - data->MinValue) / (data->MaxValue - data->MinValue) * (lv_bar_get_max_value(obj) - lv_bar_get_min_value(obj)) + lv_bar_get_min_value(obj), LV_ANIM_OFF);
		lv_obj_set_style_local_radius(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, 0);
		lv_obj_set_style_local_bg_opa(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_border_opa(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_shadow_opa(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_pattern_image(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, data->BgImageSource ? data->BgImageSource : (envPathCombine("IRSPATH", (char*)(_Width >= _Height ? "Image/hBar1.png" : "Image/vBar1.png"))));
		lv_obj_set_style_local_radius(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, 0);
		lv_obj_set_style_local_bg_opa(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_border_opa(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_shadow_opa(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_pattern_image(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, data->BarImageSource ? data->BarImageSource : (envPathCombine("IRSPATH", (char*)(_Width >= _Height ? "Image/hBar2.png" : "Image/vBar2.png"))));
		lv_obj_set_style_local_pad_left(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, _Width >= _Height ? data->Pad : 0);
		lv_obj_set_style_local_pad_right(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, _Width >= _Height ? data->Pad : 0);
		lv_obj_set_style_local_pad_top(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, _Width >= _Height ? 0 : data->Pad);
		lv_obj_set_style_local_pad_bottom(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, _Width >= _Height ? 0 : data->Pad);
		lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
	}
	return obj;
}

lv_obj_t* ImageButtonInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                          int8_t _ButtonType, uint16_t _Opacity, HMI_ImageButton_Data* data)
{
	lv_obj_t* obj = lv_imgbtn_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	if (_ButtonType != 1)
	{
		lv_obj_set_style_local_image_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, _Opacity * 255 / 100);
		lv_obj_set_style_local_image_recolor_opa(obj, LV_IMGBTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_30);
		lv_obj_set_style_local_image_recolor(obj, LV_IMGBTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_BLACK);
	}
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		char* path = data->ImageSource ? data->ImageSource : (envPathCombine("IRSPATH", "Image/ib1.png"));
		lv_imgbtn_set_src(obj, LV_BTN_STATE_RELEASED, path);
		if (_ButtonType == 0)
			lv_imgbtn_set_src(obj, LV_BTN_STATE_PRESSED, path);
		else
			lv_imgbtn_set_src(obj, LV_BTN_STATE_PRESSED, getZoomPicture(path, 0.9));
	}
	return obj;
}

lv_obj_t* ImageGaugeInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                         int8_t _NeedleCount, int32_t _MaxValue, int32_t _MinValue, int8_t _NeedleThickness, int8_t _CenterPointSize, uint16_t _AngleStart, uint16_t _AngleEnd, int8_t _LabelCnt, int8_t _IntervalTicks, uint16_t _CriticalValue, bool _Background, 
                         bool _NeedleMode, HMI_ImageGauge_Data* data)
{
	lv_obj_t* obj = GaugeInit(parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, _NeedleCount, _MaxValue, _MinValue, _NeedleThickness, _CenterPointSize, _AngleStart, _AngleEnd, _LabelCnt, _IntervalTicks, _CriticalValue, _Background, NULL);
	if (!_NeedleMode)
	{
		lv_obj_set_style_local_line_opa(obj, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_text_opa(obj, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_line_opa(obj, LV_GAUGE_PART_MAJOR, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_border_opa(obj, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_outline_opa(obj, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_bg_opa(obj, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_border_opa(obj, LV_GAUGE_PART_MAJOR, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_outline_opa(obj, LV_GAUGE_PART_MAJOR, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_bg_opa(obj, LV_GAUGE_PART_MAJOR, LV_STATE_DEFAULT, LV_OPA_0);
	}
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		if (data->NeedleImageSource)
		{
			lv_img_dsc_t * zoomImg = getZoomPicture(data->NeedleImageSource, 1.0);
			lv_gauge_set_needle_img(obj, zoomImg, 0, zoomImg->header.h / 2);
		}
		else
		{
			lv_img_dsc_t * zoomImg = getZoomPicture(envPathCombine("IRSPATH", "Image/ga01.png"), 1.0);
			lv_gauge_set_needle_img(obj, zoomImg, 0, zoomImg->header.h / 2);
		}
		lv_gauge_set_value(obj, 0, data->NeedleValue1);
	}
	return obj;
}

lv_obj_t* ImageSliderInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                          int16_t _MaxValue, int16_t _MinValue, int16_t _Value, double _TextColor, int8_t _TextFont, HMI_ImageSlider_Data* data)
{
	lv_obj_t* obj = SliderInit(parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, _MaxValue, _MinValue, _Value, _TextColor, _TextFont, NULL);
	bool hor = _Width >= _Height;
	int horLabelPos = 10 + (hor ? _Height / 2 : 0);
	int verLabelPos = 10 + (hor ? 0 : _Width / 2);
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		lv_obj_set_style_local_radius(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, 0);
		lv_obj_set_style_local_bg_opa(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_border_opa(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_shadow_opa(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_pattern_image(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, data->BgImageSource ? data->BgImageSource : (envPathCombine("IRSPATH", (char*)(_Width >= _Height ? "Image/hSl1.png" : "Image/vSl1.png"))));
		lv_obj_set_style_local_radius(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, 0);
		lv_obj_set_style_local_bg_opa(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_border_opa(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_shadow_opa(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_pattern_image(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, data->BarImageSource ? data->BarImageSource : (envPathCombine("IRSPATH", (char*)(_Width >= _Height ? "Image/hSl2.png" : "Image/vSl2.png"))));
		lv_obj_set_style_local_radius(obj, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 0);
		lv_obj_set_style_local_bg_opa(obj, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_border_opa(obj, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_shadow_opa(obj, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_0);
		lv_obj_set_style_local_pattern_image(obj, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, data->KnobImageSource ? data->KnobImageSource : (envPathCombine("IRSPATH", (char*)(_Width >= _Height ? "Image/hSl3.png" : "Image/vSl3.png"))));
		lv_obj_set_style_local_pad_left(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, _Width >= _Height ? data->Pad : 0);
		lv_obj_set_style_local_pad_right(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, _Width >= _Height ? data->Pad : 0);
		lv_obj_set_style_local_pad_top(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, _Width >= _Height ? 0 : data->Pad);
		lv_obj_set_style_local_pad_bottom(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, _Width >= _Height ? 0 : data->Pad);
	}
	return obj;
}

lv_obj_t* ImageToggleButtonInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                                int8_t _ButtonType, uint16_t _Opacity, 
                                bool _Checked, HMI_ImageToggleButton_Data* data)
{
	lv_obj_t* obj = ImageButtonInit(parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, _ButtonType, _Opacity, NULL);
	lv_imgbtn_set_checkable(obj, true);
	lv_imgbtn_set_state(obj, lv_obj_get_state(obj, LV_OBJ_PART_MAIN) & LV_STATE_DISABLED
	 ? (_Checked ? LV_BTN_STATE_CHECKED_DISABLED : LV_BTN_STATE_DISABLED)
	 : (_Checked ? LV_BTN_STATE_CHECKED_RELEASED : LV_BTN_STATE_RELEASED));
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		char* path = data->ImageSource ? data->ImageSource : (envPathCombine("IRSPATH", "Image/itb2.png"));
		char* path2 = data->ImageChecked ? data->ImageChecked : (envPathCombine("IRSPATH", "Image/itb1.png"));
		lv_imgbtn_set_src(obj, LV_BTN_STATE_RELEASED, path);
		if (_ButtonType == 0)
			lv_imgbtn_set_src(obj, LV_BTN_STATE_PRESSED, path);
		else
			lv_imgbtn_set_src(obj, LV_BTN_STATE_PRESSED, getZoomPicture(path, 0.9));
		lv_imgbtn_set_src(obj, LV_BTN_STATE_CHECKED_RELEASED, path2);
		if (_ButtonType == 0)
			lv_imgbtn_set_src(obj, LV_BTN_STATE_CHECKED_PRESSED, path2);
		else
			lv_imgbtn_set_src(obj, LV_BTN_STATE_CHECKED_PRESSED, getZoomPicture(path2, 0.9));
	}
	return obj;
}

lv_obj_t* LabelInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                    int8_t _TextAlign, double _TextColor, int8_t _TextFont, HMI_Label_Data* data)
{
	lv_obj_t* obj = lv_label_create(parent, NULL);
	lv_label_set_long_mode(obj, _TextAlign == LV_LABEL_LONG_EXPAND ? LV_LABEL_LONG_EXPAND : LV_LABEL_LONG_BREAK);
	lv_label_set_align(obj, _TextAlign - 1);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	if (_TextColor != -1)
		lv_obj_set_style_local_text_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(_TextColor));
	set_font_id(obj, _TextFont);
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		for (int i = 0; i < 9; i++)
			data->_textlist[i] = strdup(data->TextList[i]);
		LabelRefreshText(obj);
	}
	return obj;
}

void LabelRefreshText(lv_obj_t* o)
{
	HMI_Label_Data* d = (HMI_Label_Data*)lv_obj_get_user_data(o);
	lv_label_set_text_fmt(o, "%s%s%s", d->PrefixList[_LangID], d->_textlist[_LangID], d->SuffixlList[_LangID]);
}

lv_obj_t* LEDInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                  uint16_t _Bright, HMI_BasicObj_Data* data)
{
	lv_obj_t* obj = lv_led_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, data);
	lv_led_set_bright(obj, _Bright * 135 / 100 + 120);
	return obj;
}

lv_obj_t* ListInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                   int8_t _Spacing, int8_t _TextFont, HMI_List_Data* data)
{
	int space = 0;
	lv_obj_t * btn;
	char * str = "Item A";
	lv_obj_t* obj =  lv_list_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	set_font_id(obj, _TextFont);
	for (int i = 0; i < data->ItemCount + 2; i++)
	{
		if (data != NULL)
			btn = lv_list_add_btn(obj, strlen(data->Symbols[i]) ? data->Symbols[i] : NULL , data->ListItems[_LangID * 16 + i]);
		else
		{
			str[5] = 'A' + i;
			btn = lv_list_add_btn(obj, NULL , str);
		}
		if (_Spacing == 0) space = 0;
		else if (_Spacing == 1) space = 15;
		else if (_Spacing == 2) space = 20;
		else if (_Spacing == 3) space = 25;
		else if (_Spacing == 4) space = 40;
		lv_obj_set_style_local_pad_top(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, space);
		lv_obj_set_style_local_pad_bottom(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, space);
		set_font_id(btn, _TextFont);
		set_color_id(btn, _Color);
		lv_obj_set_user_data(btn, obj);
		lv_obj_set_event_cb(btn, list_btn_event_handler);
	}
	void (*enable_cb)(lv_obj_t*, lv_state_t) = _Enable ? lv_obj_clear_state : lv_obj_add_state;
	lv_obj_t* child = NULL;	while (child = lv_obj_get_child(obj, child))		enable_cb(child, LV_STATE_DISABLED);	child = NULL;	while (child = lv_list_get_next_btn(obj, child))		enable_cb(child, LV_STATE_DISABLED);	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		ListRefreshText(obj);
	}
	return obj;
}

void ListRefreshText(lv_obj_t* o)
{
	HMI_List_Data* d = (HMI_List_Data*)lv_obj_get_user_data(o);
	lv_obj_t * btn = NULL;
	while (btn = lv_list_get_next_btn(o, btn))
		lv_label_set_text(lv_list_get_btn_label(btn), d->ListItems[_LangID * 16 + lv_list_get_btn_index(o, btn)]);
}

void list_btn_event_handler(lv_obj_t* btn, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		EventNode btnEN = {btn, 0};
		lv_event_send((lv_obj_t*)lv_obj_get_user_data(btn), LV_EVENT_VALUE_CHANGED, &btnEN);
	}
}

lv_obj_t* NumberInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     int8_t _TextAlign, double _TextColor, int8_t _TextFont, HMI_Number_Data* data)
{
	lv_obj_t* obj = LabelInit(parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, _TextAlign, _TextColor, _TextFont, NULL);
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		NumberRefreshText(obj);
	}
	return obj;
}

void NumberRefreshText(lv_obj_t* o)
{
	HMI_Number_Data* d = (HMI_Number_Data*)lv_obj_get_user_data(o);
	char s[10] = "%s%.0lf%s";
	s[4] += d->Decimal;
	lv_label_set_text_fmt(o, s, d->PrefixList[_LangID], d->Value, d->SuffixlList[_LangID]);
}

lv_obj_t* NumberInputInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                          int8_t _TextFont, HMI_NumberInput_Data* data)
{
	lv_obj_t* obj = TextAreaInit(parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, _TextFont, NULL);
	lv_textarea_set_accepted_chars(obj, "0123456789.- ");
	lv_textarea_set_max_length(obj, 20);
	lv_textarea_set_one_line(obj, true);
	void (*enable_cb)(lv_obj_t*, lv_state_t) = _Enable ? lv_obj_clear_state : lv_obj_add_state;
	lv_obj_t* child = NULL;	while (child = lv_obj_get_child(obj, child))		enable_cb(child, LV_STATE_DISABLED);	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
	}
	return obj;
}

Oscilloscope* OscilloscopeInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                               int8_t _OscMode, int8_t _TickTextXOption, int8_t _TickTextYOption, int8_t _TextFont, uint16_t _DivLineV, uint16_t _DivLineH, int8_t _LeftPadSize, int8_t _RightPadSize, int8_t _BottomPadSize, int8_t _TopPadSize, int8_t _RealtimeMode, int8_t _DisplayQuality, int8_t _DivLineType, int8_t _BorderX, int8_t _BorderY, int8_t _BorderThickness, HMI_Oscilloscope_Data* data)
{
	Oscilloscope* obj = new Oscilloscope(_RealtimeMode == 0, false, data->SamplingCycle, data->PointCount, 3);
	lv_obj_t* o = obj->begin(parent, _X, _Y, _Width, _Height, _Enable, _DivLineV, _DivLineH, (OSC_DIVISION_LINE_TYPE)_DivLineType, (OSC_BORDER_SIDE_TYPE)_BorderX, (OSC_BORDER_SIDE_TYPE)_BorderY, (OSC_BORDER_THICKNESS_TYPE)_BorderThickness, _TickTextXOption, _TickTextYOption, data->TickX, data->TickY, data->TickSize, data->ChannelColor1 < 0 ? get_theme_color(0, 0) : data->ChannelColor1, data->TimeDiv, data->MinValueDouble, data->MaxValueDouble);
	int pad;
	obj->setHMITextFont(_TextFont);
	obj->setTriggerMode((OSC_TRIGGER_MODE)_OscMode);
	if (_DisplayQuality == 0)
	{
		obj->setFPS(2);
		obj->setDisplayQuality(5);
	}
	else if (_DisplayQuality == 1)
	{
		obj->setFPS(8);
		obj->setDisplayQuality(5);
	}
	else if (_DisplayQuality == 2)
	{
		obj->setFPS(15);
		obj->setDisplayQuality(5);
	}
	else if (_DisplayQuality == 3)
	{
		obj->setFPS(25);
		obj->setDisplayQuality(5);
	}
	else if (_DisplayQuality == 4)
	{
		obj->setFPS(15);
		obj->setDisplayQuality(2);
	}
	switch(_LeftPadSize)
	{
		case 0: pad = -1; break;
		case 1: pad = 8; break;
		case 2: pad = 13; break;
		case 3: pad = 50; break;
		case 4: pad = 100; break;
		case 5: pad = 150; break;
	}
	obj->setLeftPadSize(pad);
	switch(_RightPadSize)
	{
		case 0: pad = -1; break;
		case 1: pad = 8; break;
		case 2: pad = 13; break;
		case 3: pad = 50; break;
		case 4: pad = 100; break;
		case 5: pad = 150; break;
	}
	obj->setRightPadSize(pad);
	switch(_TopPadSize)
	{
		case 0: pad = -1; break;
		case 1: pad = 8; break;
		case 2: pad = 13; break;
		case 3: pad = 50; break;
		case 4: pad = 100; break;
		case 5: pad = 150; break;
	}
	obj->setTopPadSize(pad);
	switch(_BottomPadSize)
	{
		case 0: pad = -1; break;
		case 1: pad = 8; break;
		case 2: pad = 13; break;
		case 3: pad = 50; break;
		case 4: pad = 100; break;
		case 5: pad = 150; break;
	}
	obj->setBottomPadSize(pad);
	data->_osc = obj;
	lv_obj_set_user_data(o, data);
	return obj;
}

lv_obj_t* QrCodeInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     char* _LinkSource, HMI_BasicObj_Data* data)
{
	lv_obj_t* obj = lv_qrcode_create(parent, _Width, LV_COLOR_BLACK, LV_COLOR_WHITE);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	lv_qrcode_update(obj, _LinkSource, strlen(_LinkSource));
	lv_qrcode_set_private_data(obj, data);
	return obj;
}

lv_obj_t* RectangleInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                        double _BgColor, uint16_t _Opacity, HMI_BasicObj_Data* data)
{
	lv_obj_t* obj = lv_cont_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, data);
	if (_BgColor != -1)
	{
		lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex((uint32_t)_BgColor));
		lv_obj_set_style_local_bg_grad_dir(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
		lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, _Opacity * 255 / 100);
	}
	return obj;
}

lv_obj_t* RollerInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     int8_t _TextFont, HMI_Roller_Data* data)
{
	lv_obj_t* obj =  lv_roller_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	set_font_id(obj, _TextFont);
	void (*enable_cb)(lv_obj_t*, lv_state_t) = _Enable ? lv_obj_clear_state : lv_obj_add_state;
	lv_obj_t* child = NULL;	while (child = lv_obj_get_child(obj, child))		enable_cb(child, LV_STATE_DISABLED);	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		RollerRefreshText(obj);
	}
	return obj;
}

void RollerRefreshText(lv_obj_t* o)
{
	HMI_Roller_Data* d = (HMI_Roller_Data*)lv_obj_get_user_data(o);
	int index = lv_roller_get_selected(o);
	lv_roller_set_options(o, d->RollerText[_LangID], d->RollerMode);
	lv_roller_set_selected(o, lv_roller_get_option_cnt(o) > index ? index : 0, LV_ANIM_OFF);
}

lv_obj_t* SliderInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     int16_t _MaxValue, int16_t _MinValue, int16_t _Value, double _TextColor, int8_t _TextFont, HMI_Slider_Data* data)
{
	lv_obj_t* obj = lv_slider_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	lv_slider_set_range(obj, _MinValue, _MaxValue);
	lv_slider_set_value(obj, _Value, LV_ANIM_OFF);
	bool hor = _Width >= _Height;
	int horLabelPos = 10 + (hor ? _Height / 2 : 0);
	int verLabelPos = 10 + (hor ? 0 : _Width / 2);
	if (_TextColor != -1)
	{
		lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(_TextColor));
		lv_obj_set_style_local_value_color(obj, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_color_hex(_TextColor));
	}
	set_font_id(obj, _TextFont);
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
	if(data->ShowValue == 1)
	{
		lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_MID);
		lv_obj_set_style_local_value_ofs_y(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, -verLabelPos);
	}
	else if(data->ShowValue == 2)
	{
		lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
		lv_obj_set_style_local_value_ofs_y(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, verLabelPos);
	}
	else if(data->ShowValue == 3)
	{
		lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_LEFT_MID);
		lv_obj_set_style_local_value_ofs_x(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, -horLabelPos);
	}
	else if(data->ShowValue == 4)
	{
		lv_obj_set_style_local_value_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_RIGHT_MID);
		lv_obj_set_style_local_value_ofs_x(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, horLabelPos);
	}
	else if(data->ShowValue == 5)
	{
		if (_Width < _Height)
		{
			lv_obj_set_style_local_value_align(obj, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_ALIGN_OUT_LEFT_MID);
			lv_obj_set_style_local_value_ofs_x(obj, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, -horLabelPos);
		}
		else
		{
			lv_obj_set_style_local_value_align(obj, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_MID);
			lv_obj_set_style_local_value_ofs_y(obj, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, -verLabelPos);
		}
	}
		SliderRefreshText(obj);
	}
	return obj;
}

void SliderRefreshText(lv_obj_t* o)
{
	HMI_Slider_Data* d = (HMI_Slider_Data*)lv_obj_get_user_data(o);
	if (d->ShowValue)
	{
		char* buf = (char*)lv_obj_get_style_value_str(o, LV_OBJ_PART_MAIN);
		char* bufk = (char*)lv_obj_get_style_value_str(o, LV_SLIDER_PART_KNOB);
		int len = strlen(d->PrefixList[_LangID]) + strlen(d->SuffixList[_LangID]) + 7;
		if(buf == NULL)
			buf = (char*)malloc((len + 1) * sizeof(char));
		if(bufk == NULL)
			bufk = (char*)malloc((len + 1) * sizeof(char));
		sprintf(d->ShowValue == 5 ? bufk : buf, "%s%d%s", d->PrefixList[_LangID], lv_slider_get_value(o), d->SuffixList[_LangID]);
		sprintf(d->ShowValue == 5 ? buf : bufk, "");
		lv_obj_set_style_local_value_str(o, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, buf);
		lv_obj_set_style_local_value_str(o, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, bufk);
	}
}

lv_obj_t* SpinboxInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                      int8_t _ButtonType, int8_t _NumericLimit, int8_t _TextFont, HMI_Spinbox_Data* data)
{
	int h;
	lv_obj_t *sp, *btn1, *btn2, *btn3, *btn4, *l1, *l2, *l3, *l4;
	void (*enable_cb)(lv_obj_t*, lv_state_t) = _Enable ? lv_obj_clear_state : lv_obj_add_state;
	lv_obj_t* obj =  lv_obj_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
	lv_obj_set_style_local_border_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
	sp = lv_spinbox_create(obj, NULL);
	lv_spinbox_set_range(sp, _NumericLimit ? 0 : -2147483648, 2147483647);
	set_font_id(sp, _TextFont);
	((lv_spinbox_ext_t*)lv_obj_get_ext_attr(sp))->ta.cursor.hidden = true;
	h = lv_obj_get_height(sp);
	lv_obj_set_width(sp, _Width - (h * 2 + 10) * (_ButtonType < 2 ? 1 : 2));
	lv_obj_align(sp, NULL, LV_ALIGN_CENTER, 0, 0);
	btn1 = lv_btn_create(obj, NULL);
	btn2 = lv_btn_create(obj, NULL);
	l1 = lv_label_create(btn1, NULL);
	l2 = lv_label_create(btn2, NULL);
	set_font_id(l1, _TextFont);
	set_font_id(l2, _TextFont);
	lv_label_set_text(l1, _ButtonType % 2 == 0 ? LV_SYMBOL_PLUS : LV_SYMBOL_MINUS);
	lv_label_set_text(l2, _ButtonType % 2 == 0 ? LV_SYMBOL_MINUS : LV_SYMBOL_PLUS);
	lv_obj_set_size(btn1, h, h);
	lv_obj_set_size(btn2, h, h);
	lv_theme_apply(btn1, LV_THEME_SPINBOX_BTN);
	lv_theme_apply(btn2, LV_THEME_SPINBOX_BTN);
	lv_obj_align(btn1, sp, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_obj_align(btn2, sp, LV_ALIGN_OUT_LEFT_MID, -5, 0);
	lv_obj_set_user_data(sp, sp);
	lv_obj_set_user_data(btn1, sp);
	lv_obj_set_user_data(btn2, sp);
	lv_obj_refresh_style(sp, LV_OBJ_PART_MAIN, LV_STYLE_PROP_ALL);
	lv_obj_refresh_style(btn1, LV_OBJ_PART_MAIN, LV_STYLE_PROP_ALL);
	lv_obj_refresh_style(btn2, LV_OBJ_PART_MAIN, LV_STYLE_PROP_ALL);
	lv_obj_set_event_cb(btn1, lv_spinbox_event_cb);
	lv_obj_set_event_cb(btn2, lv_spinbox_event_cb);
	enable_cb(sp, LV_STATE_DISABLED);
	enable_cb(btn1, LV_STATE_DISABLED);
	enable_cb(btn2, LV_STATE_DISABLED);
	if (_ButtonType >= 2)
	{
		btn3 = lv_btn_create(obj, NULL);
		btn4 = lv_btn_create(obj, NULL);
		l3 = lv_label_create(btn3, NULL);
		l4 = lv_label_create(btn4, NULL);
		set_font_id(l3, _TextFont);
		set_font_id(l4, _TextFont);
		lv_label_set_text(l3, LV_SYMBOL_RIGHT);
		lv_label_set_text(l4, LV_SYMBOL_LEFT);
		lv_obj_set_size(btn3, h, h);
		lv_obj_set_size(btn4, h, h);
		lv_theme_apply(btn3, LV_THEME_SPINBOX_BTN);
		lv_theme_apply(btn4, LV_THEME_SPINBOX_BTN);
		lv_obj_align(btn3, btn1, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
		lv_obj_align(btn4, btn2, LV_ALIGN_OUT_LEFT_MID, -5, 0);
		lv_obj_set_user_data(btn3, sp);
		lv_obj_set_user_data(btn4, sp);
		lv_obj_refresh_style(btn3, LV_OBJ_PART_MAIN, LV_STYLE_PROP_ALL);
		lv_obj_refresh_style(btn4, LV_OBJ_PART_MAIN, LV_STYLE_PROP_ALL);
		lv_obj_set_event_cb(btn3, lv_spinbox_event_cb);
		lv_obj_set_event_cb(btn4, lv_spinbox_event_cb);
		enable_cb(btn3, LV_STATE_DISABLED);
		enable_cb(btn4, LV_STATE_DISABLED);
	}
	lv_obj_set_event_cb(sp, lv_spinbox_event_cb);
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		lv_spinbox_set_digit_format(sp, data->Digit + 1, data->Decimal > 0 ? data->Digit + 1 - data->Decimal : 0);
	}
	return obj;
}

void lv_spinbox_event_cb(lv_obj_t* obj, lv_event_t e)
{
	lv_obj_t* sp = (lv_obj_t*)lv_obj_get_user_data(obj);
	if (obj != sp && (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT))
	{
		char* txt = lv_label_get_text(lv_obj_get_child(obj, NULL));
		if(strcmp(txt, LV_SYMBOL_PLUS) == 0) lv_spinbox_increment(sp);
		else if(strcmp(txt, LV_SYMBOL_MINUS) == 0) lv_spinbox_decrement(sp);
		else if(strcmp(txt, LV_SYMBOL_RIGHT) == 0) lv_spinbox_step_next(sp);
		else if(strcmp(txt, LV_SYMBOL_LEFT) == 0) lv_spinbox_step_prev(sp);
	}
	else if (e == LV_EVENT_FOCUSED)
		((lv_spinbox_ext_t*)lv_obj_get_ext_attr(sp))->ta.cursor.hidden = false;
	else if (e == LV_EVENT_DEFOCUSED)
		((lv_spinbox_ext_t*)lv_obj_get_ext_attr(sp))->ta.cursor.hidden = true;
}

lv_obj_t* SwitchInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     bool _Checked, HMI_BasicObj_Data* data)
{
	lv_obj_t* obj = lv_switch_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, data);
	if(_Checked)
		lv_switch_on(obj, LV_ANIM_OFF);
	else
		lv_switch_off(obj, LV_ANIM_OFF);
	return obj;
}

lv_obj_t* TableInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                    uint16_t _RowCnt, int8_t _BaseTextAlign, uint16_t _ColCnt, int8_t _Header, double _BaseBgColor, double _BaseTextColor, int8_t _BaseBorderSide, int8_t _TextFont, bool _Background, uint16_t _Opacity, HMI_Table_Data* data)
{
	lv_obj_t* obj = lv_table_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, NULL);
	bool rhEn = _Header == 1 || _Header == 3;
	bool chEn = _Header == 2 || _Header == 3;
	lv_table_set_col_cnt(obj, _ColCnt);
	lv_table_set_row_cnt(obj, _RowCnt);
	if (_BaseTextColor != -1)
		lv_obj_set_style_local_text_color(obj, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, lv_color_hex(_BaseTextColor));
	if (_BaseBgColor != -1)
		lv_obj_set_style_local_bg_color(obj, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, lv_color_hex(_BaseBgColor));
	lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, _Opacity * 255 / 100);
	for (int i = LV_TABLE_PART_CELL1; i <= LV_TABLE_PART_CELL3; i++)
	{
		if(_BaseBorderSide == 0)
			lv_obj_set_style_local_border_side(obj, i, LV_STATE_DEFAULT, LV_BORDER_SIDE_NONE);
		else if(_BaseBorderSide == 1)
			lv_obj_set_style_local_border_side(obj, i, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_TOP);
		else if(_BaseBorderSide == 2)
			lv_obj_set_style_local_border_side(obj, i, LV_STATE_DEFAULT, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_RIGHT);
		else if(_BaseBorderSide == 3)
			lv_obj_set_style_local_border_side(obj, i, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL);
		lv_obj_set_style_local_bg_opa(obj, i, LV_STATE_DEFAULT, _Opacity * 255 / 100);
	}
	for (int i = rhEn ? 1 : 0; i < _ColCnt; i++)
		for (int j = chEn ? 1 : 0; j < _RowCnt; j++)
		{
			lv_table_set_cell_type(obj, j, i, LV_TABLE_PART_CELL1);
			lv_table_set_cell_align(obj, j, i, _BaseTextAlign);
		}
	set_font_id(obj, _TextFont);
	if(!_Background)
	{
		lv_obj_set_style_local_border_width(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
		lv_obj_set_style_local_pad_all(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
		lv_obj_set_style_local_pad_inner(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	}
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		if (data->ColHeaderTextColor != -1)
			lv_obj_set_style_local_text_color(obj, LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, lv_color_hex(data->ColHeaderTextColor));
		if (data->RowHeaderTextColor != -1)
			lv_obj_set_style_local_text_color(obj, LV_TABLE_PART_CELL3, LV_STATE_DEFAULT, lv_color_hex(data->RowHeaderTextColor));
		if (data->ColHeaderBgColor != -1)
			lv_obj_set_style_local_bg_color(obj, LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, lv_color_hex(data->ColHeaderBgColor));
		if (data->RowHeaderBgColor != -1)
			lv_obj_set_style_local_bg_color(obj, LV_TABLE_PART_CELL3, LV_STATE_DEFAULT, lv_color_hex(data->RowHeaderBgColor));
		for (int i = 0; i < _ColCnt && chEn; i++)
		{
			lv_table_set_cell_type(obj, 0, i, LV_TABLE_PART_CELL2);
			if (i == 0 && rhEn) continue;
			lv_table_set_cell_align(obj, 0, i, data->ColHeaderTextAlign);
		}
		for (int i = 0; i < _RowCnt && rhEn; i++)
		{
			lv_table_set_cell_align(obj, i, 0, data->RowHeaderTextAlign);
			if (i == 0 && chEn) continue;
			lv_table_set_cell_type(obj, i, 0, LV_TABLE_PART_CELL3);
		}
		lv_table_set_col_width(obj, 0, data->ColWidth0);
		lv_table_set_col_width(obj, 1, data->ColWidth1);
		lv_table_set_col_width(obj, 2, data->ColWidth2);
		lv_table_set_col_width(obj, 3, data->ColWidth3);
		lv_table_set_col_width(obj, 4, data->ColWidth4);
		lv_table_set_col_width(obj, 5, data->ColWidth5);
		lv_table_set_col_width(obj, 6, data->ColWidth6);
		lv_table_set_col_width(obj, 7, data->ColWidth7);
		lv_table_set_col_width(obj, 8, data->ColWidth8);
		lv_table_set_col_width(obj, 9, data->ColWidth9);
		lv_table_set_col_width(obj, 10, data->ColWidth10);
		for (int i = LV_TABLE_PART_CELL1; i <= LV_TABLE_PART_CELL3; i++)
		{
			lv_obj_set_style_local_pad_bottom(obj, i, LV_STATE_DEFAULT, data->LineSpacingGap);
			lv_obj_set_style_local_pad_top(obj, i, LV_STATE_DEFAULT, data->LineSpacingGap);
		}
		TableRefreshText(obj);
	}
	return obj;
}

void TableRefreshText(lv_obj_t* o)
{
	HMI_Table_Data* d = (HMI_Table_Data*)lv_obj_get_user_data(o);
	char *p1, *p2, buf[7];
	for (int i, j = 0; j < 2; j++)
	{
		p1 = !j ? d->RowHeaderTextList[_LangID] : d->ColHeaderTextList[_LangID];
		if (strcmp(p1, "(Auto)") == 0) continue;
		for (i = 0; (!j && i < lv_table_get_row_cnt(o)) || (j && i < lv_table_get_col_cnt(o)); i++)
		{
			if ((p2 = strstr(p1, ",\n")) == NULL)
			{
				lv_table_set_cell_value_fmt(o, !j * i, j * i, "%s", p1);
				break;
			}
			strncpy(buf, p1, p2 - p1);
			buf[p2 - p1] = '\0';
			p1 = p2 + strlen(",\n");
			lv_table_set_cell_value_fmt(o, !j * i, j * i, "%s", buf);
		}
	}
}

lv_obj_t* TextAreaInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                       int8_t _TextFont, HMI_BasicObj_Data* data)
{
	lv_obj_t* obj = lv_textarea_create(parent, NULL);
	BasicObjInit(obj, parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, data);
	set_font_id(obj, _TextFont);
	lv_textarea_set_text(obj, "");
	lv_textarea_set_cursor_hidden(obj, true);
	return obj;
}

void kb_create(lv_obj_t* o)
{
	kb = lv_keyboard_create(lv_layer_top(), NULL);
	lv_keyboard_set_cursor_manage(kb, true);
	lv_obj_set_event_cb(kb, kb_event_cb);
	lv_keyboard_set_textarea(kb, o);
}

void kb_event_cb(lv_obj_t* keyboard, lv_event_t e)
{
	lv_keyboard_def_event_cb(kb, e);
	if (e == LV_EVENT_APPLY || e == LV_EVENT_CANCEL)
	{
		if (lv_keyboard_get_textarea(kb) != NULL)
			lv_event_send(lv_keyboard_get_textarea(kb), e, NULL);
		lv_keyboard_set_textarea(kb, NULL);
		lv_obj_del(kb);
		kb = NULL;
	}
}

lv_obj_t* TextInputInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                        int8_t _TextFont, HMI_TextInput_Data* data)
{
	lv_obj_t* obj = TextAreaInit(parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, _TextFont, NULL);
	lv_textarea_set_one_line(obj, true);
	lv_textarea_set_max_length(obj, 255);
	void (*enable_cb)(lv_obj_t*, lv_state_t) = _Enable ? lv_obj_clear_state : lv_obj_add_state;
	lv_obj_t* child = NULL;
	while (child = lv_obj_get_child(obj, child))
		enable_cb(child, LV_STATE_DISABLED);
	if (data != NULL)
	{
		memset(data->_buffer, '\0', 256);
		lv_obj_set_user_data(obj, data);
	}
	return obj;
}

TextList* TextListInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                       int8_t _LineNumbers, int8_t _LineBorder, int8_t _LineSpacing, int8_t _TextIndent, int8_t _LineCount, double _NumColor, double _TextColor, int8_t _NumFont, int8_t _TextFont, double _OddLineColor, double _EvenLineColor, int8_t _FocusLineColor, HMI_TextList_Data* data)
{
	TextList* obj = new TextList();
	int spacing = 0, indent = 0, lnCnt = 0;
	if (_LineSpacing == 0) spacing = 0;
	else if (_LineSpacing == 1) spacing = 5;
	else if (_LineSpacing == 2) spacing = 10;
	else if (_LineSpacing == 3) spacing = 15;
	if (_LineCount == 0) lnCnt = 99;
	else if (_LineCount == 1) lnCnt = 999;
	else if (_LineCount == 2) lnCnt = 9999;
	lv_obj_t* o = obj->begin(parent, _X, _Y, _Width, _Height, _Enable, _LineNumbers == 0, 1, lnCnt, spacing);
	obj->setFocusLineColor(get_theme_color(_FocusLineColor, 0));
	obj->setOddLineColor(_OddLineColor);
	obj->setEvenLineColor(_EvenLineColor);
	obj->setNumColor(_NumColor);
	obj->setTextColor(_TextColor);
	obj->setNumFont(get_theme_font(_NumFont));
	obj->setTextFont(get_theme_font(_TextFont));
	if (_TextIndent == 0) indent = 0;
	else if (_TextIndent == 1) indent = 5;
	else if (_TextIndent == 2) indent = 10;
	else if (_TextIndent == 3) indent = 15;
	obj->setTextIndent(indent);
	if(_LineBorder == 0)
		obj->setBorderSide(LV_BORDER_SIDE_NONE);
	else if(_LineBorder == 1)
		obj->setBorderSide(LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_TOP);
	else if(_LineBorder == 2)
		obj->setBorderSide(LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_RIGHT);
	else if(_LineBorder == 3)
		obj->setBorderSide(LV_BORDER_SIDE_FULL);
	if (data != NULL)
	{
		data->_tl = obj;
		lv_obj_set_user_data(o, data);
	}
	return obj;
}

lv_obj_t* ToggleButtonInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                           int8_t _ButtonType, double _TextColor, int8_t _TextFont, 
                           bool _Checked, HMI_ToggleButton_Data* data)
{
	lv_obj_t* obj = ButtonInit(parent, _X, _Y, _Width, _Height, _Color, _Enable, _Order, _Checked, _TextColor, _TextFont, NULL);
	lv_btn_set_checkable(obj, true);
	if (data != NULL)
	{
		lv_obj_set_user_data(obj, data);
		ToggleButtonRefreshText(obj);
	}
	return obj;
}

void ToggleButtonRefreshText(lv_obj_t* o)
{
	HMI_ToggleButton_Data* d = (HMI_ToggleButton_Data*)lv_obj_get_user_data(o);
	lv_obj_t* label = lv_obj_get_child(o, NULL);
	char* s = lv_btn_get_state(o) == LV_BTN_STATE_CHECKED_RELEASED || lv_btn_get_state(o) == LV_BTN_STATE_CHECKED_PRESSED || lv_btn_get_state(o) == LV_BTN_STATE_CHECKED_DISABLED ? d->CheckedTextList[_LangID] : d->TextList[_LangID];
	if (strlen(d->Symbol) != 0)
		lv_label_set_text_fmt(label, "%s %s", d->Symbol, s);
	else
		lv_label_set_text_fmt(label, "%s", s);
}


/* Widgets event_handler function */
void tileviewEventCallback(lv_obj_t* o, lv_event_t e)
{
	lv_coord_t x, y;
	EventNode leaveEN, enterEN;
	if (e == LV_EVENT_VALUE_CHANGED)
	{
		lv_tileview_get_tile_act(tileview, &x, &y);
		if (x == 0) enterEN = {p1, 0};
		if (x == 1) enterEN = {p2, 0};
		if (x == 2) enterEN = {p3, 0};
		if (x == 3) enterEN = {p4, 0};
		if (x == 4) enterEN = {p5, 0};
		if (x == 5) enterEN = {p6, 0};
		if (x == 6) enterEN = {p7, 0};
		if (enterEN.obj == currentPage) return;
		leaveEN = {currentPage, 1};
		lv_event_send(leaveEN.obj, LV_EVENT_VALUE_CHANGED, &leaveEN);
		lv_event_send(enterEN.obj, LV_EVENT_VALUE_CHANGED, &enterEN);
		currentPage = enterEN.obj;
	}
}

void p1EventCallback(lv_obj_t* o, lv_event_t e)
{
	HMI_SlidePage_Data* d = (HMI_SlidePage_Data*)lv_obj_get_user_data(o);
	EventNode* en = (EventNode*)lv_event_get_data();
	if(e == LV_EVENT_VALUE_CHANGED && en != NULL)
	{
		if(en->eventId == 0)
		{
			d->_t = timer_NowTime();
			d->_en = d->TimerEnable;
			eventQueuePush({o, 0});
		}
		else if(en->eventId == 1)
		{
			eventQueuePush({o, 1});
		}
		else if(en->eventId == 2)
		{
			eventQueuePush({o, 2});
		}
	}
}

void p1b1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p1b2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p2EventCallback(lv_obj_t* o, lv_event_t e)
{
	HMI_SlidePage_Data* d = (HMI_SlidePage_Data*)lv_obj_get_user_data(o);
	EventNode* en = (EventNode*)lv_event_get_data();
	if(e == LV_EVENT_VALUE_CHANGED && en != NULL)
	{
		if(en->eventId == 0)
		{
			d->_t = timer_NowTime();
			d->_en = d->TimerEnable;
			eventQueuePush({o, 0});
		}
		else if(en->eventId == 1)
		{
			eventQueuePush({o, 1});
		}
		else if(en->eventId == 2)
		{
			eventQueuePush({o, 2});
		}
	}
}

void p2b1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p2b2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p2b3EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p2tb1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		ToggleButtonRefreshText(p2tb1);
		if(lv_btn_get_state(p2tb1) == LV_BTN_STATE_CHECKED_RELEASED || lv_btn_get_state(p2tb1) == LV_BTN_STATE_CHECKED_PRESSED || lv_btn_get_state(p2tb1) == LV_BTN_STATE_CHECKED_DISABLED)
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p2ib1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p2itb1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_imgbtn_get_state(p2itb1) == LV_BTN_STATE_CHECKED_RELEASED || lv_imgbtn_get_state(p2itb1) == LV_BTN_STATE_CHECKED_PRESSED || lv_imgbtn_get_state(p2itb1) == LV_BTN_STATE_CHECKED_DISABLED)
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p2b4EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p2tb2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		ToggleButtonRefreshText(p2tb2);
		if(lv_btn_get_state(p2tb2) == LV_BTN_STATE_CHECKED_RELEASED || lv_btn_get_state(p2tb2) == LV_BTN_STATE_CHECKED_PRESSED || lv_btn_get_state(p2tb2) == LV_BTN_STATE_CHECKED_DISABLED)
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p2ib2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p2itb2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_imgbtn_get_state(p2itb2) == LV_BTN_STATE_CHECKED_RELEASED || lv_imgbtn_get_state(p2itb2) == LV_BTN_STATE_CHECKED_PRESSED || lv_imgbtn_get_state(p2itb2) == LV_BTN_STATE_CHECKED_DISABLED)
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p2b5EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p2b6EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p2tb3EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		ToggleButtonRefreshText(p2tb3);
		if(lv_btn_get_state(p2tb3) == LV_BTN_STATE_CHECKED_RELEASED || lv_btn_get_state(p2tb3) == LV_BTN_STATE_CHECKED_PRESSED || lv_btn_get_state(p2tb3) == LV_BTN_STATE_CHECKED_DISABLED)
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p2tb4EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		ToggleButtonRefreshText(p2tb4);
		if(lv_btn_get_state(p2tb4) == LV_BTN_STATE_CHECKED_RELEASED || lv_btn_get_state(p2tb4) == LV_BTN_STATE_CHECKED_PRESSED || lv_btn_get_state(p2tb4) == LV_BTN_STATE_CHECKED_DISABLED)
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p2itb3EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_imgbtn_get_state(p2itb3) == LV_BTN_STATE_CHECKED_RELEASED || lv_imgbtn_get_state(p2itb3) == LV_BTN_STATE_CHECKED_PRESSED || lv_imgbtn_get_state(p2itb3) == LV_BTN_STATE_CHECKED_DISABLED)
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p2itb4EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_imgbtn_get_state(p2itb4) == LV_BTN_STATE_CHECKED_RELEASED || lv_imgbtn_get_state(p2itb4) == LV_BTN_STATE_CHECKED_PRESSED || lv_imgbtn_get_state(p2itb4) == LV_BTN_STATE_CHECKED_DISABLED)
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p2ib3EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p2ib4EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p3EventCallback(lv_obj_t* o, lv_event_t e)
{
	HMI_SlidePage_Data* d = (HMI_SlidePage_Data*)lv_obj_get_user_data(o);
	EventNode* en = (EventNode*)lv_event_get_data();
	if(e == LV_EVENT_VALUE_CHANGED && en != NULL)
	{
		if(en->eventId == 0)
		{
			d->_t = timer_NowTime();
			d->_en = d->TimerEnable;
			eventQueuePush({o, 0});
		}
		else if(en->eventId == 1)
		{
			eventQueuePush({o, 1});
		}
		else if(en->eventId == 2)
		{
			eventQueuePush({o, 2});
		}
	}
}

void p3b1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p3b2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p3sl2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		SliderRefreshText(p3sl2);
		eventQueuePush({o, 0});
	}
	if(e == LV_EVENT_RELEASED)
	{
		eventQueuePush({o, 1});
	}
}

void p3sl3EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		SliderRefreshText(p3sl3);
		eventQueuePush({o, 0});
	}
	if(e == LV_EVENT_RELEASED)
	{
		eventQueuePush({o, 1});
	}
}

void p3isl1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		eventQueuePush({o, 0});
	}
	if(e == LV_EVENT_RELEASED)
	{
		eventQueuePush({o, 1});
	}
}

void p3asl1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		ArcSliderRefreshText(p3asl1);
		eventQueuePush({o, 0});
	}
	if(e == LV_EVENT_RELEASED)
	{
		eventQueuePush({o, 1});
	}
}

void p3asl2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		ArcSliderRefreshText(p3asl2);
		eventQueuePush({o, 0});
	}
	if(e == LV_EVENT_RELEASED)
	{
		eventQueuePush({o, 1});
	}
}

void p3sw1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_switch_get_state(p3sw1))
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p3sw2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_switch_get_state(p3sw2))
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p3cb1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_checkbox_is_checked(p3cb1))
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p3cb2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_checkbox_is_checked(p3cb2))
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p3cb3EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_checkbox_is_checked(p3cb3))
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p3cb4EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_checkbox_is_checked(p3cb4))
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p3sw3EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_switch_get_state(p3sw3))
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p3sw4EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_switch_get_state(p3sw4))
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p3sw5EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_switch_get_state(p3sw5))
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p3sw6EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_switch_get_state(p3sw6))
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p3sw7EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_switch_get_state(p3sw7))
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p3sw8EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		if(lv_switch_get_state(p3sw8))
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p4EventCallback(lv_obj_t* o, lv_event_t e)
{
	HMI_SlidePage_Data* d = (HMI_SlidePage_Data*)lv_obj_get_user_data(o);
	EventNode* en = (EventNode*)lv_event_get_data();
	if(e == LV_EVENT_VALUE_CHANGED && en != NULL)
	{
		if(en->eventId == 0)
		{
			d->_t = timer_NowTime();
			d->_en = d->TimerEnable;
			eventQueuePush({o, 0});
		}
		else if(en->eventId == 1)
		{
			eventQueuePush({o, 1});
		}
		else if(en->eventId == 2)
		{
			eventQueuePush({o, 2});
		}
	}
}

void p4b1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p4b2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p5EventCallback(lv_obj_t* o, lv_event_t e)
{
	HMI_SlidePage_Data* d = (HMI_SlidePage_Data*)lv_obj_get_user_data(o);
	EventNode* en = (EventNode*)lv_event_get_data();
	if(e == LV_EVENT_VALUE_CHANGED && en != NULL)
	{
		if(en->eventId == 0)
		{
			d->_t = timer_NowTime();
			d->_en = d->TimerEnable;
			eventQueuePush({o, 0});
		}
		else if(en->eventId == 1)
		{
			eventQueuePush({o, 1});
		}
		else if(en->eventId == 2)
		{
			eventQueuePush({o, 2});
		}
	}
}

void p5b1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p5b2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p6EventCallback(lv_obj_t* o, lv_event_t e)
{
	HMI_SlidePage_Data* d = (HMI_SlidePage_Data*)lv_obj_get_user_data(o);
	EventNode* en = (EventNode*)lv_event_get_data();
	if(e == LV_EVENT_VALUE_CHANGED && en != NULL)
	{
		if(en->eventId == 0)
		{
			d->_t = timer_NowTime();
			d->_en = d->TimerEnable;
			eventQueuePush({o, 0});
		}
		else if(en->eventId == 1)
		{
			eventQueuePush({o, 1});
		}
		else if(en->eventId == 2)
		{
			eventQueuePush({o, 2});
		}
	}
}

void p6b1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p6b2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p6tin1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if (e == LV_EVENT_APPLY)
	{
		eventQueuePush({o, 0});
	}
	else if (e == LV_EVENT_CANCEL)
	{
		eventQueuePush({o, 1});
	}
	else if (e == LV_EVENT_CLICKED)
	{
		if(kb == NULL)
			kb_create(o);
		else
			lv_keyboard_set_textarea(kb, o);
		lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
	}
}

void p6nin1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if (e == LV_EVENT_APPLY)
	{
		eventQueuePush({o, 0});
	}
	else if (e == LV_EVENT_CANCEL)
	{
		eventQueuePush({o, 1});
	}
	else if (e == LV_EVENT_CLICKED)
	{
		if(kb == NULL)
			kb_create(o);
		else
			lv_keyboard_set_textarea(kb, o);
		lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUM);
	}
	else if (e == LV_EVENT_VALUE_CHANGED)
	{
		const char* txt = lv_textarea_get_text(o);
		int i = 0, j = 0, k = 0;
		for (; txt[i] != '\0'; i++)
		{
			if (txt[i] == '.') j++;
			if (txt[i] == '-') k = i;
		}
		if (j > ((HMI_NumberInput_Data*)lv_obj_get_user_data(o))->ValueType || k > 0) lv_textarea_del_char(o);
	}
}

void p6nin2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if (e == LV_EVENT_APPLY)
	{
		eventQueuePush({o, 0});
	}
	else if (e == LV_EVENT_CANCEL)
	{
		eventQueuePush({o, 1});
	}
	else if (e == LV_EVENT_CLICKED)
	{
		if(kb == NULL)
			kb_create(o);
		else
			lv_keyboard_set_textarea(kb, o);
		lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUM);
	}
	else if (e == LV_EVENT_VALUE_CHANGED)
	{
		const char* txt = lv_textarea_get_text(o);
		int i = 0, j = 0, k = 0;
		for (; txt[i] != '\0'; i++)
		{
			if (txt[i] == '.') j++;
			if (txt[i] == '-') k = i;
		}
		if (j > ((HMI_NumberInput_Data*)lv_obj_get_user_data(o))->ValueType || k > 0) lv_textarea_del_char(o);
	}
}

void p6tin2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if (e == LV_EVENT_APPLY)
	{
		eventQueuePush({o, 0});
	}
	else if (e == LV_EVENT_CANCEL)
	{
		eventQueuePush({o, 1});
	}
	else if (e == LV_EVENT_CLICKED)
	{
		if(kb == NULL)
			kb_create(o);
		else
			lv_keyboard_set_textarea(kb, o);
		lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
	}
}

void p6din1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if (e == LV_EVENT_CLICKED)
	{
		if (kb != NULL)
			lv_event_send(kb, LV_EVENT_CANCEL, NULL);
		if (cal == NULL)
			cal_create(o);
	}
}

void p6din2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if (e == LV_EVENT_CLICKED)
	{
		if (kb != NULL)
			lv_event_send(kb, LV_EVENT_CANCEL, NULL);
		if (cal == NULL)
			cal_create(o);
	}
}

void p7EventCallback(lv_obj_t* o, lv_event_t e)
{
	HMI_SlidePage_Data* d = (HMI_SlidePage_Data*)lv_obj_get_user_data(o);
	EventNode* en = (EventNode*)lv_event_get_data();
	if(e == LV_EVENT_VALUE_CHANGED && en != NULL)
	{
		if(en->eventId == 0)
		{
			d->_t = timer_NowTime();
			d->_en = d->TimerEnable;
			eventQueuePush({o, 0});
		}
		else if(en->eventId == 1)
		{
			eventQueuePush({o, 1});
		}
		else if(en->eventId == 2)
		{
			eventQueuePush({o, 2});
		}
	}
}

void p7b1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p7b2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_CLICKED)
	{
		eventQueuePush({o, 0});
	}
	else if(e == LV_EVENT_PRESSING)
	{
		eventQueuePush({o, 1});
	}
}

void p7ddl1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		eventQueuePush({o, 0});
	}
}

void p7ddl2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		eventQueuePush({o, 0});
	}
}

void p7li1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		eventQueuePush({o, 0});
	}
}

void p7ro1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		eventQueuePush({o, 0});
	}
}

void p7ro2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		eventQueuePush({o, 0});
	}
}

void popUpMsgBoxEventCallback(lv_obj_t* o, lv_event_t e)
{
	if (e == LV_EVENT_VALUE_CHANGED)
	{
		if (!popUpBlock)
		{
			lv_obj_del(popUpMsgBox);
			popUpMsgBox = NULL;
			lv_obj_set_click(lv_layer_top(), false);
			lv_obj_set_style_local_bg_opa(lv_layer_top(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
		}
		else
			popUpBlock = false;
	}
}