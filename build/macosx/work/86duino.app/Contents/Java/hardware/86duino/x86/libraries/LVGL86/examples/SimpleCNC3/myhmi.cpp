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
	char* Name;
	int8_t TimerMode;
	uint32_t TimerInterval;
	bool TimerEnable;
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

struct HMI_Button_Data
{
	char* Name;
	char* Symbol;
	char* TextList[9];
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

struct HMI_Label_Data
{
	char* Name;
	char* TextList[9];
	char* PrefixList[9];
	char* SuffixlList[9];
	char* _textlist[9];
};

struct HMI_Number_Data
{
	char* Name;
	char* PrefixList[9];
	char* SuffixlList[9];
	int8_t Decimal;
	double Value;
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
lv_obj_t *p1, *p1rect5, *p1rect1, *p1rect2, *p1rect3, *p1rect4, *p1b1, *p1b2, *p1b3, *p1b4, *p1b5, *p1b6, *p1abar1, *p1abar2, *p1abar3, *p1led1, *p1led2, *p1led3, *p1num1, *p1num2, *p1num3, *p1num4, *p1num5, *p1num6, *p1l1, *p1l2, *p1l3, *p1l4, *p1l5, *p1l6, *p1b7, *p1ddl1, *p1l7, *p1b8, *p1tb1, *p1tb2, *p1ddl2, *p1l8, *p1b12, *p1b14;
lv_obj_t *p2, *p2rect4, *p2b1, *p2b2, *p2b5, *p2l7, *p2b7;
lv_obj_t *p4, *p4rect1, *p4rect2, *p4rect3, *p4rect5, *p4abar1, *p4abar2, *p4abar3, *p4led1, *p4led2, *p4led3, *p4num1, *p4num2, *p4num3, *p4num4, *p4num5, *p4num6, *p4l1, *p4l2, *p4l3, *p4rect4, *p4l4, *p4l5, *p4l6, *p4b1, *p4tb1, *p4b5, *p4b6, *p4b7, *p4b9;
lv_obj_t *p6, *p6rect1, *p6b1, *p6b2, *p6b4, *p6rect3, *p6l1, *p6num1, *p6num2, *p6l2, *p6l3, *p6num3, *p6l4, *p6num4, *p6l5, *p6num5, *p6l6, *p6rect4, *p6l7, *p6num6, *p6num7, *p6l8, *p6l9, *p6num8, *p6l10, *p6num9, *p6l11, *p6num10, *p6l12, *p6l13, *p6num11, *p6l14, *p6num12;
FileBrowser *p2fb1;
TextList *p4tl1;
Oscilloscope *p6osc1, *p6osc2;
static lv_obj_t* tileview;
static lv_obj_t* currentPage;
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

lv_obj_t* ButtonInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     int8_t _ButtonType, double _TextColor, int8_t _TextFont, HMI_Button_Data* data);

void ButtonRefreshText(lv_obj_t* o);

lv_obj_t* DropDownListInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                           int8_t _TextFont, HMI_DropDownList_Data* data);

void DropDownListRefreshText(lv_obj_t* o);

FileBrowser* FileBrowserInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                             int8_t _Spacing, int8_t _TextFont, HMI_FileBrowser_Data* data);

lv_obj_t* LabelInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                    int8_t _TextAlign, double _TextColor, int8_t _TextFont, HMI_Label_Data* data);

void LabelRefreshText(lv_obj_t* o);

lv_obj_t* LEDInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                  uint16_t _Bright, HMI_BasicObj_Data* data);

lv_obj_t* NumberInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                     int8_t _TextAlign, double _TextColor, int8_t _TextFont, HMI_Number_Data* data);

void NumberRefreshText(lv_obj_t* o);

Oscilloscope* OscilloscopeInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                               int8_t _OscMode, int8_t _TickTextXOption, int8_t _TickTextYOption, int8_t _TextFont, uint16_t _DivLineV, uint16_t _DivLineH, int8_t _LeftPadSize, int8_t _RightPadSize, int8_t _BottomPadSize, int8_t _TopPadSize, int8_t _RealtimeMode, int8_t _DisplayQuality, int8_t _DivLineType, int8_t _BorderX, int8_t _BorderY, int8_t _BorderThickness, HMI_Oscilloscope_Data* data);

lv_obj_t* RectangleInit(lv_obj_t* parent, int32_t _X, int32_t _Y, int32_t _Width, int32_t _Height, int8_t _Color, bool _Enable, int32_t _Order, 
                        double _BgColor, uint16_t _Opacity, HMI_BasicObj_Data* data);

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
void p1b3EventCallback(lv_obj_t* o, lv_event_t e);
void p1b4EventCallback(lv_obj_t* o, lv_event_t e);
void p1b5EventCallback(lv_obj_t* o, lv_event_t e);
void p1b6EventCallback(lv_obj_t* o, lv_event_t e);
void p1b7EventCallback(lv_obj_t* o, lv_event_t e);
void p1ddl1EventCallback(lv_obj_t* o, lv_event_t e);
void p1b8EventCallback(lv_obj_t* o, lv_event_t e);
void p1tb1EventCallback(lv_obj_t* o, lv_event_t e);
void p1tb2EventCallback(lv_obj_t* o, lv_event_t e);
void p1ddl2EventCallback(lv_obj_t* o, lv_event_t e);
void p1b12EventCallback(lv_obj_t* o, lv_event_t e);
void p1b14EventCallback(lv_obj_t* o, lv_event_t e);
void p2EventCallback(lv_obj_t* o, lv_event_t e);
void p2b1EventCallback(lv_obj_t* o, lv_event_t e);
void p2b2EventCallback(lv_obj_t* o, lv_event_t e);
void p2b5EventCallback(lv_obj_t* o, lv_event_t e);
void p2b7EventCallback(lv_obj_t* o, lv_event_t e);
void p4EventCallback(lv_obj_t* o, lv_event_t e);
void p4b1EventCallback(lv_obj_t* o, lv_event_t e);
void p4tb1EventCallback(lv_obj_t* o, lv_event_t e);
void p4b5EventCallback(lv_obj_t* o, lv_event_t e);
void p4b6EventCallback(lv_obj_t* o, lv_event_t e);
void p4b7EventCallback(lv_obj_t* o, lv_event_t e);
void p4b9EventCallback(lv_obj_t* o, lv_event_t e);
void p6EventCallback(lv_obj_t* o, lv_event_t e);
void p6b1EventCallback(lv_obj_t* o, lv_event_t e);
void p6b2EventCallback(lv_obj_t* o, lv_event_t e);
void p6b4EventCallback(lv_obj_t* o, lv_event_t e);
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
			case HMI_ABAR:
				switch(sn / 100000)
				{
					case 1: return p1abar1;
					case 2: return p1abar2;
					case 3: return p1abar3;
				}
			case HMI_B:
				switch(sn / 100000)
				{
					case 1: return p1b1;
					case 2: return p1b2;
					case 3: return p1b3;
					case 4: return p1b4;
					case 5: return p1b5;
					case 6: return p1b6;
					case 7: return p1b7;
					case 8: return p1b8;
					case 12: return p1b12;
					case 14: return p1b14;
				}
			case HMI_DDL:
				switch(sn / 100000)
				{
					case 1: return p1ddl1;
					case 2: return p1ddl2;
				}
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
				}
			case HMI_LED:
				switch(sn / 100000)
				{
					case 1: return p1led1;
					case 2: return p1led2;
					case 3: return p1led3;
				}
			case HMI_NUM:
				switch(sn / 100000)
				{
					case 1: return p1num1;
					case 2: return p1num2;
					case 3: return p1num3;
					case 4: return p1num4;
					case 5: return p1num5;
					case 6: return p1num6;
				}
			case HMI_RECT:
				switch(sn / 100000)
				{
					case 5: return p1rect5;
					case 1: return p1rect1;
					case 2: return p1rect2;
					case 3: return p1rect3;
					case 4: return p1rect4;
				}
			case HMI_TB:
				switch(sn / 100000)
				{
					case 1: return p1tb1;
					case 2: return p1tb2;
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
					case 5: return p2b5;
					case 7: return p2b7;
				}
			case HMI_FB: return p2fb1->getObj();
			case HMI_L: return p2l7;
			case HMI_RECT: return p2rect4;
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
					case 3: return p4abar3;
				}
			case HMI_B:
				switch(sn / 100000)
				{
					case 1: return p4b1;
					case 5: return p4b5;
					case 6: return p4b6;
					case 7: return p4b7;
					case 9: return p4b9;
				}
			case HMI_L:
				switch(sn / 100000)
				{
					case 1: return p4l1;
					case 2: return p4l2;
					case 3: return p4l3;
					case 4: return p4l4;
					case 5: return p4l5;
					case 6: return p4l6;
				}
			case HMI_LED:
				switch(sn / 100000)
				{
					case 1: return p4led1;
					case 2: return p4led2;
					case 3: return p4led3;
				}
			case HMI_NUM:
				switch(sn / 100000)
				{
					case 1: return p4num1;
					case 2: return p4num2;
					case 3: return p4num3;
					case 4: return p4num4;
					case 5: return p4num5;
					case 6: return p4num6;
				}
			case HMI_RECT:
				switch(sn / 100000)
				{
					case 1: return p4rect1;
					case 2: return p4rect2;
					case 3: return p4rect3;
					case 5: return p4rect5;
					case 4: return p4rect4;
				}
			case HMI_TL: return p4tl1->getObj();
			case HMI_TB: return p4tb1;
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
					case 4: return p6b4;
				}
			case HMI_L:
				switch(sn / 100000)
				{
					case 1: return p6l1;
					case 2: return p6l2;
					case 3: return p6l3;
					case 4: return p6l4;
					case 5: return p6l5;
					case 6: return p6l6;
					case 7: return p6l7;
					case 8: return p6l8;
					case 9: return p6l9;
					case 10: return p6l10;
					case 11: return p6l11;
					case 12: return p6l12;
					case 13: return p6l13;
					case 14: return p6l14;
				}
			case HMI_NUM:
				switch(sn / 100000)
				{
					case 1: return p6num1;
					case 2: return p6num2;
					case 3: return p6num3;
					case 4: return p6num4;
					case 5: return p6num5;
					case 6: return p6num6;
					case 7: return p6num7;
					case 8: return p6num8;
					case 9: return p6num9;
					case 10: return p6num10;
					case 11: return p6num11;
					case 12: return p6num12;
				}
			case HMI_OSC:
				switch(sn / 100000)
				{
					case 1: return p6osc1->getObj();
					case 2: return p6osc2->getObj();
				}
			case HMI_RECT:
				switch(sn / 100000)
				{
					case 1: return p6rect1;
					case 3: return p6rect3;
					case 4: return p6rect4;
				}
		}
	}
	return NULL;
}

lv_obj_t* MyHMI::NAME_TO_ID(char* name)
{
	if (strcmp(name, ((HMI_SlidePage_Data*)lv_obj_get_user_data(p1))->Name) == 0) return p1;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect5))->Name) == 0) return p1rect5;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect1))->Name) == 0) return p1rect1;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect2))->Name) == 0) return p1rect2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect3))->Name) == 0) return p1rect3;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect4))->Name) == 0) return p1rect4;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p1b1))->Name) == 0) return p1b1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p1b2))->Name) == 0) return p1b2;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p1b3))->Name) == 0) return p1b3;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p1b4))->Name) == 0) return p1b4;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p1b5))->Name) == 0) return p1b5;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p1b6))->Name) == 0) return p1b6;
	if (strcmp(name, ((HMI_Arc_Data*)lv_obj_get_user_data(p1abar1))->Name) == 0) return p1abar1;
	if (strcmp(name, ((HMI_Arc_Data*)lv_obj_get_user_data(p1abar2))->Name) == 0) return p1abar2;
	if (strcmp(name, ((HMI_Arc_Data*)lv_obj_get_user_data(p1abar3))->Name) == 0) return p1abar3;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1led1))->Name) == 0) return p1led1;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1led2))->Name) == 0) return p1led2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1led3))->Name) == 0) return p1led3;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p1num1))->Name) == 0) return p1num1;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p1num2))->Name) == 0) return p1num2;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p1num3))->Name) == 0) return p1num3;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p1num4))->Name) == 0) return p1num4;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p1num5))->Name) == 0) return p1num5;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p1num6))->Name) == 0) return p1num6;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l1))->Name) == 0) return p1l1;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l2))->Name) == 0) return p1l2;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l3))->Name) == 0) return p1l3;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l4))->Name) == 0) return p1l4;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l5))->Name) == 0) return p1l5;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l6))->Name) == 0) return p1l6;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p1b7))->Name) == 0) return p1b7;
	if (strcmp(name, ((HMI_DropDownList_Data*)lv_obj_get_user_data(p1ddl1))->Name) == 0) return p1ddl1;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l7))->Name) == 0) return p1l7;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p1b8))->Name) == 0) return p1b8;
	if (strcmp(name, ((HMI_ToggleButton_Data*)lv_obj_get_user_data(p1tb1))->Name) == 0) return p1tb1;
	if (strcmp(name, ((HMI_ToggleButton_Data*)lv_obj_get_user_data(p1tb2))->Name) == 0) return p1tb2;
	if (strcmp(name, ((HMI_DropDownList_Data*)lv_obj_get_user_data(p1ddl2))->Name) == 0) return p1ddl2;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p1l8))->Name) == 0) return p1l8;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p1b12))->Name) == 0) return p1b12;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p1b14))->Name) == 0) return p1b14;
	if (strcmp(name, ((HMI_SlidePage_Data*)lv_obj_get_user_data(p2))->Name) == 0) return p2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p2rect4))->Name) == 0) return p2rect4;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p2b1))->Name) == 0) return p2b1;
	if (strcmp(name, ((HMI_FileBrowser_Data*)get_list_user_data(p2fb1->getObj()))->Name) == 0) return p2fb1->getObj();
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p2b2))->Name) == 0) return p2b2;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p2b5))->Name) == 0) return p2b5;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p2l7))->Name) == 0) return p2l7;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p2b7))->Name) == 0) return p2b7;
	if (strcmp(name, ((HMI_SlidePage_Data*)lv_obj_get_user_data(p4))->Name) == 0) return p4;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect1))->Name) == 0) return p4rect1;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect2))->Name) == 0) return p4rect2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect3))->Name) == 0) return p4rect3;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect5))->Name) == 0) return p4rect5;
	if (strcmp(name, ((HMI_Arc_Data*)lv_obj_get_user_data(p4abar1))->Name) == 0) return p4abar1;
	if (strcmp(name, ((HMI_Arc_Data*)lv_obj_get_user_data(p4abar2))->Name) == 0) return p4abar2;
	if (strcmp(name, ((HMI_Arc_Data*)lv_obj_get_user_data(p4abar3))->Name) == 0) return p4abar3;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4led1))->Name) == 0) return p4led1;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4led2))->Name) == 0) return p4led2;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4led3))->Name) == 0) return p4led3;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p4num1))->Name) == 0) return p4num1;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p4num2))->Name) == 0) return p4num2;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p4num3))->Name) == 0) return p4num3;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p4num4))->Name) == 0) return p4num4;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p4num5))->Name) == 0) return p4num5;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p4num6))->Name) == 0) return p4num6;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p4l1))->Name) == 0) return p4l1;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p4l2))->Name) == 0) return p4l2;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p4l3))->Name) == 0) return p4l3;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect4))->Name) == 0) return p4rect4;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p4l4))->Name) == 0) return p4l4;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p4l5))->Name) == 0) return p4l5;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p4l6))->Name) == 0) return p4l6;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p4b1))->Name) == 0) return p4b1;
	if (strcmp(name, ((HMI_ToggleButton_Data*)lv_obj_get_user_data(p4tb1))->Name) == 0) return p4tb1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p4b5))->Name) == 0) return p4b5;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p4b6))->Name) == 0) return p4b6;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p4b7))->Name) == 0) return p4b7;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p4b9))->Name) == 0) return p4b9;
	if (strcmp(name, ((HMI_TextList_Data*)lv_obj_get_user_data(p4tl1->getObj()))->Name) == 0) return p4tl1->getObj();
	if (strcmp(name, ((HMI_SlidePage_Data*)lv_obj_get_user_data(p6))->Name) == 0) return p6;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect1))->Name) == 0) return p6rect1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p6b1))->Name) == 0) return p6b1;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p6b2))->Name) == 0) return p6b2;
	if (strcmp(name, ((HMI_Button_Data*)lv_obj_get_user_data(p6b4))->Name) == 0) return p6b4;
	if (strcmp(name, ((HMI_Oscilloscope_Data*)lv_obj_get_user_data(p6osc1->getObj()))->Name) == 0) return p6osc1->getObj();
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect3))->Name) == 0) return p6rect3;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l1))->Name) == 0) return p6l1;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p6num1))->Name) == 0) return p6num1;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p6num2))->Name) == 0) return p6num2;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l2))->Name) == 0) return p6l2;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l3))->Name) == 0) return p6l3;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p6num3))->Name) == 0) return p6num3;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l4))->Name) == 0) return p6l4;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p6num4))->Name) == 0) return p6num4;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l5))->Name) == 0) return p6l5;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p6num5))->Name) == 0) return p6num5;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l6))->Name) == 0) return p6l6;
	if (strcmp(name, ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect4))->Name) == 0) return p6rect4;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l7))->Name) == 0) return p6l7;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p6num6))->Name) == 0) return p6num6;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p6num7))->Name) == 0) return p6num7;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l8))->Name) == 0) return p6l8;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l9))->Name) == 0) return p6l9;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p6num8))->Name) == 0) return p6num8;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l10))->Name) == 0) return p6l10;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p6num9))->Name) == 0) return p6num9;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l11))->Name) == 0) return p6l11;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p6num10))->Name) == 0) return p6num10;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l12))->Name) == 0) return p6l12;
	if (strcmp(name, ((HMI_Oscilloscope_Data*)lv_obj_get_user_data(p6osc2->getObj()))->Name) == 0) return p6osc2->getObj();
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l13))->Name) == 0) return p6l13;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p6num11))->Name) == 0) return p6num11;
	if (strcmp(name, ((HMI_Label_Data*)lv_obj_get_user_data(p6l14))->Name) == 0) return p6l14;
	if (strcmp(name, ((HMI_Number_Data*)lv_obj_get_user_data(p6num12))->Name) == 0) return p6num12;
	return NULL;
}

int MyHMI::ID_TO_SN(lv_obj_t* id)
{
	if (id == p1) return 1;
	if (id == p2) return 2;
	if (id == p4) return 4;
	if (id == p6) return 6;
	if (id == p1rect5) return 543001;
	if (id == p1rect1) return 143001;
	if (id == p1rect2) return 243001;
	if (id == p1rect3) return 343001;
	if (id == p1rect4) return 443001;
	if (id == p1b1) return 105001;
	if (id == p1b2) return 205001;
	if (id == p1b3) return 305001;
	if (id == p1b4) return 405001;
	if (id == p1b5) return 505001;
	if (id == p1b6) return 605001;
	if (id == p1abar1) return 102001;
	if (id == p1abar2) return 202001;
	if (id == p1abar3) return 302001;
	if (id == p1led1) return 121001;
	if (id == p1led2) return 221001;
	if (id == p1led3) return 321001;
	if (id == p1num1) return 120001;
	if (id == p1num2) return 220001;
	if (id == p1num3) return 320001;
	if (id == p1num4) return 420001;
	if (id == p1num5) return 520001;
	if (id == p1num6) return 620001;
	if (id == p1l1) return 119001;
	if (id == p1l2) return 219001;
	if (id == p1l3) return 319001;
	if (id == p1l4) return 419001;
	if (id == p1l5) return 519001;
	if (id == p1l6) return 619001;
	if (id == p1b7) return 705001;
	if (id == p1ddl1) return 114001;
	if (id == p1l7) return 719001;
	if (id == p1b8) return 805001;
	if (id == p1tb1) return 106001;
	if (id == p1tb2) return 206001;
	if (id == p1ddl2) return 214001;
	if (id == p1l8) return 819001;
	if (id == p1b12) return 1205001;
	if (id == p1b14) return 1405001;
	if (id == p2rect4) return 443002;
	if (id == p2b1) return 105002;
	if (id == p2fb1->getObj()) return 150002;
	if (id == p2b2) return 205002;
	if (id == p2b5) return 505002;
	if (id == p2l7) return 719002;
	if (id == p2b7) return 705002;
	if (id == p4rect1) return 143004;
	if (id == p4rect2) return 243004;
	if (id == p4rect3) return 343004;
	if (id == p4rect5) return 543004;
	if (id == p4abar1) return 102004;
	if (id == p4abar2) return 202004;
	if (id == p4abar3) return 302004;
	if (id == p4led1) return 121004;
	if (id == p4led2) return 221004;
	if (id == p4led3) return 321004;
	if (id == p4num1) return 120004;
	if (id == p4num2) return 220004;
	if (id == p4num3) return 320004;
	if (id == p4num4) return 420004;
	if (id == p4num5) return 520004;
	if (id == p4num6) return 620004;
	if (id == p4l1) return 119004;
	if (id == p4l2) return 219004;
	if (id == p4l3) return 319004;
	if (id == p4rect4) return 443004;
	if (id == p4l4) return 419004;
	if (id == p4l5) return 519004;
	if (id == p4l6) return 619004;
	if (id == p4b1) return 105004;
	if (id == p4tb1) return 106004;
	if (id == p4b5) return 505004;
	if (id == p4b6) return 605004;
	if (id == p4b7) return 705004;
	if (id == p4b9) return 905004;
	if (id == p4tl1->getObj()) return 151004;
	if (id == p6rect1) return 143006;
	if (id == p6b1) return 105006;
	if (id == p6b2) return 205006;
	if (id == p6b4) return 405006;
	if (id == p6osc1->getObj()) return 142006;
	if (id == p6rect3) return 343006;
	if (id == p6l1) return 119006;
	if (id == p6num1) return 120006;
	if (id == p6num2) return 220006;
	if (id == p6l2) return 219006;
	if (id == p6l3) return 319006;
	if (id == p6num3) return 320006;
	if (id == p6l4) return 419006;
	if (id == p6num4) return 420006;
	if (id == p6l5) return 519006;
	if (id == p6num5) return 520006;
	if (id == p6l6) return 619006;
	if (id == p6rect4) return 443006;
	if (id == p6l7) return 719006;
	if (id == p6num6) return 620006;
	if (id == p6num7) return 720006;
	if (id == p6l8) return 819006;
	if (id == p6l9) return 919006;
	if (id == p6num8) return 820006;
	if (id == p6l10) return 1019006;
	if (id == p6num9) return 920006;
	if (id == p6l11) return 1119006;
	if (id == p6num10) return 1020006;
	if (id == p6l12) return 1219006;
	if (id == p6osc2->getObj()) return 242006;
	if (id == p6l13) return 1319006;
	if (id == p6num11) return 1120006;
	if (id == p6l14) return 1419006;
	if (id == p6num12) return 1220006;
	return 0;
}

char* MyHMI::ID_TO_NAME(lv_obj_t* id)
{
	if (id == p1) return ((HMI_SlidePage_Data*)lv_obj_get_user_data(p1))->Name;
	if (id == p1rect5) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect5))->Name;
	if (id == p1rect1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect1))->Name;
	if (id == p1rect2) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect2))->Name;
	if (id == p1rect3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect3))->Name;
	if (id == p1rect4) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1rect4))->Name;
	if (id == p1b1) return ((HMI_Button_Data*)lv_obj_get_user_data(p1b1))->Name;
	if (id == p1b2) return ((HMI_Button_Data*)lv_obj_get_user_data(p1b2))->Name;
	if (id == p1b3) return ((HMI_Button_Data*)lv_obj_get_user_data(p1b3))->Name;
	if (id == p1b4) return ((HMI_Button_Data*)lv_obj_get_user_data(p1b4))->Name;
	if (id == p1b5) return ((HMI_Button_Data*)lv_obj_get_user_data(p1b5))->Name;
	if (id == p1b6) return ((HMI_Button_Data*)lv_obj_get_user_data(p1b6))->Name;
	if (id == p1abar1) return ((HMI_Arc_Data*)lv_obj_get_user_data(p1abar1))->Name;
	if (id == p1abar2) return ((HMI_Arc_Data*)lv_obj_get_user_data(p1abar2))->Name;
	if (id == p1abar3) return ((HMI_Arc_Data*)lv_obj_get_user_data(p1abar3))->Name;
	if (id == p1led1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1led1))->Name;
	if (id == p1led2) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1led2))->Name;
	if (id == p1led3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p1led3))->Name;
	if (id == p1num1) return ((HMI_Number_Data*)lv_obj_get_user_data(p1num1))->Name;
	if (id == p1num2) return ((HMI_Number_Data*)lv_obj_get_user_data(p1num2))->Name;
	if (id == p1num3) return ((HMI_Number_Data*)lv_obj_get_user_data(p1num3))->Name;
	if (id == p1num4) return ((HMI_Number_Data*)lv_obj_get_user_data(p1num4))->Name;
	if (id == p1num5) return ((HMI_Number_Data*)lv_obj_get_user_data(p1num5))->Name;
	if (id == p1num6) return ((HMI_Number_Data*)lv_obj_get_user_data(p1num6))->Name;
	if (id == p1l1) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l1))->Name;
	if (id == p1l2) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l2))->Name;
	if (id == p1l3) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l3))->Name;
	if (id == p1l4) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l4))->Name;
	if (id == p1l5) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l5))->Name;
	if (id == p1l6) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l6))->Name;
	if (id == p1b7) return ((HMI_Button_Data*)lv_obj_get_user_data(p1b7))->Name;
	if (id == p1ddl1) return ((HMI_DropDownList_Data*)lv_obj_get_user_data(p1ddl1))->Name;
	if (id == p1l7) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l7))->Name;
	if (id == p1b8) return ((HMI_Button_Data*)lv_obj_get_user_data(p1b8))->Name;
	if (id == p1tb1) return ((HMI_ToggleButton_Data*)lv_obj_get_user_data(p1tb1))->Name;
	if (id == p1tb2) return ((HMI_ToggleButton_Data*)lv_obj_get_user_data(p1tb2))->Name;
	if (id == p1ddl2) return ((HMI_DropDownList_Data*)lv_obj_get_user_data(p1ddl2))->Name;
	if (id == p1l8) return ((HMI_Label_Data*)lv_obj_get_user_data(p1l8))->Name;
	if (id == p1b12) return ((HMI_Button_Data*)lv_obj_get_user_data(p1b12))->Name;
	if (id == p1b14) return ((HMI_Button_Data*)lv_obj_get_user_data(p1b14))->Name;
	if (id == p2) return ((HMI_SlidePage_Data*)lv_obj_get_user_data(p2))->Name;
	if (id == p2rect4) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p2rect4))->Name;
	if (id == p2b1) return ((HMI_Button_Data*)lv_obj_get_user_data(p2b1))->Name;
	if (id == p2fb1->getObj()) return ((HMI_FileBrowser_Data*)get_list_user_data(p2fb1->getObj()))->Name;
	if (id == p2b2) return ((HMI_Button_Data*)lv_obj_get_user_data(p2b2))->Name;
	if (id == p2b5) return ((HMI_Button_Data*)lv_obj_get_user_data(p2b5))->Name;
	if (id == p2l7) return ((HMI_Label_Data*)lv_obj_get_user_data(p2l7))->Name;
	if (id == p2b7) return ((HMI_Button_Data*)lv_obj_get_user_data(p2b7))->Name;
	if (id == p4) return ((HMI_SlidePage_Data*)lv_obj_get_user_data(p4))->Name;
	if (id == p4rect1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect1))->Name;
	if (id == p4rect2) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect2))->Name;
	if (id == p4rect3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect3))->Name;
	if (id == p4rect5) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect5))->Name;
	if (id == p4abar1) return ((HMI_Arc_Data*)lv_obj_get_user_data(p4abar1))->Name;
	if (id == p4abar2) return ((HMI_Arc_Data*)lv_obj_get_user_data(p4abar2))->Name;
	if (id == p4abar3) return ((HMI_Arc_Data*)lv_obj_get_user_data(p4abar3))->Name;
	if (id == p4led1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4led1))->Name;
	if (id == p4led2) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4led2))->Name;
	if (id == p4led3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4led3))->Name;
	if (id == p4num1) return ((HMI_Number_Data*)lv_obj_get_user_data(p4num1))->Name;
	if (id == p4num2) return ((HMI_Number_Data*)lv_obj_get_user_data(p4num2))->Name;
	if (id == p4num3) return ((HMI_Number_Data*)lv_obj_get_user_data(p4num3))->Name;
	if (id == p4num4) return ((HMI_Number_Data*)lv_obj_get_user_data(p4num4))->Name;
	if (id == p4num5) return ((HMI_Number_Data*)lv_obj_get_user_data(p4num5))->Name;
	if (id == p4num6) return ((HMI_Number_Data*)lv_obj_get_user_data(p4num6))->Name;
	if (id == p4l1) return ((HMI_Label_Data*)lv_obj_get_user_data(p4l1))->Name;
	if (id == p4l2) return ((HMI_Label_Data*)lv_obj_get_user_data(p4l2))->Name;
	if (id == p4l3) return ((HMI_Label_Data*)lv_obj_get_user_data(p4l3))->Name;
	if (id == p4rect4) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p4rect4))->Name;
	if (id == p4l4) return ((HMI_Label_Data*)lv_obj_get_user_data(p4l4))->Name;
	if (id == p4l5) return ((HMI_Label_Data*)lv_obj_get_user_data(p4l5))->Name;
	if (id == p4l6) return ((HMI_Label_Data*)lv_obj_get_user_data(p4l6))->Name;
	if (id == p4b1) return ((HMI_Button_Data*)lv_obj_get_user_data(p4b1))->Name;
	if (id == p4tb1) return ((HMI_ToggleButton_Data*)lv_obj_get_user_data(p4tb1))->Name;
	if (id == p4b5) return ((HMI_Button_Data*)lv_obj_get_user_data(p4b5))->Name;
	if (id == p4b6) return ((HMI_Button_Data*)lv_obj_get_user_data(p4b6))->Name;
	if (id == p4b7) return ((HMI_Button_Data*)lv_obj_get_user_data(p4b7))->Name;
	if (id == p4b9) return ((HMI_Button_Data*)lv_obj_get_user_data(p4b9))->Name;
	if (id == p4tl1->getObj()) return ((HMI_TextList_Data*)lv_obj_get_user_data(p4tl1->getObj()))->Name;
	if (id == p6) return ((HMI_SlidePage_Data*)lv_obj_get_user_data(p6))->Name;
	if (id == p6rect1) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect1))->Name;
	if (id == p6b1) return ((HMI_Button_Data*)lv_obj_get_user_data(p6b1))->Name;
	if (id == p6b2) return ((HMI_Button_Data*)lv_obj_get_user_data(p6b2))->Name;
	if (id == p6b4) return ((HMI_Button_Data*)lv_obj_get_user_data(p6b4))->Name;
	if (id == p6osc1->getObj()) return ((HMI_Oscilloscope_Data*)lv_obj_get_user_data(p6osc1->getObj()))->Name;
	if (id == p6rect3) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect3))->Name;
	if (id == p6l1) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l1))->Name;
	if (id == p6num1) return ((HMI_Number_Data*)lv_obj_get_user_data(p6num1))->Name;
	if (id == p6num2) return ((HMI_Number_Data*)lv_obj_get_user_data(p6num2))->Name;
	if (id == p6l2) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l2))->Name;
	if (id == p6l3) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l3))->Name;
	if (id == p6num3) return ((HMI_Number_Data*)lv_obj_get_user_data(p6num3))->Name;
	if (id == p6l4) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l4))->Name;
	if (id == p6num4) return ((HMI_Number_Data*)lv_obj_get_user_data(p6num4))->Name;
	if (id == p6l5) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l5))->Name;
	if (id == p6num5) return ((HMI_Number_Data*)lv_obj_get_user_data(p6num5))->Name;
	if (id == p6l6) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l6))->Name;
	if (id == p6rect4) return ((HMI_BasicObj_Data*)lv_obj_get_user_data(p6rect4))->Name;
	if (id == p6l7) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l7))->Name;
	if (id == p6num6) return ((HMI_Number_Data*)lv_obj_get_user_data(p6num6))->Name;
	if (id == p6num7) return ((HMI_Number_Data*)lv_obj_get_user_data(p6num7))->Name;
	if (id == p6l8) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l8))->Name;
	if (id == p6l9) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l9))->Name;
	if (id == p6num8) return ((HMI_Number_Data*)lv_obj_get_user_data(p6num8))->Name;
	if (id == p6l10) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l10))->Name;
	if (id == p6num9) return ((HMI_Number_Data*)lv_obj_get_user_data(p6num9))->Name;
	if (id == p6l11) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l11))->Name;
	if (id == p6num10) return ((HMI_Number_Data*)lv_obj_get_user_data(p6num10))->Name;
	if (id == p6l12) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l12))->Name;
	if (id == p6osc2->getObj()) return ((HMI_Oscilloscope_Data*)lv_obj_get_user_data(p6osc2->getObj()))->Name;
	if (id == p6l13) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l13))->Name;
	if (id == p6num11) return ((HMI_Number_Data*)lv_obj_get_user_data(p6num11))->Name;
	if (id == p6l14) return ((HMI_Label_Data*)lv_obj_get_user_data(p6l14))->Name;
	if (id == p6num12) return ((HMI_Number_Data*)lv_obj_get_user_data(p6num12))->Name;
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
	ButtonRefreshText(p1b1);
	ButtonRefreshText(p1b2);
	ButtonRefreshText(p1b3);
	ButtonRefreshText(p1b4);
	ButtonRefreshText(p1b5);
	ButtonRefreshText(p1b6);
	ArcBarRefreshText(p1abar1);
	ArcBarRefreshText(p1abar2);
	ArcBarRefreshText(p1abar3);
	NumberRefreshText(p1num1);
	NumberRefreshText(p1num2);
	NumberRefreshText(p1num3);
	NumberRefreshText(p1num4);
	NumberRefreshText(p1num5);
	NumberRefreshText(p1num6);
	LabelRefreshText(p1l1);
	LabelRefreshText(p1l2);
	LabelRefreshText(p1l3);
	LabelRefreshText(p1l4);
	LabelRefreshText(p1l5);
	LabelRefreshText(p1l6);
	ButtonRefreshText(p1b7);
	DropDownListRefreshText(p1ddl1);
	LabelRefreshText(p1l7);
	ButtonRefreshText(p1b8);
	ToggleButtonRefreshText(p1tb1);
	ToggleButtonRefreshText(p1tb2);
	DropDownListRefreshText(p1ddl2);
	LabelRefreshText(p1l8);
	ButtonRefreshText(p1b12);
	ButtonRefreshText(p1b14);
	ButtonRefreshText(p2b1);
	ButtonRefreshText(p2b2);
	ButtonRefreshText(p2b5);
	LabelRefreshText(p2l7);
	ButtonRefreshText(p2b7);
	ArcBarRefreshText(p4abar1);
	ArcBarRefreshText(p4abar2);
	ArcBarRefreshText(p4abar3);
	NumberRefreshText(p4num1);
	NumberRefreshText(p4num2);
	NumberRefreshText(p4num3);
	NumberRefreshText(p4num4);
	NumberRefreshText(p4num5);
	NumberRefreshText(p4num6);
	LabelRefreshText(p4l1);
	LabelRefreshText(p4l2);
	LabelRefreshText(p4l3);
	LabelRefreshText(p4l4);
	LabelRefreshText(p4l5);
	LabelRefreshText(p4l6);
	ButtonRefreshText(p4b1);
	ToggleButtonRefreshText(p4tb1);
	ButtonRefreshText(p4b5);
	ButtonRefreshText(p4b6);
	ButtonRefreshText(p4b7);
	ButtonRefreshText(p4b9);
	ButtonRefreshText(p6b1);
	ButtonRefreshText(p6b2);
	ButtonRefreshText(p6b4);
	LabelRefreshText(p6l1);
	NumberRefreshText(p6num1);
	NumberRefreshText(p6num2);
	LabelRefreshText(p6l2);
	LabelRefreshText(p6l3);
	NumberRefreshText(p6num3);
	LabelRefreshText(p6l4);
	NumberRefreshText(p6num4);
	LabelRefreshText(p6l5);
	NumberRefreshText(p6num5);
	LabelRefreshText(p6l6);
	LabelRefreshText(p6l7);
	NumberRefreshText(p6num6);
	NumberRefreshText(p6num7);
	LabelRefreshText(p6l8);
	LabelRefreshText(p6l9);
	NumberRefreshText(p6num8);
	LabelRefreshText(p6l10);
	NumberRefreshText(p6num9);
	LabelRefreshText(p6l11);
	NumberRefreshText(p6num10);
	LabelRefreshText(p6l12);
	LabelRefreshText(p6l13);
	NumberRefreshText(p6num11);
	LabelRefreshText(p6l14);
	NumberRefreshText(p6num12);
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return;
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
	return 0;
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
	return 0;
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
	return false;
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
	return 0;
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
	return 0;
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
	return 0;
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
	return 0;
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
	return 0;
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
	return 0;
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
	return 0;
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
	return 0;
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
	return 0;
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
	return 0;
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
	return NULL;
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
	return false;
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
	return false;
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
	return false;
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
	return false;
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
	return false;
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
	return false;
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
	return 0;
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
	return 0;
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
	return 0;
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
	lv_freetype_font_init(themeFonts[0], envPathCombine("IRSPATH", "Font/microhei.ttc"), 22);
	themeFonts[1] = (lv_font_t*)malloc(sizeof(lv_font_t));
	lv_freetype_font_init(themeFonts[1], envPathCombine("IRSPATH", "Font/microhei.ttc"), 34);
	themeFonts[2] = (lv_font_t*)malloc(sizeof(lv_font_t));
	lv_freetype_font_init(themeFonts[2], envPathCombine("IRSPATH", "Font/microhei.ttc"), 18);
	themeFonts[3] = (lv_font_t*)malloc(sizeof(lv_font_t));
	lv_freetype_font_init(themeFonts[3], envPathCombine("IRSPATH", "Font/microhei.ttc"), 18);
	themeFonts[4] = (lv_font_t*)malloc(sizeof(lv_font_t));
	lv_freetype_font_init(themeFonts[4], envPathCombine("IRSPATH", "Font/microhei.ttc"), 28);
	themeFonts[5] = (lv_font_t*)malloc(sizeof(lv_font_t));
	lv_freetype_font_init(themeFonts[5], envPathCombine("IRSPATH", "Font/microhei.ttc"), 16);
	lv_theme_material86_init(themePalette, themeFonts);
	static HMI_SlidePage_Data p1_Data = {"My Page p1", 1, 100, true};
	static HMI_BasicObj_Data p1rect5_Data = {"My Rectangle p1rect5"};
	static HMI_BasicObj_Data p1rect1_Data = {"My Rectangle p1rect1"};
	static HMI_BasicObj_Data p1rect2_Data = {"My Rectangle p1rect2"};
	static HMI_BasicObj_Data p1rect3_Data = {"My Rectangle p1rect3"};
	static HMI_BasicObj_Data p1rect4_Data = {"My Rectangle p1rect4"};
	static HMI_Button_Data p1b1_Data = {"My Button p1b1", "", {"X+", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p1b2_Data = {"My Button p1b2", "", {"X-", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p1b3_Data = {"My Button p1b3", "", {"Y+", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p1b4_Data = {"My Button p1b4", "", {"Y-", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p1b5_Data = {"My Button p1b5", "", {"Z+", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p1b6_Data = {"My Button p1b6", "", {"Z-", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Arc_Data p1abar1_Data = {"My ArcBar p1abar1", 0, 260, 0, 0, 0, {"", "", "", "", "", "", "", "", ""}, 2};
	static HMI_Arc_Data p1abar2_Data = {"My ArcBar p1abar2", 0, 160, 0, 0, 0, {"", "", "", "", "", "", "", "", ""}, 2};
	static HMI_Arc_Data p1abar3_Data = {"My ArcBar p1abar3", 0, 0, -30, 0, 0, {"", "", "", "", "", "", "", "", ""}, 2};
	static HMI_BasicObj_Data p1led1_Data = {"My LED p1led1"};
	static HMI_BasicObj_Data p1led2_Data = {"My LED p1led2"};
	static HMI_BasicObj_Data p1led3_Data = {"My LED p1led3"};
	static HMI_Number_Data p1num1_Data = {"My Number p1num1", {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}, 1, 0};
	static HMI_Number_Data p1num2_Data = {"My Number p1num2", {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}, 1, 0};
	static HMI_Number_Data p1num3_Data = {"My Number p1num3", {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}, 1, 0};
	static HMI_Number_Data p1num4_Data = {"My Number p1num4", {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}, 0, 0};
	static HMI_Number_Data p1num5_Data = {"My Number p1num5", {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}, 0, 0};
	static HMI_Number_Data p1num6_Data = {"My Number p1num6", {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}, 0, 0};
	static HMI_Label_Data p1l1_Data = {"My Label p1l1", {"X", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p1l2_Data = {"My Label p1l2", {"Y", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p1l3_Data = {"My Label p1l3", {"Z", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p1l4_Data = {"My Label p1l4", {"ENC", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p1l5_Data = {"My Label p1l5", {"ENC", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p1l6_Data = {"My Label p1l6", {"ENC", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Button_Data p1b7_Data = {"My Button p1b7", "", {"G-Code", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_DropDownList_Data p1ddl1_Data = {"My DropDownList p1ddl1", {"x1\nx10\nx100", "Item A2\nItem B2\n", "Item A3\nItem B3\n", "Item A4\nItem B4\n", "Item A5\nItem B5\n", "Item A6\nItem B6\n", "Item A7\nItem B7\n", "Item A8\nItem B8\n", "Item A9\nItem B9\n"}};
	static HMI_Label_Data p1l7_Data = {"My Label p1l7", {"IncJog: ", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Button_Data p1b8_Data = {"My Button p1b8", LV_SYMBOL_HOME, {"Home", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_ToggleButton_Data p1tb1_Data = {"My ToggleButton p1tb1", LV_SYMBOL_WARNING, {"Clear", "OFF2", "OFF3", "OFF4", "OFF5", "OFF6", "OFF7", "OFF8", "OFF9"}, {"EMG", "ON2", "ON3", "ON4", "ON5", "ON6", "ON7", "ON8", "ON9"}};
	static HMI_ToggleButton_Data p1tb2_Data = {"My ToggleButton p1tb2", LV_SYMBOL_CHARGE, {"OFF", "OFF2", "OFF3", "OFF4", "OFF5", "OFF6", "OFF7", "OFF8", "OFF9"}, {"ON", "ON2", "ON3", "ON4", "ON5", "ON6", "ON7", "ON8", "ON9"}};
	static HMI_DropDownList_Data p1ddl2_Data = {"My DropDownList p1ddl2", {"400\n200\n100", "Item A2\nItem B2\n", "Item A3\nItem B3\n", "Item A4\nItem B4\n", "Item A5\nItem B5\n", "Item A6\nItem B6\n", "Item A7\nItem B7\n", "Item A8\nItem B8\n", "Item A9\nItem B9\n"}};
	static HMI_Label_Data p1l8_Data = {"My Label p1l8", {"Feedrate: ", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Button_Data p1b12_Data = {"My Button p1b12", "", {"Jog", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p1b14_Data = {"My Button p1b14", "", {"Status", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_SlidePage_Data p2_Data = {"My Page p2", 0, 100, false};
	static HMI_BasicObj_Data p2rect4_Data = {"My Rectangle p2rect4"};
	static HMI_Button_Data p2b1_Data = {"My Button p2b1", "", {"Jog", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_FileBrowser_Data p2fb1_Data = {"My FileBrowser p2fb1"};
	static HMI_Button_Data p2b2_Data = {"My Button p2b2", LV_SYMBOL_DRIVE, {"Load", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p2b5_Data = {"My Button p2b5", "", {"G-Code", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Label_Data p2l7_Data = {"My Label p2l7", {"Choose your G-Code file:", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Button_Data p2b7_Data = {"My Button p2b7", "", {"Status", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_SlidePage_Data p4_Data = {"My Page p4", 1, 100, true};
	static HMI_BasicObj_Data p4rect1_Data = {"My Rectangle p3rect1"};
	static HMI_BasicObj_Data p4rect2_Data = {"My Rectangle p3rect2"};
	static HMI_BasicObj_Data p4rect3_Data = {"My Rectangle p3rect3"};
	static HMI_BasicObj_Data p4rect5_Data = {"My Rectangle p3rect5"};
	static HMI_Arc_Data p4abar1_Data = {"My ArcBar p3abar1", 0, 260, 0, 0, 0, {"", "", "", "", "", "", "", "", ""}, 2};
	static HMI_Arc_Data p4abar2_Data = {"My ArcBar p3abar2", 0, 160, 0, 0, 0, {"", "", "", "", "", "", "", "", ""}, 2};
	static HMI_Arc_Data p4abar3_Data = {"My ArcBar p3abar3", 0, 0, -30, 0, 0, {"", "", "", "", "", "", "", "", ""}, 2};
	static HMI_BasicObj_Data p4led1_Data = {"My LED p3led1"};
	static HMI_BasicObj_Data p4led2_Data = {"My LED p3led2"};
	static HMI_BasicObj_Data p4led3_Data = {"My LED p3led3"};
	static HMI_Number_Data p4num1_Data = {"My Number p3num1", {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}, 1, 0};
	static HMI_Number_Data p4num2_Data = {"My Number p3num2", {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}, 1, 0};
	static HMI_Number_Data p4num3_Data = {"My Number p3num3", {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}, 1, 0};
	static HMI_Number_Data p4num4_Data = {"My Number p3num4", {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}, 0, 0};
	static HMI_Number_Data p4num5_Data = {"My Number p3num5", {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}, 0, 0};
	static HMI_Number_Data p4num6_Data = {"My Number p3num6", {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}, 0, 0};
	static HMI_Label_Data p4l1_Data = {"My Label p3l1", {"X", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p4l2_Data = {"My Label p3l2", {"Y", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p4l3_Data = {"My Label p3l3", {"Z", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_BasicObj_Data p4rect4_Data = {"My Rectangle p3rect4"};
	static HMI_Label_Data p4l4_Data = {"My Label p3l4", {"ENC", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p4l5_Data = {"My Label p3l5", {"ENC", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p4l6_Data = {"My Label p3l6", {"ENC", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Button_Data p4b1_Data = {"My Button p3b1", LV_SYMBOL_CLOSE, {"Close", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_ToggleButton_Data p4tb1_Data = {"My ToggleButton p3tb1", LV_SYMBOL_WARNING, {"Clear", "OFF2", "OFF3", "OFF4", "OFF5", "OFF6", "OFF7", "OFF8", "OFF9"}, {"EMG", "ON2", "ON3", "ON4", "ON5", "ON6", "ON7", "ON8", "ON9"}};
	static HMI_Button_Data p4b5_Data = {"My Button p3b5", LV_SYMBOL_PLAY, {"Play", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p4b6_Data = {"My Button p3b6", "", {"G-Code", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p4b7_Data = {"My Button p3b7", "", {"Jog", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p4b9_Data = {"My Button p3b9", "", {"Status", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_TextList_Data p4tl1_Data = {"My TextList p3tl1"};
	static HMI_SlidePage_Data p6_Data = {"My Page p6", 1, 100, true};
	static HMI_BasicObj_Data p6rect1_Data = {"My Rectangle p5rect1"};
	static HMI_Button_Data p6b1_Data = {"My Button p5b1", "", {"Jog", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p6b2_Data = {"My Button p5b2", "", {"G-Code", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Button_Data p6b4_Data = {"My Button p5b4", "", {"Status", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}};
	static HMI_Oscilloscope_Data p6osc1_Data = {"My Oscilloscope p5osc1", 0, 10000, 100000, 2500000, 0xFFC866, 0, 0, 10, -10};
	static HMI_BasicObj_Data p6rect3_Data = {"My Rectangle p5rect3"};
	static HMI_Label_Data p6l1_Data = {"My Label p5l1", {"Temperature: ", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Number_Data p6num1_Data = {"My Number p5num1", {"", "", "", "", "", "", "", "", ""}, {" °C", "", "", "", "", "", "", "", ""}, 2, 0};
	static HMI_Number_Data p6num2_Data = {"My Number p5num2", {"", "", "", "", "", "", "", "", ""}, {" V", "", "", "", "", "", "", "", ""}, 2, 0};
	static HMI_Label_Data p6l2_Data = {"My Label p5l2", {"Vs: ", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p6l3_Data = {"My Label p5l3", {"Vp: ", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Number_Data p6num3_Data = {"My Number p5num3", {"", "", "", "", "", "", "", "", ""}, {" V", "", "", "", "", "", "", "", ""}, 2, 0};
	static HMI_Label_Data p6l4_Data = {"My Label p5l4", {"Is: ", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Number_Data p6num4_Data = {"My Number p5num4", {"", "", "", "", "", "", "", "", ""}, {" A", "", "", "", "", "", "", "", ""}, 2, 0};
	static HMI_Label_Data p6l5_Data = {"My Label p5l5", {"Ip: ", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Number_Data p6num5_Data = {"My Number p5num5", {"", "", "", "", "", "", "", "", ""}, {" A", "", "", "", "", "", "", "", ""}, 2, 0};
	static HMI_Label_Data p6l6_Data = {"My Label p5l6", {"QEC-M-070T", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_BasicObj_Data p6rect4_Data = {"My Rectangle p5rect4"};
	static HMI_Label_Data p6l7_Data = {"My Label p5l7", {"Temperature: ", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Number_Data p6num6_Data = {"My Number p5num6", {"", "", "", "", "", "", "", "", ""}, {" °C", "", "", "", "", "", "", "", ""}, 2, 0};
	static HMI_Number_Data p6num7_Data = {"My Number p5num7", {"", "", "", "", "", "", "", "", ""}, {" V", "", "", "", "", "", "", "", ""}, 2, 0};
	static HMI_Label_Data p6l8_Data = {"My Label p5l8", {"Vs: ", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Label_Data p6l9_Data = {"My Label p5l9", {"Vp: ", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Number_Data p6num8_Data = {"My Number p5num8", {"", "", "", "", "", "", "", "", ""}, {" V", "", "", "", "", "", "", "", ""}, 2, 0};
	static HMI_Label_Data p6l10_Data = {"My Label p5l10", {"Is: ", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Number_Data p6num9_Data = {"My Number p5num9", {"", "", "", "", "", "", "", "", ""}, {" A", "", "", "", "", "", "", "", ""}, 2, 0};
	static HMI_Label_Data p6l11_Data = {"My Label p5l11", {"Ip: ", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Number_Data p6num10_Data = {"My Number p5num10", {"", "", "", "", "", "", "", "", ""}, {" A", "", "", "", "", "", "", "", ""}, 2, 0};
	static HMI_Label_Data p6l12_Data = {"My Label p5l12", {"QEC-R11MP3S-N", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Oscilloscope_Data p6osc2_Data = {"My Oscilloscope p5osc2", 0, 10000, 100000, 2500000, 0xFFC866, 0, 0, 10, -10};
	static HMI_Label_Data p6l13_Data = {"My Label p5l13", {"CO2 emissions: ", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Number_Data p6num11_Data = {"My Number p5num11", {"", "", "", "", "", "", "", "", ""}, {" g/hr", "", "", "", "", "", "", "", ""}, 2, 0};
	static HMI_Label_Data p6l14_Data = {"My Label p5l14", {"CO2 emissions: ", "Text2", "Text3", "Text4", "Text5", "Text6", "Text7", "Text8", "Text9"}, {"", "", "", "", "", "", "", "", ""}, {"", "", "", "", "", "", "", "", ""}};
	static HMI_Number_Data p6num12_Data = {"My Number p5num12", {"", "", "", "", "", "", "", "", ""}, {" g/hr", "", "", "", "", "", "", "", ""}, 2, 0};
	tileview = lv_tileview_create(lv_scr_act(), NULL);
	lv_page_set_scrlbar_mode(tileview, LV_SCRLBAR_MODE_OFF);
	lv_obj_set_style_local_bg_opa(tileview, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
	lv_obj_set_style_local_border_width(tileview, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 0);
	p1 = SlidePageInit(tileview, NULL, &p1_Data);
	lv_obj_set_pos(p1, LV_HOR_RES * 0, 0);
	p1rect5 = RectangleInit(p1, -10, -10, 820, 70, -2, false, 39, 0x5B5E63, 100, &p1rect5_Data);
	p1rect1 = RectangleInit(p1, 72, 65, 190, 180, -2, false, 38, -1, 100, &p1rect1_Data);
	p1rect2 = RectangleInit(p1, 297, 65, 190, 180, -2, false, 37, -1, 100, &p1rect2_Data);
	p1rect3 = RectangleInit(p1, 522, 65, 190, 180, -2, false, 36, -1, 100, &p1rect3_Data);
	p1rect4 = RectangleInit(p1, 72, 250, 640, 225, -2, false, 35, -1, 100, &p1rect4_Data);
	p1b1 = ButtonInit(p1, 120, 330, 50, 50, 1, true, 34, 0, -1, -1, &p1b1_Data);
	p1b2 = ButtonInit(p1, 120, 400, 50, 50, 1, true, 33, 0, -1, -1, &p1b2_Data);
	p1b3 = ButtonInit(p1, 198, 330, 50, 50, 2, true, 32, 0, -1, -1, &p1b3_Data);
	p1b4 = ButtonInit(p1, 198, 400, 50, 50, 2, true, 31, 0, -1, -1, &p1b4_Data);
	p1b5 = ButtonInit(p1, 276, 330, 50, 50, 3, true, 30, 0, -1, -1, &p1b5_Data);
	p1b6 = ButtonInit(p1, 276, 400, 50, 50, 3, true, 29, 0, -1, -1, &p1b6_Data);
	p1abar1 = ArcBarInit(p1, 72, 53, 190, 190, 1, true, 28, 180, 0, -1, -1, -1, 2, &p1abar1_Data);
	p1abar2 = ArcBarInit(p1, 297, 53, 190, 190, 2, true, 27, 180, 0, -1, -1, -1, 2, &p1abar2_Data);
	p1abar3 = ArcBarInit(p1, 522, 53, 190, 190, 3, true, 26, 180, 0, -1, -1, -1, 2, &p1abar3_Data);
	p1led1 = LEDInit(p1, 116, 177, 20, 20, 1, true, 25, 0, &p1led1_Data);
	p1led2 = LEDInit(p1, 341, 177, 20, 20, 2, true, 24, 0, &p1led2_Data);
	p1led3 = LEDInit(p1, 566, 177, 20, 20, 3, true, 23, 0, &p1led3_Data);
	p1num1 = NumberInit(p1, 153, 174, 70, 26, -2, true, 22, 3, -1, -1, &p1num1_Data);
	p1num2 = NumberInit(p1, 378, 174, 70, 26, -2, true, 21, 3, -1, -1, &p1num2_Data);
	p1num3 = NumberInit(p1, 603, 174, 70, 26, -2, true, 20, 3, -1, -1, &p1num3_Data);
	p1num4 = NumberInit(p1, 153, 206, 70, 26, -2, true, 19, 3, -1, -1, &p1num4_Data);
	p1num5 = NumberInit(p1, 378, 206, 70, 26, -2, true, 18, 3, -1, -1, &p1num5_Data);
	p1num6 = NumberInit(p1, 603, 206, 70, 26, -2, true, 17, 3, -1, -1, &p1num6_Data);
	p1l1 = LabelInit(p1, 158, 117, 19, 40, -2, true, 16, 0, -1, 1, &p1l1_Data);
	p1l2 = LabelInit(p1, 383, 117, 18, 40, -2, true, 15, 0, -1, 1, &p1l2_Data);
	p1l3 = LabelInit(p1, 608, 117, 18, 40, -2, true, 14, 0, -1, 1, &p1l3_Data);
	p1l4 = LabelInit(p1, 109, 206, 41, 26, -2, true, 13, 0, -1, -1, &p1l4_Data);
	p1l5 = LabelInit(p1, 334, 206, 41, 26, -2, true, 12, 0, -1, -1, &p1l5_Data);
	p1l6 = LabelInit(p1, 559, 206, 41, 26, -2, true, 11, 0, -1, -1, &p1l6_Data);
	p1b7 = ButtonInit(p1, 150, 10, 130, 43, -1, true, 10, 0, -1, -1, &p1b7_Data);
	p1ddl1 = DropDownListInit(p1, 192, 267, 150, 42, -1, true, 9, 0, &p1ddl1_Data);
	p1l7 = LabelInit(p1, 115, 275, 73, 26, -2, true, 8, 0, -1, -1, &p1l7_Data);
	p1b8 = ButtonInit(p1, 380, 350, 120, 80, -1, true, 7, 0, -1, -1, &p1b8_Data);
	p1tb1 = ToggleButtonInit(p1, 530, 400, 150, 50, 1, true, 6, -1, -1, -1, true, &p1tb1_Data);
	p1tb2 = ToggleButtonInit(p1, 530, 330, 150, 50, 2, true, 5, -1, -1, -1, true, &p1tb2_Data);
	p1ddl2 = DropDownListInit(p1, 501, 267, 150, 42, -1, true, 4, 0, &p1ddl2_Data);
	p1l8 = LabelInit(p1, 392, 275, 100, 26, -2, true, 3, 0, -1, -1, &p1l8_Data);
	p1b12 = ButtonInit(p1, 10, 10, 130, 43, -1, true, 2, 1, -1, -1, &p1b12_Data);
	p1b14 = ButtonInit(p1, 290, 10, 130, 43, -1, true, 1, 0, -1, -1, &p1b14_Data);

	p2 = SlidePageInit(tileview, NULL, &p2_Data);
	lv_obj_set_pos(p2, LV_HOR_RES * 1, 0);
	p2rect4 = RectangleInit(p2, -10, -10, 820, 70, -2, false, 7, 0x5B5E63, 100, &p2rect4_Data);
	p2b1 = ButtonInit(p2, 10, 10, 130, 43, -1, true, 6, 0, -1, -1, &p2b1_Data);
	p2fb1 = FileBrowserInit(p2, 150, 115, 500, 295, 3, true, 5, 2, 2, &p2fb1_Data);
	p2b2 = ButtonInit(p2, 315, 418, 170, 50, 3, true, 4, 1, -1, 1, &p2b2_Data);
	p2b5 = ButtonInit(p2, 150, 10, 130, 43, -1, true, 3, 1, -1, -1, &p2b5_Data);
	p2l7 = LabelInit(p2, 210, 69, 381, 40, -2, true, 2, 0, -1, 1, &p2l7_Data);
	p2b7 = ButtonInit(p2, 290, 10, 130, 43, -1, true, 1, 0, -1, -1, &p2b7_Data);

	p4 = SlidePageInit(tileview, NULL, &p4_Data);
	lv_obj_set_pos(p4, LV_HOR_RES * 2, 0);
	p4rect1 = RectangleInit(p4, 72, 65, 190, 180, -2, false, 30, -1, 100, &p4rect1_Data);
	p4rect2 = RectangleInit(p4, 297, 65, 190, 180, -2, false, 29, -1, 100, &p4rect2_Data);
	p4rect3 = RectangleInit(p4, 522, 65, 190, 180, -2, false, 28, -1, 100, &p4rect3_Data);
	p4rect5 = RectangleInit(p4, 72, 250, 640, 225, -2, false, 27, -1, 100, &p4rect5_Data);
	p4abar1 = ArcBarInit(p4, 72, 53, 190, 190, 1, true, 26, 180, 0, -1, -1, -1, 2, &p4abar1_Data);
	p4abar2 = ArcBarInit(p4, 297, 53, 190, 190, 2, true, 25, 180, 0, -1, -1, -1, 2, &p4abar2_Data);
	p4abar3 = ArcBarInit(p4, 522, 53, 190, 190, 3, true, 24, 180, 0, -1, -1, -1, 2, &p4abar3_Data);
	p4led1 = LEDInit(p4, 116, 177, 20, 20, 1, true, 23, 0, &p4led1_Data);
	p4led2 = LEDInit(p4, 341, 177, 20, 20, 2, true, 22, 0, &p4led2_Data);
	p4led3 = LEDInit(p4, 566, 177, 20, 20, 3, true, 21, 0, &p4led3_Data);
	p4num1 = NumberInit(p4, 153, 174, 70, 26, -2, true, 20, 3, -1, -1, &p4num1_Data);
	p4num2 = NumberInit(p4, 378, 174, 70, 26, -2, true, 19, 3, -1, -1, &p4num2_Data);
	p4num3 = NumberInit(p4, 603, 174, 70, 26, -2, true, 18, 3, -1, -1, &p4num3_Data);
	p4num4 = NumberInit(p4, 153, 206, 70, 26, -2, true, 17, 3, -1, -1, &p4num4_Data);
	p4num5 = NumberInit(p4, 378, 206, 70, 26, -2, true, 16, 3, -1, -1, &p4num5_Data);
	p4num6 = NumberInit(p4, 603, 206, 70, 26, -2, true, 15, 3, -1, -1, &p4num6_Data);
	p4l1 = LabelInit(p4, 158, 117, 19, 40, -2, true, 14, 0, -1, 1, &p4l1_Data);
	p4l2 = LabelInit(p4, 383, 117, 18, 40, -2, true, 13, 0, -1, 1, &p4l2_Data);
	p4l3 = LabelInit(p4, 608, 117, 18, 40, -2, true, 12, 0, -1, 1, &p4l3_Data);
	p4rect4 = RectangleInit(p4, -10, -10, 820, 70, -2, false, 11, 0x5B5E63, 100, &p4rect4_Data);
	p4l4 = LabelInit(p4, 109, 206, 41, 26, -2, true, 10, 0, -1, -1, &p4l4_Data);
	p4l5 = LabelInit(p4, 334, 206, 41, 26, -2, true, 9, 0, -1, -1, &p4l5_Data);
	p4l6 = LabelInit(p4, 559, 206, 41, 26, -2, true, 8, 0, -1, -1, &p4l6_Data);
	p4b1 = ButtonInit(p4, 660, 10, 130, 43, 1, true, 7, 1, -1, -1, &p4b1_Data);
	p4tb1 = ToggleButtonInit(p4, 530, 375, 150, 80, 1, true, 6, -1, -1, -1, true, &p4tb1_Data);
	p4b5 = ButtonInit(p4, 530, 271, 150, 80, 3, true, 5, 1, -1, -1, &p4b5_Data);
	p4b6 = ButtonInit(p4, 150, 10, 130, 43, -1, true, 4, 1, -1, -1, &p4b6_Data);
	p4b7 = ButtonInit(p4, 10, 10, 130, 43, -1, false, 3, 0, -1, -1, &p4b7_Data);
	p4b9 = ButtonInit(p4, 290, 10, 130, 43, -1, false, 2, 0, -1, -1, &p4b9_Data);
	p4tl1 = TextListInit(p4, 92, 270, 405, 185, -2, true, 1, 0, 1, 1, 1, 1, 0xFFFF8B, -1, 0, -1, 0x686D77, -1, 0, &p4tl1_Data);

	p6 = SlidePageInit(tileview, NULL, &p6_Data);
	lv_obj_set_pos(p6, LV_HOR_RES * 3, 0);
	p6rect1 = RectangleInit(p6, -10, -10, 820, 70, -2, false, 34, 0x5B5E63, 100, &p6rect1_Data);
	p6b1 = ButtonInit(p6, 10, 10, 130, 43, -1, true, 33, 0, -1, -1, &p6b1_Data);
	p6b2 = ButtonInit(p6, 150, 10, 130, 43, -1, true, 32, 0, -1, -1, &p6b2_Data);
	p6b4 = ButtonInit(p6, 290, 10, 130, 43, -1, true, 31, 1, -1, -1, &p6b4_Data);
	p6osc1 = OscilloscopeInit(p6, 93, 293, 295, 180, -2, true, 30, 0, 1, 1, 5, 3, 3, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, &p6osc1_Data);
	p6rect3 = RectangleInit(p6, 93, 65, 295, 220, -2, false, 29, -1, 100, &p6rect3_Data);
	p6l1 = LabelInit(p6, 107, 203, 142, 26, -2, true, 28, 0, -1, -1, &p6l1_Data);
	p6num1 = NumberInit(p6, 275, 203, 100, 26, -2, true, 27, 3, 0x83FFDC, -1, &p6num1_Data);
	p6num2 = NumberInit(p6, 150, 126, 80, 26, -2, true, 26, 3, 0x83FFDC, -1, &p6num2_Data);
	p6l2 = LabelInit(p6, 107, 126, 35, 26, -2, true, 25, 0, -1, -1, &p6l2_Data);
	p6l3 = LabelInit(p6, 107, 164, 38, 26, -2, true, 24, 0, -1, -1, &p6l3_Data);
	p6num3 = NumberInit(p6, 149, 164, 80, 26, -2, true, 23, 3, 0x83FFDC, -1, &p6num3_Data);
	p6l4 = LabelInit(p6, 253, 126, 30, 26, -2, true, 22, 0, -1, -1, &p6l4_Data);
	p6num4 = NumberInit(p6, 295, 126, 80, 26, -2, true, 21, 3, 0x83FFDC, -1, &p6num4_Data);
	p6l5 = LabelInit(p6, 253, 164, 33, 26, -2, true, 20, 0, -1, -1, &p6l5_Data);
	p6num5 = NumberInit(p6, 295, 164, 80, 26, -2, true, 19, 3, 0x83FFDC, -1, &p6num5_Data);
	p6l6 = LabelInit(p6, 145, 76, 191, 40, -2, true, 18, 0, -1, 1, &p6l6_Data);
	p6rect4 = RectangleInit(p6, 398, 66, 295, 220, -2, false, 17, -1, 100, &p6rect4_Data);
	p6l7 = LabelInit(p6, 414, 203, 142, 26, -2, true, 16, 0, -1, -1, &p6l7_Data);
	p6num6 = NumberInit(p6, 582, 203, 100, 26, -2, true, 15, 3, 0x83FFDC, -1, &p6num6_Data);
	p6num7 = NumberInit(p6, 457, 126, 80, 26, -2, true, 14, 3, 0x83FFDC, -1, &p6num7_Data);
	p6l8 = LabelInit(p6, 414, 126, 35, 26, -2, true, 13, 0, -1, -1, &p6l8_Data);
	p6l9 = LabelInit(p6, 413, 164, 38, 26, -2, true, 12, 0, -1, -1, &p6l9_Data);
	p6num8 = NumberInit(p6, 455, 164, 80, 26, -2, true, 11, 3, 0x83FFDC, -1, &p6num8_Data);
	p6l10 = LabelInit(p6, 560, 126, 30, 26, -2, true, 10, 0, -1, -1, &p6l10_Data);
	p6num9 = NumberInit(p6, 601, 126, 80, 26, -2, true, 9, 3, 0x83FFDC, -1, &p6num9_Data);
	p6l11 = LabelInit(p6, 559, 164, 33, 26, -2, true, 8, 0, -1, -1, &p6l11_Data);
	p6num10 = NumberInit(p6, 601, 164, 80, 26, -2, true, 7, 3, 0x83FFDC, -1, &p6num10_Data);
	p6l12 = LabelInit(p6, 419, 76, 256, 40, -2, true, 6, 0, -1, 1, &p6l12_Data);
	p6osc2 = OscilloscopeInit(p6, 398, 293, 295, 180, -2, true, 5, 0, 1, 1, 5, 3, 3, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, &p6osc2_Data);
	p6l13 = LabelInit(p6, 107, 240, 159, 26, -2, true, 4, 0, -1, -1, &p6l13_Data);
	p6num11 = NumberInit(p6, 255, 240, 120, 26, -2, true, 3, 3, 0x83FFDC, -1, &p6num11_Data);
	p6l14 = LabelInit(p6, 414, 240, 159, 26, -2, true, 2, 0, -1, -1, &p6l14_Data);
	p6num12 = NumberInit(p6, 562, 240, 120, 26, -2, true, 1, 3, 0x83FFDC, -1, &p6num12_Data);

	currentPage = p1;
	static lv_point_t valid_pos;
	valid_pos.x = 0;
	lv_tileview_set_valid_positions(tileview, &valid_pos, 1);
	lv_obj_set_event_cb(tileview, tileviewEventCallback);
	lv_obj_set_event_cb(p1, p1EventCallback);
	lv_obj_set_event_cb(p1b1, p1b1EventCallback);
	lv_obj_set_event_cb(p1b2, p1b2EventCallback);
	lv_obj_set_event_cb(p1b3, p1b3EventCallback);
	lv_obj_set_event_cb(p1b4, p1b4EventCallback);
	lv_obj_set_event_cb(p1b5, p1b5EventCallback);
	lv_obj_set_event_cb(p1b6, p1b6EventCallback);
	lv_obj_set_event_cb(p1b7, p1b7EventCallback);
	lv_obj_set_event_cb(p1ddl1, p1ddl1EventCallback);
	lv_obj_set_event_cb(p1b8, p1b8EventCallback);
	lv_obj_set_event_cb(p1tb1, p1tb1EventCallback);
	lv_obj_set_event_cb(p1tb2, p1tb2EventCallback);
	lv_obj_set_event_cb(p1ddl2, p1ddl2EventCallback);
	lv_obj_set_event_cb(p1b12, p1b12EventCallback);
	lv_obj_set_event_cb(p1b14, p1b14EventCallback);
	lv_obj_set_event_cb(p2, p2EventCallback);
	lv_obj_set_event_cb(p2b1, p2b1EventCallback);
	lv_obj_set_event_cb(p2b2, p2b2EventCallback);
	lv_obj_set_event_cb(p2b5, p2b5EventCallback);
	lv_obj_set_event_cb(p2b7, p2b7EventCallback);
	lv_obj_set_event_cb(p4, p4EventCallback);
	lv_obj_set_event_cb(p4b1, p4b1EventCallback);
	lv_obj_set_event_cb(p4tb1, p4tb1EventCallback);
	lv_obj_set_event_cb(p4b5, p4b5EventCallback);
	lv_obj_set_event_cb(p4b6, p4b6EventCallback);
	lv_obj_set_event_cb(p4b7, p4b7EventCallback);
	lv_obj_set_event_cb(p4b9, p4b9EventCallback);
	lv_obj_set_event_cb(p6, p6EventCallback);
	lv_obj_set_event_cb(p6b1, p6b1EventCallback);
	lv_obj_set_event_cb(p6b2, p6b2EventCallback);
	lv_obj_set_event_cb(p6b4, p6b4EventCallback);
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
}

void audioUpdate()
{
}

void oscUpdate()
{
	if (currentPage == p6 && p6osc1->refreshTimeUp()) p6osc1->refresh();
	if (currentPage == p6 && p6osc2->refreshTimeUp()) p6osc2->refresh();
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
		if (x == 2) enterEN = {p4, 0};
		if (x == 3) enterEN = {p6, 0};
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

void p1b3EventCallback(lv_obj_t* o, lv_event_t e)
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

void p1b4EventCallback(lv_obj_t* o, lv_event_t e)
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

void p1b5EventCallback(lv_obj_t* o, lv_event_t e)
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

void p1b6EventCallback(lv_obj_t* o, lv_event_t e)
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

void p1b7EventCallback(lv_obj_t* o, lv_event_t e)
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

void p1ddl1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		eventQueuePush({o, 0});
	}
}

void p1b8EventCallback(lv_obj_t* o, lv_event_t e)
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

void p1tb1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		ToggleButtonRefreshText(p1tb1);
		if(lv_btn_get_state(p1tb1) == LV_BTN_STATE_CHECKED_RELEASED || lv_btn_get_state(p1tb1) == LV_BTN_STATE_CHECKED_PRESSED || lv_btn_get_state(p1tb1) == LV_BTN_STATE_CHECKED_DISABLED)
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p1tb2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		ToggleButtonRefreshText(p1tb2);
		if(lv_btn_get_state(p1tb2) == LV_BTN_STATE_CHECKED_RELEASED || lv_btn_get_state(p1tb2) == LV_BTN_STATE_CHECKED_PRESSED || lv_btn_get_state(p1tb2) == LV_BTN_STATE_CHECKED_DISABLED)
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p1ddl2EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		eventQueuePush({o, 0});
	}
}

void p1b12EventCallback(lv_obj_t* o, lv_event_t e)
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

void p1b14EventCallback(lv_obj_t* o, lv_event_t e)
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

void p2b7EventCallback(lv_obj_t* o, lv_event_t e)
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

void p4tb1EventCallback(lv_obj_t* o, lv_event_t e)
{
	if(e == LV_EVENT_VALUE_CHANGED)
	{
		ToggleButtonRefreshText(p4tb1);
		if(lv_btn_get_state(p4tb1) == LV_BTN_STATE_CHECKED_RELEASED || lv_btn_get_state(p4tb1) == LV_BTN_STATE_CHECKED_PRESSED || lv_btn_get_state(p4tb1) == LV_BTN_STATE_CHECKED_DISABLED)
		{
			eventQueuePush({o, 0});
		}
		else
		{
			eventQueuePush({o, 1});
		}
	}
}

void p4b5EventCallback(lv_obj_t* o, lv_event_t e)
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

void p4b6EventCallback(lv_obj_t* o, lv_event_t e)
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

void p4b7EventCallback(lv_obj_t* o, lv_event_t e)
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

void p4b9EventCallback(lv_obj_t* o, lv_event_t e)
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

void p6b4EventCallback(lv_obj_t* o, lv_event_t e)
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