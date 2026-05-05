#ifndef Console_H
#define Console_H

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

namespace LVGL86 {
	class Console
#ifndef WIN32
		: public Stream, public Printable
#endif
	{
	public:
		Console(bool _autoRefreshMode = true);
		~Console();
		lv_obj_t* begin(lv_obj_t* parent = NULL, int _x = 0, int _y = 0, int _width = 400, int _height = 240, int8_t _font = -1, uint16_t maxLineLen = 100, uint16_t maxLineCnt = 100, double textColor = -1, double bgColor = -1, bool _enable = true);

		bool refreshTimeUp();
		void refresh();

		void enableGradient();
		void disableGradient();
		void setBgColor(uint32_t c);
		void setBgStartColor(uint32_t c);
		void setTextColor(uint32_t c);

		void resetBgStartColor();
		void resetBgColor();
		void resetTextColor();

		bool isDefaultBgStartColor();
		bool isDefaultBgColor();
		bool isDefaultTextColor();

		uint32_t getBgStartColor();
		uint32_t getBgColor();
		uint32_t getTextColor();

		void setXY(int x, int y);
		void setSize(int w, int h);
		void setHMIColor(int colorId);
		void setHMITextFont(int fontId);
		void setEnable(bool enable);

		int getX() { return X; }
		int getY() { return Y; }
		int getWidth() { return Width; }
		int getHeight() { return Height; }
		int getHMIColor() { return get_color_id(obj); }
		int getHMITextFont() { return get_font_id(obj); }
		bool getEnable() { return Enable; }

		void scrollbarON(void);
		void scrollbarOFF(void);

		size_t write(const unsigned char* buffer, size_t size);
		virtual size_t write(unsigned char);
		void fflush(void);
		virtual int available(void);
		virtual int peek(void);
		virtual int read(void);

#ifndef WIN32
		using Print::write; // pull in write(str) and write(buf, size) from Print
		void printf(const char* fmt, ...);
		virtual size_t printTo(Print& p) const;
#endif
		lv_obj_t* getObj() { return obj; }

		lv_obj_user_data_t user_data;
	private:
		int X, Y, Width, Height;
		bool Enable;
		lv_obj_t* obj;

		struct Impl;
		Impl* pimpl;
	};
	extern Console console;
}

#endif