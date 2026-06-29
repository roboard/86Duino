#ifndef __MyHMI_H__
#define __MyHMI_H__

#include <lvgl86.h>
#include <AudioSample.h>

using namespace LVGL86;

#define BEGIN_HMI_EVENT_PROC	for (int _hmiEventCounter = 0; _hmiEventCounter < 128 && Hmi.eventPop(); _hmiEventCounter++) {
#define END_HMI_EVENT_PROC	} helperYield();

enum HMI_SN
{
	HMI_PAGE = 0,
	HMI_ABAR = 2,
	HMI_ASL = 3,
	HMI_BAR = 4,
	HMI_B = 5,
	HMI_TB = 6,
	HMI_CB = 10,
	HMI_CH = 11,
	HMI_DDL = 14,
	HMI_GA = 15,
	HMI_IMG = 16,
	HMI_IB = 17,
	HMI_L = 19,
	HMI_NUM = 20,
	HMI_LED = 21,
	HMI_LI = 23,
	HMI_RO = 29,
	HMI_SL = 30,
	HMI_SP = 31,
	HMI_SW = 33,
	HMI_TBL = 34,
	HMI_TIN = 37,
	HMI_QR = 40,
	HMI_ITB = 41,
	HMI_OSC = 42,
	HMI_RECT = 43,
	HMI_NIN = 44,
	HMI_DIN = 45,
	HMI_CIR = 46,
	HMI_IBAR = 47,
	HMI_ISL = 48,
	HMI_IGA = 49,
	HMI_FB = 50,
	HMI_TL = 51,
	HMI_CSL = 52,
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
	void setEnable(Oscilloscope* obj, bool _Enable) { if (obj != NULL) setEnable(obj->getObj(), _Enable); }
	void setEnable(FileBrowser* obj, bool _Enable) { if (obj != NULL) setEnable(obj->getObj(), _Enable); }
	void setEnable(TextList* obj, bool _Enable) { if (obj != NULL) setEnable(obj->getObj(), _Enable); }
	void setEnable(Console* obj, bool _Enable) { if (obj != NULL) setEnable(obj->getObj(), _Enable); }

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
	void enablePageTimer(lv_obj_t* id);
	void enablePageTimer(int sn);
	void enablePageTimer(char* name);
	void disablePageTimer(lv_obj_t* id);
	void disablePageTimer(int sn);
	void disablePageTimer(char* name);
	void setArcBarValue(lv_obj_t* id, double value);
	void setArcBarValue(int sn, double value);
	void setArcBarValue(char* name, double value);
	void setArcSliderValue(lv_obj_t* id, int value);
	void setArcSliderValue(int sn, int value);
	void setArcSliderValue(char* name, int value);
	void setBarValue(lv_obj_t* id, double value);
	void setBarValue(int sn, double value);
	void setBarValue(char* name, double value);
	void setToggleButtonState(lv_obj_t* id, bool checked);
	void setToggleButtonState(int sn, bool checked);
	void setToggleButtonState(char* name, bool checked);
	void setCheckboxState(lv_obj_t* id, bool checked);
	void setCheckboxState(int sn, bool checked);
	void setCheckboxState(char* name, bool checked);
	void setChartValue(lv_obj_t* id, int channel, int index, int value);
	void setChartValue(int sn, int channel, int index, int value);
	void setChartValue(char* name, int channel, int index, int value);
	void setChartValues(lv_obj_t* id, int channel, int* values);
	void setChartValues(int sn, int channel, int* values);
	void setChartValues(char* name, int channel, int* values);
	void disableChartChannel(lv_obj_t* id, int channel);
	void disableChartChannel(int sn, int channel);
	void disableChartChannel(char* name, int channel);
	void enableChartChannel(lv_obj_t* id, int channel);
	void enableChartChannel(int sn, int channel);
	void enableChartChannel(char* name, int channel);
	void setGaugeValue(lv_obj_t* id, int value);
	void setGaugeValue(int sn, int value);
	void setGaugeValue(char* name, int value);
	void setGaugeValue(lv_obj_t* id, int value, int needle);
	void setGaugeValue(int sn, int value, int needle);
	void setGaugeValue(char* name, int value, int needle);
	void setImageHidden(lv_obj_t* id, bool hidden);
	void setImageHidden(int sn, bool hidden);
	void setImageHidden(char* name, bool hidden);
	void setImageRotate(lv_obj_t* id, double v);
	void setImageRotate(int sn, double v);
	void setImageRotate(char* name, double v);
	void setImageZoom(lv_obj_t* id, double v);
	void setImageZoom(int sn, double v);
	void setImageZoom(char* name, double v);
	void setImagePos(lv_obj_t* id, int x, int y);
	void setImagePos(int sn, int x, int y);
	void setImagePos(char* name, int x, int y);
	void setImageX(lv_obj_t* id, int x);
	void setImageX(int sn, int x);
	void setImageX(char* name, int x);
	void setImageY(lv_obj_t* id, int y);
	void setImageY(int sn, int y);
	void setImageY(char* name, int y);
	void setLabelText(lv_obj_t* id, char* text);
	void setLabelText(int sn, char* text);
	void setLabelText(char* name, char* text);
	void setLabelText(lv_obj_t* id, char* text, int langID);
	void setLabelText(int sn, char* text, int langID);
	void setLabelText(char* name, char* text, int langID);
	void setNumberValue(lv_obj_t* id, double value);
	void setNumberValue(int sn, double value);
	void setNumberValue(char* name, double value);
	void setLedBright(lv_obj_t* id, int bright);
	void setLedBright(int sn, int bright);
	void setLedBright(char* name, int bright);
	void setSliderValue(lv_obj_t* id, int value);
	void setSliderValue(int sn, int value);
	void setSliderValue(char* name, int value);
	void setSpinboxValue(lv_obj_t* id, double value);
	void setSpinboxValue(int sn, double value);
	void setSpinboxValue(char* name, double value);
	void setSwitchState(lv_obj_t* id, bool checked);
	void setSwitchState(int sn, bool checked);
	void setSwitchState(char* name, bool checked);
	void setTableCellText(lv_obj_t* id, int x, int y, char* text);
	void setTableCellText(int sn, int x, int y, char* text);
	void setTableCellText(char* name, int x, int y, char* text);
	void setTableCellValue(lv_obj_t* id, int x, int y, int value);
	void setTableCellValue(int sn, int x, int y, int value);
	void setTableCellValue(char* name, int x, int y, int value);
	void setTableCellValue(lv_obj_t* id, int x, int y, double value);
	void setTableCellValue(int sn, int x, int y, double value);
	void setTableCellValue(char* name, int x, int y, double value);
	void setTableAlign(lv_obj_t* id, int x, int y, lv_label_align_t align);
	void setTableAlign(int sn, int x, int y, lv_label_align_t align);
	void setTableAlign(char* name, int x, int y, lv_label_align_t align);
	void setTableAlign(lv_obj_t* id, int x1, int y1, int x2, int y2, lv_label_align_t align);
	void setTableAlign(int sn, int x1, int y1, int x2, int y2, lv_label_align_t align);
	void setTableAlign(char* name, int x1, int y1, int x2, int y2, lv_label_align_t align);
	void setTableRowAlign(lv_obj_t* id, int row, lv_label_align_t align);
	void setTableRowAlign(int sn, int row, lv_label_align_t align);
	void setTableRowAlign(char* name, int row, lv_label_align_t align);
	void setTableColAlign(lv_obj_t* id, int col, lv_label_align_t align);
	void setTableColAlign(int sn, int col, lv_label_align_t align);
	void setTableColAlign(char* name, int col, lv_label_align_t align);
	void setTableCellStyle(lv_obj_t* id, int x, int y, int styleID);
	void setTableCellStyle(int sn, int x, int y, int styleID);
	void setTableCellStyle(char* name, int x, int y, int styleID);
	void setTableCellStyle(lv_obj_t* id, int x1, int y1, int x2, int y2, int styleID);
	void setTableCellStyle(int sn, int x1, int y1, int x2, int y2, int styleID);
	void setTableCellStyle(char* name, int x1, int y1, int x2, int y2, int styleID);
	void setTableRowStyle(lv_obj_t* id, int row, int styleID);
	void setTableRowStyle(int sn, int row, int styleID);
	void setTableRowStyle(char* name, int row, int styleID);
	void setTableColStyle(lv_obj_t* id, int col, int styleID);
	void setTableColStyle(int sn, int col, int styleID);
	void setTableColStyle(char* name, int col, int styleID);
	void setTableStyleTextColor(lv_obj_t* id, int styleID, int color);
	void setTableStyleTextColor(int sn, int styleID, int color);
	void setTableStyleTextColor(char* name, int styleID, int color);
	void setTableStyleBgColor(lv_obj_t* id, int styleID, int color);
	void setTableStyleBgColor(int sn, int styleID, int color);
	void setTableStyleBgColor(char* name, int styleID, int color);
	void setTextInputText(lv_obj_t* id, char* text);
	void setTextInputText(int sn, char* text);
	void setTextInputText(char* name, char* text);
	void setQrCodeLinkSource(lv_obj_t* id, char* source);
	void setQrCodeLinkSource(int sn, char* source);
	void setQrCodeLinkSource(char* name, char* source);
	void setImageToggleButtonState(lv_obj_t* id, bool checked);
	void setImageToggleButtonState(int sn, bool checked);
	void setImageToggleButtonState(char* name, bool checked);
	void addOscSample(lv_obj_t* id, double value);
	void addOscSample(int sn, double value);
	void addOscSample(char* name, double value);
	void addOscSample(Oscilloscope* id, double value);
	void setOscTriggerMode(lv_obj_t* id, OSC_TRIGGER_MODE mode);
	void setOscTriggerMode(int sn, OSC_TRIGGER_MODE mode);
	void setOscTriggerMode(char* name, OSC_TRIGGER_MODE mode);
	void setOscTriggerMode(Oscilloscope* id, OSC_TRIGGER_MODE mode);
	void setOscTriggerLevel(lv_obj_t* id, double value);
	void setOscTriggerLevel(int sn, double value);
	void setOscTriggerLevel(char* name, double value);
	void setOscTriggerLevel(Oscilloscope* id, double value);
	void setOscTriggerOffset(lv_obj_t* id, long value);
	void setOscTriggerOffset(int sn, long value);
	void setOscTriggerOffset(char* name, long value);
	void setOscTriggerOffset(Oscilloscope* id, long value);
	void setOscTimeDiv(lv_obj_t* id, unsigned long value);
	void setOscTimeDiv(int sn, unsigned long value);
	void setOscTimeDiv(char* name, unsigned long value);
	void setOscTimeDiv(Oscilloscope* id, unsigned long value);
	void setOscYRange(lv_obj_t* id, double min, double max);
	void setOscYRange(int sn, double min, double max);
	void setOscYRange(char* name, double min, double max);
	void setOscYRange(Oscilloscope* id, double min, double max);
	void setOscYDiv(lv_obj_t* id, double value);
	void setOscYDiv(int sn, double value);
	void setOscYDiv(char* name, double value);
	void setOscYDiv(Oscilloscope* id, double value);
	void setNumberInputValue(lv_obj_t* id, double value);
	void setNumberInputValue(int sn, double value);
	void setNumberInputValue(char* name, double value);
	void setDateInput(lv_obj_t* id, int year, int month, int day);
	void setDateInput(int sn, int year, int month, int day);
	void setDateInput(char* name, int year, int month, int day);
	void setImageBarValue(lv_obj_t* id, double value);
	void setImageBarValue(int sn, double value);
	void setImageBarValue(char* name, double value);
	void setImageSliderValue(lv_obj_t* id, int value);
	void setImageSliderValue(int sn, int value);
	void setImageSliderValue(char* name, int value);
	void setImageGaugeValue(lv_obj_t* id, int value);
	void setImageGaugeValue(int sn, int value);
	void setImageGaugeValue(char* name, int value);
	void setFileBrowserFilter(lv_obj_t* id, char* filter, int filterIndex);
	void setFileBrowserFilter(int sn, char* filter, int filterIndex);
	void setFileBrowserFilter(char* name, char* filter, int filterIndex);
	void setFileBrowserFilter(FileBrowser* id, char* filter, int filterIndex);
	void setFileBrowserFilter(lv_obj_t* id, char* filter);
	void setFileBrowserFilter(int sn, char* filter);
	void setFileBrowserFilter(char* name, char* filter);
	void setFileBrowserFilter(FileBrowser* id, char* filter);
	void deleteFileBrowserFilter(lv_obj_t* id, int filterIndex);
	void deleteFileBrowserFilter(int sn, int filterIndex);
	void deleteFileBrowserFilter(char* name, int filterIndex);
	void deleteFileBrowserFilter(FileBrowser* id, int filterIndex);
	void deleteFileBrowserFilter(lv_obj_t* id);
	void deleteFileBrowserFilter(int sn);
	void deleteFileBrowserFilter(char* name);
	void deleteFileBrowserFilter(FileBrowser* id);
	void setFileBrowserDisk(lv_obj_t* id, int disk);
	void setFileBrowserDisk(int sn, int disk);
	void setFileBrowserDisk(char* name, int disk);
	void setFileBrowserDisk(FileBrowser* id, int disk);
	void setTextListLine(lv_obj_t* id, int lineNumber, char* text);
	void setTextListLine(int sn, int lineNumber, char* text);
	void setTextListLine(char* name, int lineNumber, char* text);
	void setTextListLine(TextList* id, int lineNumber, char* text);
	void importToTextList(lv_obj_t* id, char* path);
	void importToTextList(int sn, char* path);
	void importToTextList(char* name, char* path);
	void importToTextList(TextList* id, char* path);
	void importToTextList(lv_obj_t* id, char* path, int disk);
	void importToTextList(int sn, char* path, int disk);
	void importToTextList(char* name, char* path, int disk);
	void importToTextList(TextList* id, char* path, int disk);
	void clearTextList(lv_obj_t* id);
	void clearTextList(int sn);
	void clearTextList(char* name);
	void clearTextList(TextList* id);
	void focusTextListLine(lv_obj_t* id, int lineNumber);
	void focusTextListLine(int sn, int lineNumber);
	void focusTextListLine(char* name, int lineNumber);
	void focusTextListLine(TextList* id, int lineNumber);
	void setTextListViewRange(lv_obj_t* id, int startLine, int endLine);
	void setTextListViewRange(int sn, int startLine, int endLine);
	void setTextListViewRange(char* name, int startLine, int endLine);
	void setTextListViewRange(TextList* id, int startLine, int endLine);

	/* States */
	double getArcBarValue(lv_obj_t* id);
	double getArcBarValue(int sn);
	double getArcBarValue(char* name);
	int getArcSliderValue(lv_obj_t* id);
	int getArcSliderValue(int sn);
	int getArcSliderValue(char* name);
	double getBarValue(lv_obj_t* id);
	double getBarValue(int sn);
	double getBarValue(char* name);
	bool getToggleButtonState(lv_obj_t* id);
	bool getToggleButtonState(int sn);
	bool getToggleButtonState(char* name);
	bool getCheckboxState(lv_obj_t* id);
	bool getCheckboxState(int sn);
	bool getCheckboxState(char* name);
	int getDropDownListSelectedItem(lv_obj_t* id);
	int getDropDownListSelectedItem(int sn);
	int getDropDownListSelectedItem(char* name);
	int getGaugeValue(lv_obj_t* id);
	int getGaugeValue(int sn);
	int getGaugeValue(char* name);
	int getGaugeValue(lv_obj_t* id, int needle);
	int getGaugeValue(int sn, int needle);
	int getGaugeValue(char* name, int needle);
	int getImageX(lv_obj_t* id);
	int getImageX(int sn);
	int getImageX(char* name);
	int getImageY(lv_obj_t* id);
	int getImageY(int sn);
	int getImageY(char* name);
	double getNumberValue(lv_obj_t* id);
	double getNumberValue(int sn);
	double getNumberValue(char* name);
	int getListSelectedItem(lv_obj_t* id);
	int getListSelectedItem(int sn);
	int getListSelectedItem(char* name);
	int getRollerSelectedItem(lv_obj_t* id);
	int getRollerSelectedItem(int sn);
	int getRollerSelectedItem(char* name);
	int getSliderValue(lv_obj_t* id);
	int getSliderValue(int sn);
	int getSliderValue(char* name);
	double getSpinboxValue(lv_obj_t* id);
	double getSpinboxValue(int sn);
	double getSpinboxValue(char* name);
	int getSpinboxInteger(lv_obj_t* id);
	int getSpinboxInteger(int sn);
	int getSpinboxInteger(char* name);
	bool getSwitchState(lv_obj_t* id);
	bool getSwitchState(int sn);
	bool getSwitchState(char* name);
	char* getTextInputText(lv_obj_t* id);
	char* getTextInputText(int sn);
	char* getTextInputText(char* name);
	bool getImageToggleButtonState(lv_obj_t* id);
	bool getImageToggleButtonState(int sn);
	bool getImageToggleButtonState(char* name);
	double getNumberInputValue(lv_obj_t* id);
	double getNumberInputValue(int sn);
	double getNumberInputValue(char* name);
	int getNumberInputInteger(lv_obj_t* id);
	int getNumberInputInteger(int sn);
	int getNumberInputInteger(char* name);
	int getDateInputYear(lv_obj_t* id);
	int getDateInputYear(int sn);
	int getDateInputYear(char* name);
	int getDateInputMonth(lv_obj_t* id);
	int getDateInputMonth(int sn);
	int getDateInputMonth(char* name);
	int getDateInputDay(lv_obj_t* id);
	int getDateInputDay(int sn);
	int getDateInputDay(char* name);
	double getImageBarValue(lv_obj_t* id);
	double getImageBarValue(int sn);
	double getImageBarValue(char* name);
	int getImageSliderValue(lv_obj_t* id);
	int getImageSliderValue(int sn);
	int getImageSliderValue(char* name);
	int getImageGaugeValue(lv_obj_t* id);
	int getImageGaugeValue(int sn);
	int getImageGaugeValue(char* name);
	char* getFileBrowserFolder(lv_obj_t* id);
	char* getFileBrowserFolder(int sn);
	char* getFileBrowserFolder(char* name);
	char* getFileBrowserFolder(FileBrowser* id);
	char* getFileBrowserFilePath(lv_obj_t* id);
	char* getFileBrowserFilePath(int sn);
	char* getFileBrowserFilePath(char* name);
	char* getFileBrowserFilePath(FileBrowser* id);
	char* getTextListLine(lv_obj_t* id, int lineNumber);
	char* getTextListLine(int sn, int lineNumber);
	char* getTextListLine(char* name, int lineNumber);
	char* getTextListLine(TextList* id, int lineNumber);
	char* getTextListLine(lv_obj_t* id, int lineNumber, char* buf);
	char* getTextListLine(int sn, int lineNumber, char* buf);
	char* getTextListLine(char* name, int lineNumber, char* buf);
	char* getTextListLine(TextList* id, int lineNumber, char* buf);
	int getTextListViewRangeStart(lv_obj_t* id);
	int getTextListViewRangeStart(int sn);
	int getTextListViewRangeStart(char* name);
	int getTextListViewRangeStart(TextList* id);
	int getTextListViewRangeEnd(lv_obj_t* id);
	int getTextListViewRangeEnd(int sn);
	int getTextListViewRangeEnd(char* name);
	int getTextListViewRangeEnd(TextList* id);

	/* Events */
	bool pageEnter(lv_obj_t* id);
	bool pageEnter(int sn);
	bool pageEnter(char* name);
	bool pageLeave(lv_obj_t* id);
	bool pageLeave(int sn);
	bool pageLeave(char* name);
	bool pageTimerEventTriggered(lv_obj_t* id);
	bool pageTimerEventTriggered(int sn);
	bool pageTimerEventTriggered(char* name);
	bool arcSliderDragging(lv_obj_t* id);
	bool arcSliderDragging(int sn);
	bool arcSliderDragging(char* name);
	bool arcSliderDropped(lv_obj_t* id);
	bool arcSliderDropped(int sn);
	bool arcSliderDropped(char* name);
	bool buttonClicked(lv_obj_t* id);
	bool buttonClicked(int sn);
	bool buttonClicked(char* name);
	bool buttonPressing(lv_obj_t* id);
	bool buttonPressing(int sn);
	bool buttonPressing(char* name);
	bool toggleButtonChecked(lv_obj_t* id);
	bool toggleButtonChecked(int sn);
	bool toggleButtonChecked(char* name);
	bool toggleButtonUnchecked(lv_obj_t* id);
	bool toggleButtonUnchecked(int sn);
	bool toggleButtonUnchecked(char* name);
	bool checkboxChecked(lv_obj_t* id);
	bool checkboxChecked(int sn);
	bool checkboxChecked(char* name);
	bool checkboxUnchecked(lv_obj_t* id);
	bool checkboxUnchecked(int sn);
	bool checkboxUnchecked(char* name);
	bool dropDownListItemSelected(lv_obj_t* id);
	bool dropDownListItemSelected(int sn);
	bool dropDownListItemSelected(char* name);
	bool imageButtonClicked(lv_obj_t* id);
	bool imageButtonClicked(int sn);
	bool imageButtonClicked(char* name);
	bool imageButtonPressing(lv_obj_t* id);
	bool imageButtonPressing(int sn);
	bool imageButtonPressing(char* name);
	bool listItemSelected(lv_obj_t* id);
	bool listItemSelected(int sn);
	bool listItemSelected(char* name);
	bool rollerItemSelected(lv_obj_t* id);
	bool rollerItemSelected(int sn);
	bool rollerItemSelected(char* name);
	bool sliderDragging(lv_obj_t* id);
	bool sliderDragging(int sn);
	bool sliderDragging(char* name);
	bool sliderDropped(lv_obj_t* id);
	bool sliderDropped(int sn);
	bool sliderDropped(char* name);
	bool switchOn(lv_obj_t* id);
	bool switchOn(int sn);
	bool switchOn(char* name);
	bool switchOff(lv_obj_t* id);
	bool switchOff(int sn);
	bool switchOff(char* name);
	bool textInputApplied(lv_obj_t* id);
	bool textInputApplied(int sn);
	bool textInputApplied(char* name);
	bool textInputCanceled(lv_obj_t* id);
	bool textInputCanceled(int sn);
	bool textInputCanceled(char* name);
	bool imageToggleButtonChecked(lv_obj_t* id);
	bool imageToggleButtonChecked(int sn);
	bool imageToggleButtonChecked(char* name);
	bool imageToggleButtonUnchecked(lv_obj_t* id);
	bool imageToggleButtonUnchecked(int sn);
	bool imageToggleButtonUnchecked(char* name);
	bool numberInputApplied(lv_obj_t* id);
	bool numberInputApplied(int sn);
	bool numberInputApplied(char* name);
	bool numberInputCanceled(lv_obj_t* id);
	bool numberInputCanceled(int sn);
	bool numberInputCanceled(char* name);
	bool imageSliderDragging(lv_obj_t* id);
	bool imageSliderDragging(int sn);
	bool imageSliderDragging(char* name);
	bool imageSliderDropped(lv_obj_t* id);
	bool imageSliderDropped(int sn);
	bool imageSliderDropped(char* name);
	bool eventPop();
};

extern MyHMI Hmi;
extern lv_obj_t *p1;
extern lv_obj_t *p1rect5;
extern lv_obj_t *p1rect1;
extern lv_obj_t *p1rect2;
extern lv_obj_t *p1rect3;
extern lv_obj_t *p1rect4;
extern lv_obj_t *p1b1;
extern lv_obj_t *p1b2;
extern lv_obj_t *p1b3;
extern lv_obj_t *p1b4;
extern lv_obj_t *p1b5;
extern lv_obj_t *p1b6;
extern lv_obj_t *p1abar1;
extern lv_obj_t *p1abar2;
extern lv_obj_t *p1abar3;
extern lv_obj_t *p1led1;
extern lv_obj_t *p1led2;
extern lv_obj_t *p1led3;
extern lv_obj_t *p1num1;
extern lv_obj_t *p1num2;
extern lv_obj_t *p1num3;
extern lv_obj_t *p1num4;
extern lv_obj_t *p1num5;
extern lv_obj_t *p1num6;
extern lv_obj_t *p1l1;
extern lv_obj_t *p1l2;
extern lv_obj_t *p1l3;
extern lv_obj_t *p1l4;
extern lv_obj_t *p1l5;
extern lv_obj_t *p1l6;
extern lv_obj_t *p1b7;
extern lv_obj_t *p1ddl1;
extern lv_obj_t *p1l7;
extern lv_obj_t *p1b8;
extern lv_obj_t *p1tb1;
extern lv_obj_t *p1tb2;
extern lv_obj_t *p1ddl2;
extern lv_obj_t *p1l8;
extern lv_obj_t *p1b12;
extern lv_obj_t *p1b14;

extern lv_obj_t *p2;
extern lv_obj_t *p2rect4;
extern lv_obj_t *p2b1;
extern FileBrowser *p2fb1;
extern lv_obj_t *p2b2;
extern lv_obj_t *p2b5;
extern lv_obj_t *p2l7;
extern lv_obj_t *p2b7;

extern lv_obj_t *p4;
extern lv_obj_t *p4rect1;
extern lv_obj_t *p4rect2;
extern lv_obj_t *p4rect3;
extern lv_obj_t *p4rect5;
extern lv_obj_t *p4abar1;
extern lv_obj_t *p4abar2;
extern lv_obj_t *p4abar3;
extern lv_obj_t *p4led1;
extern lv_obj_t *p4led2;
extern lv_obj_t *p4led3;
extern lv_obj_t *p4num1;
extern lv_obj_t *p4num2;
extern lv_obj_t *p4num3;
extern lv_obj_t *p4num4;
extern lv_obj_t *p4num5;
extern lv_obj_t *p4num6;
extern lv_obj_t *p4l1;
extern lv_obj_t *p4l2;
extern lv_obj_t *p4l3;
extern lv_obj_t *p4rect4;
extern lv_obj_t *p4l4;
extern lv_obj_t *p4l5;
extern lv_obj_t *p4l6;
extern lv_obj_t *p4b1;
extern lv_obj_t *p4tb1;
extern lv_obj_t *p4b5;
extern lv_obj_t *p4b6;
extern lv_obj_t *p4b7;
extern lv_obj_t *p4b9;
extern TextList *p4tl1;

extern lv_obj_t *p6;
extern lv_obj_t *p6rect1;
extern lv_obj_t *p6b1;
extern lv_obj_t *p6b2;
extern lv_obj_t *p6b4;
extern Oscilloscope *p6osc1;
extern lv_obj_t *p6rect3;
extern lv_obj_t *p6l1;
extern lv_obj_t *p6num1;
extern lv_obj_t *p6num2;
extern lv_obj_t *p6l2;
extern lv_obj_t *p6l3;
extern lv_obj_t *p6num3;
extern lv_obj_t *p6l4;
extern lv_obj_t *p6num4;
extern lv_obj_t *p6l5;
extern lv_obj_t *p6num5;
extern lv_obj_t *p6l6;
extern lv_obj_t *p6rect4;
extern lv_obj_t *p6l7;
extern lv_obj_t *p6num6;
extern lv_obj_t *p6num7;
extern lv_obj_t *p6l8;
extern lv_obj_t *p6l9;
extern lv_obj_t *p6num8;
extern lv_obj_t *p6l10;
extern lv_obj_t *p6num9;
extern lv_obj_t *p6l11;
extern lv_obj_t *p6num10;
extern lv_obj_t *p6l12;
extern Oscilloscope *p6osc2;
extern lv_obj_t *p6l13;
extern lv_obj_t *p6num11;
extern lv_obj_t *p6l14;
extern lv_obj_t *p6num12;


#endif
