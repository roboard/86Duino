#ifndef LV_TEXTLIST_H
#define LV_TEXTLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef WIN32
#include "../lv_conf.h"
#include "lvgl.h"
#include <SD.h>
#else
#include "lv_conf.h"
#include "lvgl/lvgl.h"
#endif

namespace LVGL86 {
	class TextList {
	public:
		TextList();
		~TextList();

		lv_obj_t* begin(lv_obj_t* parent = NULL, int _x = 0, int _y = 0, int _width = 260, int _height = 300, bool _enable = true, bool _lnVisiable = true, uint32_t _lineNumHead = 1, uint32_t _lineNumTail = 9999, uint16_t _lineSpacingGap = 10);

		void print(uint32_t lineNumber, char* text);
		void clear();
		void focus(uint32_t lineNumber, bool alignCenter = true);
		void highlight(uint32_t lineNumber);
		void setViewRange(uint32_t startLine, uint32_t endLine);
		void openFile(const char* path, int skipline = 0);
		void openFile(int bank, const char* path, int skipline = 0);

		const char* getLine(uint32_t lineNumber, char* buf = NULL);
		uint32_t getViewRangeStart();
		uint32_t getViewRangeEnd();

		void setXY(int x, int y);
		void setSize(int w, int h);
		void setEnable(bool enable);
		void setLineRange(uint32_t head, uint32_t tail);
		void setLineNumVisiable(bool visiable);
		void setFocusLineColor(double color = -1);
		void setOddLineColor(double color = -1);
		void setEvenLineColor(double color = -1);
		void setNumColor(double color = -1);
		void setTextColor(double color = -1);
		void setNumFont(lv_font_t* font = NULL);
		void setTextFont(lv_font_t* font = NULL);
		void setLineSpacing(uint16_t val);
		void setTextIndent(int val);
		void setBorderSide(lv_border_side_t val);

		int getX() { return X; }
		int getY() { return Y; }
		int getWidth() { return Width; }
		int getHeight() { return Height; }
		bool getEnable() { return Enable; }
		bool getLineNumVisiable();
		double getOddLineColor();
		double getEvenLineColor();
		double getNumColor();
		double getTextColor();
		uint16_t getLineSpacing();
		int getTextIndent();
		lv_border_side_t getBorderSide();

		lv_obj_t* getObj() { return obj; }
		lv_obj_user_data_t user_data;
	private:
		int X, Y, Width, Height;
		bool Enable;

		lv_obj_t* obj;
		struct Impl;
		Impl* pimpl;
	};

	extern TextList textList;
}
#endif