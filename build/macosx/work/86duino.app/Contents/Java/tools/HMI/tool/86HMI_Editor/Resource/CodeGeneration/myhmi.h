#ifndef __MyHMI_H__
#define __MyHMI_H__

#include <lvgl86.h>
#include <AudioSample.h>

using namespace LVGL86;

#define BEGIN_HMI_EVENT_PROC	for (int _hmiEventCounter = 0; _hmiEventCounter < 128 && Hmi.eventPop(); _hmiEventCounter++) {
#define END_HMI_EVENT_PROC	} helperYield();

enum HMI_SN
{
};

enum MBOX_WIDTH
{
	MBOX_NORMAL = 0,
	MBOX_LARGER = 1,
	MBOX_LARGEST = 2,
};

enum ABANNER_POS
{
	ABANNER_TOP = 0,
	ABANNER_CENTER = 1,
	ABANNER_BOTTOM = 2,
};

class MyHMI
{
public:

	MyHMI();
	void begin();
	lv_obj_t* SN_TO_ID(int sn);
	lv_obj_t* NAME_TO_ID(char* name);
	int ID_TO_SN(lv_obj_t* id);
	char* ID_TO_NAME(lv_obj_t* id);
	int GET_SN(int p);
	int GET_SN(int p, HMI_SN obj, int id);

	/* General */
	void gotoPage(lv_obj_t* pageId);
	lv_obj_t* getCurrentPage();
	int getCurrentPageNumber();
	void setLanguage(int lang_id);
	void setEnable(lv_obj_t* obj, bool enable);

	void playSound(AudioSample* sample) { sample->play(); }
	void playSound(int soundNo);
	void playSound(char* description);
	void stopSound(AudioSample* sample) { sample->stop(); }
	void stopSound(int soundNo);
	void stopSound(char* description);
	void stopSounds() { Audio.stop(); }
	void setSoundVolume(int volume) { Audio.prepare(NULL, 1024, volume); }

	int showMessageBox(char* title, char* btn0 = "OK", char* btn1 = NULL, char* btn2 = NULL); // In the main loop, it's a blocking function. // But when it called in the helper task, it will continue to execute directly and always returns -1.
	void setMessageBoxWidth(MBOX_WIDTH width); // The MBOX_WIDTH can be modified with MBOX_NORMAL, MBOX_LARGER, or MBOX_LARGEST.

	void showAlarmBanner(char* msg, int bank = 0);
	void deleteAlarmBanner(int bank = -1);
	void setAlarmBannerSpacing(int space);
	void setAlarmBannerPosition(ABANNER_POS pos); // The ABANNER_POS can be modified with ABANNER_TOP, ABANNER_CENTER, or ABANNER_BOTTOM.

	/* Commands */

	/* States */

	/* Events */
	bool eventPop();
};

extern MyHMI Hmi;

#endif
