#include <lv_qrcode.h>
#include <lv_freetype.h>
#include <lv_lodepng.h>
#include <lv_sjpg.h>
#include "myhmi.h"

/* User's global code */

#define EVENT_QUEUE_SIZE 256
#define ALARM_BANNER_BANK_SIZE 16
#define LANG_UPDATE_INTERVAL 50
#define _ISOBJ(_id,_objsn) (ID_TO_SN(_id) / 1000 % 100 == _objsn)

MyHMI Hmi = *new MyHMI();

/* Struct declaration */
struct EventNode
{
	lv_obj_t* obj;
	int eventId;
};

/* Global variable */
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

static uint32_t langUpdateT;
static int langUpdateNum;

void widgetsInit();
void audioInit();
void audioLoad();
void audioUpdate();
void languageUpdate();
void pageTimerUpdate();
void imageGaugeUpdate();
void oscUpdate();
void consoleUpdate();
void hmi_task_handler();
EventNode* eventQueuePop();
bool eventQueuePush(EventNode e);
void alarmBannerRefreshText();
char* envPathCombine(char* env, char* path);

/* Widgets init declaration */

/* Widgets event_handler declaration */
void popUpMsgBoxEventCallback(lv_obj_t* o, lv_event_t e);

MyHMI::MyHMI()
{
}

void MyHMI::begin()
{
	if (inUse) return;
	lv86_init();
	char* buf = envPathCombine("IRSPATH", "FONT/ICON.TTF");
	lv_freetype_init(buf);
	free(buf);
	lv_split_jpeg_init();
	lv_lodepng_init();
	audioInit();
	audioLoad();
	widgetsInit();
	_taskRTC = myHelper.startLoop(100 * 1024);
	if (_taskRTC == NULL) return;
	myHelper.start(0);
	_taskRTC->registerFunc(hmi_task_handler);
	_taskRTC->start();
	inUse = true;
}

lv_obj_t* MyHMI::SN_TO_ID(int sn)
{
}

lv_obj_t* MyHMI::NAME_TO_ID(char* name)
{
}

int MyHMI::ID_TO_SN(lv_obj_t* id)
{
}

char* MyHMI::ID_TO_NAME(lv_obj_t* id)
{
}

int MyHMI::GET_SN(int p)
{
	return p < 1 || p > 999 ? 0 : p;
}

int MyHMI::GET_SN(int p, HMI_SN obj, int id)
{
}

void MyHMI::gotoPage(lv_obj_t* pageId)
{
}

lv_obj_t* MyHMI::getCurrentPage()
{
}

int MyHMI::getCurrentPageNumber()
{
	return ID_TO_SN(getCurrentPage());
}

void MyHMI::setLanguage(int lang_id)
{
}

void MyHMI::setEnable(lv_obj_t* obj, bool _Enable)
{
}

void MyHMI::playSound(int soundNo)
{
}

void MyHMI::playSound(char* description)
{
}

void MyHMI::stopSound(int soundNo)
{
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

/* States Function */

/* Events Function */

bool MyHMI::eventPop()
{
	popOutEvent = eventQueuePop();
	return popOutEvent->obj != NULL;
}

void widgetsInit()
{
}

void audioInit()
{
}

void audioLoad()
{
}

void audioUpdate()
{
}

void languageUpdate()
{
}

void pageTimerUpdate()
{
}

void imageGaugeUpdate()
{
}

void oscUpdate()
{
}

void consoleUpdate()
{
}

void hmi_task_handler()
{
	languageUpdate();
	audioUpdate();
	pageTimerUpdate();
	imageGaugeUpdate();
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

/* Widgets event_handler function */
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