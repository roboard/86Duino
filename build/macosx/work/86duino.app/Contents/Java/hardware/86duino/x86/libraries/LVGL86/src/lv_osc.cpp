#include "lv_osc.h"
#include "math.h"
#ifndef WIN32
#include "irq.h"
#endif

using namespace LVGL86;

#ifndef WIN32
extern unsigned int DRW;

static unsigned long long _micros() {
	return (timer_GetClocks64() / CLOCKS_PER_MICROSEC);
}

static unsigned long long int timer_NowTime64() {
	return timer_GetClocks64() / VORTEX86EX_CLOCKS_PER_MS;
}
#endif

void morphNumericString(char* s)
{
	char* p, * end, * decimal, * nonzero;

	end = p = strchr(s, '\0');
	decimal = nonzero = NULL;
	while (p > s)
	{
		p--;
		if (!nonzero && *p != '0')
			nonzero = p;
		if (!decimal && *p == '.')
		{
			decimal = p;
			break;
		}
	}
	if (decimal && nonzero && nonzero > decimal)
		*(nonzero + 1) = '\0';
	else
		*(decimal ? decimal : end) = '\0';
}

Oscilloscope::Oscilloscope(bool _realtime, bool _autoRefresh, unsigned long _sampleTime, unsigned long _bufSize, uint8_t _maxDec)
{

	TriggerMode = OSC_FREE_RUN, TriggerLevel = 0, Offset = 0;
	TimeDiv = 1000, MinValue = -10, MaxValue = 10;
	FPS = 15, DisplayQuality = 5;


	X = 0, Y = 0, Width = 390, Height = 260, Enable = true;
	DivLineV = 4, DivLineH = 3, DivLineType = OSC_DASHED_LINE, BorderX = OSC_NO_SIDE, BorderY = OSC_NO_SIDE, BorderThickness = OSC_NORMAL_BORDER;
	TickTextEnableX = true, TickTextEnableY = true, IntervalTicksX = 0, IntervalTicksY = 0, TickSize = 1;
	ChannelColor = 0x01A2B1;
	PadSizeL = -1, PadSizeR = -1, PadSizeT = -1, PadSizeB = -1;
	Font = NULL;

	refreshTimer = 0, rtTimer = 0;
	refreshFSM = 0, trigger_p = 0, prett = -1;
	ticktextX = NULL, ticktextY = NULL;
	intBuf = NULL, intTimerBuf = NULL;
	sampleBuf = NULL;
	obj = NULL, cst = NULL;

#ifndef WIN32
	intBuf = (int*)malloc(sizeof(int) * 1024);
	intTimerBuf = (unsigned long long*)malloc(sizeof(unsigned long long) * 1024);
#endif
	sBufFull = false;
	sBufPtr = 0, intBufCnt = -1;

	IsReady = false;
	configRealtimeMode(_realtime);
	configAutoRefreshMode(_autoRefresh);
	configSampleingCycle(_sampleTime);
	configSampleBufferSize(_bufSize);
	configDecimalPlaces(_maxDec);
}

Oscilloscope::~Oscilloscope()
{
	if (ticktextX) free(ticktextX);
	if (ticktextY) free(ticktextY);
	if (intBuf) free(intBuf);
	if (intTimerBuf) free(intTimerBuf);
	if (sampleBuf) free(sampleBuf);

	if (obj) lv_obj_del(obj);
	obj = NULL;
}

lv_obj_t* Oscilloscope::begin(lv_obj_t* parent, int _x, int _y, int _width, int _height, bool _enable,
	uint8_t _divLineV, uint8_t _divLineH, OSC_DIVISION_LINE_TYPE _divLineType, OSC_BORDER_SIDE_TYPE _borderX, OSC_BORDER_SIDE_TYPE _borderY, OSC_BORDER_THICKNESS_TYPE _borderThickness, bool _enableTickTextX, bool _enableTickTextY,
	uint8_t _interTickX, uint8_t _interTickY, uint8_t _tickSize, double _channelColor,
	unsigned long _timeDiv, double _min, double _max)
{
	if (parent == NULL)
		obj = lv_chart_create(lv_scr_act(), NULL);
	else
		obj = lv_chart_create(parent, NULL);
	lv_chart_set_type(obj, LV_CHART_TYPE_LINE);
	lv_obj_set_style_local_size(obj, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_opa(obj, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, LV_OPA_0);
	lv_obj_set_style_local_bg_grad_dir(obj, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
	lv_obj_set_style_local_bg_main_stop(obj, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_grad_stop(obj, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, 0);

	setXY(_x, _y);
	setSize(_width, _height);
	setEnable(_enable);
	setDivisionLines(_divLineV, _divLineH);
	setDivisionLineType(_divLineType);
	setBorderSide(_borderX, _borderY);
	setBorderThickness(_borderThickness);
	setTickTextEnable(_enableTickTextX, _enableTickTextY);
	setIntervalTicks(_interTickX, _interTickY);
	setTickSize(_tickSize);
	setChannelColor(_channelColor);
	setLeftPadSize(PadSizeL);
	setRightPadSize(PadSizeR);
	setTopPadSize(PadSizeT);
	setBottomPadSize(PadSizeB);

	setTriggerMode(TriggerMode);
	setTriggerLevel(TriggerLevel);
	setOffset(Offset);
	setTimeDiv(_timeDiv);
	setYRange(_min, _max);
	setDisplayQuality(DisplayQuality);
	setFPS(FPS);

	IsReady = true;
	refreshText();
	return obj;
}

bool Oscilloscope::refreshTimeUp()
{
#ifndef WIN32
	return (timer_NowTime64() - refreshTimer) * FPS >= 1000;
#else
	return false;
#endif
}

void Oscilloscope::refresh()
{
#ifndef WIN32
	int i, sId, sId2, lsId, cId, cId2, cpc, tcnt;
	double f, tt;
	if (!IsReady) return;
	if (!sBufFull && sBufPtr == 0) return;

	io_DisableINT();
	intBufCnt = 0;
	io_RestoreINT();

	cpc = lv_chart_get_point_count(obj);
	tt = (double)TimeDiv * (DivLineV + 1) / SampleingCycle;
	lsId = (sBufPtr > 0 ? sBufPtr : SampleBufferSize) - 1;
	cId2 = 0;
	if (TriggerMode == OSC_FREE_RUN)
	{
		for (cId = 1; cId <= cpc; cId++)
		{
			i = (int)(tt * cId / cpc);
			f = (double)tt * cId / cpc - i;
			sId = (int)(lsId - tt + i + Offset + SampleBufferSize) % SampleBufferSize;
			sId2 = (sId + 1) % SampleBufferSize;
			if (sBufFull || (!sBufFull && sId >= 0 && sId <= lsId))
				lv_chart_set_point_id(obj, cst, sampleBuf[sId] + f * (sampleBuf[sId2] - sampleBuf[sId]), cId2++);
		}
		while (cId2 < cpc)
			lv_chart_set_point_id(obj, cst, LV_CHART_POINT_DEF, cId2++);
	}
	else
	{
		if (tt != prett)
		{
			refreshFSM = 0;
			prett = tt;
			cst->color = lv_color_hex(0xD4D7D9);
		}
		switch (refreshFSM)
		{
		case 0:
			trigger_p = lsId;
			refreshFSM = 1;
			break;
		case 1:
			while (refreshFSM == 1 && trigger_p != lsId)
			{
				sId = (trigger_p + 1) % SampleBufferSize;
				if ((TriggerMode == OSC_POSITIVE_SLOPE && sampleBuf[trigger_p] < TriggerLevel && sampleBuf[sId] >= TriggerLevel) ||
					(TriggerMode == OSC_NEGATIVE_SLOPE && sampleBuf[trigger_p] > TriggerLevel && sampleBuf[sId] <= TriggerLevel))
					refreshFSM = 2;
				trigger_p = sId;
			}
			break;
		case 2:
			tcnt = lsId - trigger_p + (trigger_p > lsId ? SampleBufferSize : 0);
			if (tcnt >= tt + Offset || tcnt + 2000000.0 / FPS / SampleingCycle >= SampleBufferSize)
			{
				for (cId = 0; cId < cpc; cId++)
				{
					i = (int)(tt * cId / cpc);
					f = (double)tt * cId / cpc - i;
					sId = (int)(trigger_p + i + Offset + SampleBufferSize) % SampleBufferSize;
					sId2 = (sId + 1) % SampleBufferSize;
					if ((sBufFull && i < tcnt) || (!sBufFull && sId < lsId))
						lv_chart_set_point_id(obj, cst, sampleBuf[sId] + f * (sampleBuf[sId2] - sampleBuf[sId]), cId2++);
					else
						break;
				}
				while (cId2 < cpc)
					lv_chart_set_point_id(obj, cst, LV_CHART_POINT_DEF, cId2++);
				refreshFSM = 0;
			}
			break;
		}
	}
	if (cId2 > 0)
		cst->color = lv_color_hex(ChannelColor);

	io_DisableINT();
	for (i = 0; i < intBufCnt; i++)
		if (RealtimeMode)
			insertSample(intBuf[i], intTimerBuf[i]);
		else
			insertSample(intBuf[i]);
	intBufCnt = -1;
	io_RestoreINT();

	lv_chart_refresh(obj);
	refreshTimer = timer_NowTime64();
#endif
}

void Oscilloscope::addSample(double sample)
{
#ifndef WIN32
	int val;
	if (!IsReady) return;

	val = sample * Magnification;

	if (RealtimeMode)
		insertSample(val, _micros());
	else
		insertSample(val);
	if (AutoRefreshMode && refreshTimeUp())
	{
		refresh();
		lv_task_handler();
	}
#endif
}

void Oscilloscope::addSampleInInterrupt(int Numerator, int denominator)
{
#ifndef WIN32
	int val;
	if (irq_InInterrupt() == false) return;
	if (!IsReady) return;

	val = (long long)Numerator * Magnification / denominator;

	if (intBufCnt < 0)
	{
		if (RealtimeMode)
			insertSample(val, _micros());
		else
			insertSample(val);
	}
	else if (intBufCnt < 1024)
	{
		if (RealtimeMode)
			intTimerBuf[intBufCnt] = _micros();
		intBuf[intBufCnt++] = val;
	}
#endif
}

void Oscilloscope::clear(void)
{
	if (obj == NULL) return;

#ifndef WIN32
	io_DisableINT();
	sBufPtr = 0;
	sBufFull = false;
	io_RestoreINT();
#endif
	lv_chart_clear_series(obj, cst);
	lv_chart_refresh(obj);
}


void Oscilloscope::configRealtimeMode(bool mode)
{
	if (IsReady) return;
	RealtimeMode = mode;
}

void Oscilloscope::configAutoRefreshMode(bool mode)
{
	if (IsReady) return;
	AutoRefreshMode = mode;
}

void Oscilloscope::configSampleingCycle(unsigned long val)
{
	if (IsReady) return;
	if (val == 0) return;
	SampleingCycle = val;
}

void Oscilloscope::configSampleBufferSize(unsigned long size)
{
	if (IsReady) return;
	SampleBufferSize = size;
#ifndef WIN32
	if (sampleBuf != NULL) free(sampleBuf);
	sampleBuf = (int*)malloc(sizeof(int) * SampleBufferSize);
#endif
}

void Oscilloscope::configDecimalPlaces(uint8_t n)
{
	int i;
	if (n > 6) return;
	if (IsReady) return;
	DecimalPlaces = n;
	Magnification = pow(10, DecimalPlaces);
}

void Oscilloscope::setTriggerMode(OSC_TRIGGER_MODE mode)
{
	if (mode == TriggerMode) return;
	prett = -1;
	TriggerMode = mode;
}

void Oscilloscope::setTriggerLevel(double val)
{
	if (TriggerLevel == val * Magnification) return;
	prett = -1;
	TriggerLevel = val * Magnification;
}

void Oscilloscope::setOffset(long microSec)
{
	if (microSec / SampleingCycle + SampleBufferSize < 0) return;
	if (Offset == (double)microSec / SampleingCycle) return;
	prett = -1;
	Offset = (double)microSec / SampleingCycle;
}

void Oscilloscope::setTimeDiv(unsigned long microSec)
{
	TimeDiv = microSec;
	if (TimeDiv * (DivLineV + 1) / SampleingCycle > SampleBufferSize)
		TimeDiv = (double)SampleBufferSize / (DivLineV + 1) * SampleingCycle;
	const char* unit[3] = { "µs", "ms", "s" };
	int sizeCnt, i, j, k;
	double t;
	if (TickTextEnableX)
	{
		for (sizeCnt = 0, i = 0; i <= (DivLineV + 1); i++)
		{
			sizeCnt += 10;
			if (TimeDiv * i >= 1000000000)
				for (j = TimeDiv * i / 1000000000; j > 0; j /= 10) sizeCnt++;
		}
		if (ticktextX != NULL) free(ticktextX);
		ticktextX = (char*)malloc(sizeCnt * sizeof(char));
		sprintf(ticktextX, "0");
		for (i = 1; i <= (DivLineV + 1); i++)
		{
			t = TimeDiv * i;
			j = 0;
			while (t >= 1000 && j < 2)
			{
				t /= 1000;
				j++;
			}
			if (i == 1) sprintf(ticktextX + strlen(ticktextX), "%s", unit[j]);
			sprintf(ticktextX + strlen(ticktextX), "\n%.3lf", t);
			morphNumericString(ticktextX);
			sprintf(ticktextX + strlen(ticktextX), "%s", unit[j]);
		}
	}
	else
	{
		if (ticktextX != NULL) free(ticktextX);
		ticktextX = (char*)malloc((DivLineV + 2) * sizeof(char));
		memset(ticktextX, '\n', DivLineV + 1);
		ticktextX[DivLineV + 1] = '\0';
	}
	refreshText();
}

void Oscilloscope::setYRange(double min, double max)
{
	int sizeCnt, i, j;
	if (obj == NULL) return;
	MinValue = min * Magnification;
	MaxValue = max * Magnification;
	lv_chart_set_y_range(obj, LV_CHART_AXIS_PRIMARY_Y, MinValue, MaxValue);
	if (TickTextEnableY)
	{
		for (sizeCnt = 1, i = 0; i <= (DivLineH + 1); i++)
		{
			sizeCnt += 3;
			for (j = MaxValue - ((double)(MaxValue - MinValue) * i / (DivLineH + 1)); j != 0; j /= 10) sizeCnt++;
		}
		if (ticktextY != NULL) free(ticktextY);
		ticktextY = (char*)malloc(sizeCnt * sizeof(char));
		sprintf(ticktextY, "%.3lf", (double)MaxValue / Magnification);
		morphNumericString(ticktextY);
		for (i = 1; i <= (DivLineH + 1); i++)
		{
			sprintf(ticktextY + strlen(ticktextY), "\n%.3lf", ((double)MaxValue - ((double)(MaxValue - MinValue) * i / (DivLineH + 1))) / Magnification);
			morphNumericString(ticktextY);
		}
	}
	else
	{
		if (ticktextY != NULL) free(ticktextY);
		ticktextY = (char*)malloc((DivLineH + 2) * sizeof(char));
		memset(ticktextY, '\n', DivLineH + 1);
		ticktextY[DivLineH + 1] = '\0';
	}
	refreshText();
}

void Oscilloscope::setFPS(unsigned int fps)
{
	if (fps > 60) return;
	FPS = fps;
}

void Oscilloscope::setDisplayQuality(unsigned int gap)
{
	if (obj == NULL) return;
	int w = lv_obj_get_width(obj);
	DisplayQuality = gap;
#ifndef WIN32
	lv_chart_set_point_count(obj, (w > DRW ? DRW : w) / DisplayQuality);
#else
	lv_chart_set_point_count(obj, w / DisplayQuality);
#endif
}

void Oscilloscope::setXY(int x, int y)
{
	if (obj == NULL) return;
	X = x;
	Y = y;
	lv_obj_set_pos(obj, X, Y);
}

void Oscilloscope::setSize(int w, int h)
{
	if (obj == NULL) return;
	Width = w;
	Height = h;
	lv_obj_set_size(obj, Width, Height);
}

void Oscilloscope::setEnable(bool enable)
{
	if (obj == NULL) return;
	Enable = enable;
	void (*enable_cb)(lv_obj_t*, lv_state_t) = Enable ? lv_obj_clear_state : lv_obj_add_state;
	enable_cb(obj, LV_STATE_DISABLED);
}

void Oscilloscope::setDivisionLines(uint8_t x, uint8_t y)
{
	if (obj == NULL) return;
	if (x < 0 || x > 30 || y < 0 || y > 30) return;
	DivLineH = y;
	DivLineV = x;
	lv_chart_set_div_line_count(obj, DivLineH, DivLineV);

	if (!IsReady) return;
	if (TickTextEnableX) setTimeDiv(TimeDiv);
	if (TickTextEnableY) setYRange((double)MinValue / Magnification, (double)MaxValue / Magnification);
}

void Oscilloscope::setDivisionLineType(OSC_DIVISION_LINE_TYPE type)
{
	if (obj == NULL) return;
	DivLineType = type;
	if (DivLineType == OSC_NO_LINE)
		lv_obj_set_style_local_line_width(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, 0);
	else if (DivLineType == OSC_SOLID_LINE)
	{
		lv_obj_set_style_local_line_width(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, 1);
		lv_obj_set_style_local_line_dash_gap(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, 0);
	}
	else if (DivLineType == OSC_DASHED_LINE)
	{
		lv_obj_set_style_local_line_width(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, 1);
		lv_obj_set_style_local_line_dash_gap(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, 8);
	}
}

void Oscilloscope::setBorderSide(OSC_BORDER_SIDE_TYPE typeX, OSC_BORDER_SIDE_TYPE typeY)
{
	if (obj == NULL) return;
	BorderX = typeX;
	BorderY = typeY;
	lv_obj_set_style_local_border_side(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, (BorderX << 0) | (BorderY << 2));
	lv_obj_set_style_local_border_width(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, lv_obj_get_style_border_side(obj, LV_CHART_PART_SERIES_BG) > 0 ? BorderThickness + 1 : 0);
}

void Oscilloscope::setBorderThickness(OSC_BORDER_THICKNESS_TYPE type)
{
	if (obj == NULL) return;
	BorderThickness = type;
	lv_obj_set_style_local_border_side(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, (BorderX << 0) | (BorderY << 2));
	lv_obj_set_style_local_border_width(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, lv_obj_get_style_border_side(obj, LV_CHART_PART_SERIES_BG) > 0 ? BorderThickness + 1 : 0);
}

void Oscilloscope::setTickTextEnable(bool x, bool y)
{
	bool fx, fy;
	fx = TickTextEnableX != x;
	fy = TickTextEnableY != y;
	TickTextEnableX = x;
	TickTextEnableY = y;
	if (!IsReady) return;
	if (fx) setTimeDiv(TimeDiv);
	if (fy) setYRange((double)MinValue / Magnification, (double)MaxValue / Magnification);
}

void Oscilloscope::setIntervalTicks(uint8_t x, uint8_t y)
{
	if (x < 10) IntervalTicksX = x;
	if (y < 10) IntervalTicksY = y;
	refreshText();
}

void Oscilloscope::setTickSize(uint8_t size)
{
	TickSize = size;
	refreshText();
}

void Oscilloscope::setChannelColor(double color)
{
	if (obj == NULL) return;
	ChannelColor = color;

	if (cst == NULL)
		cst = lv_chart_add_series(obj, lv_color_hex((uint32_t)(ChannelColor < 0 ? get_theme_color(0, 0) : ChannelColor)));
	else
		cst->color = lv_color_hex((uint32_t)(ChannelColor < 0 ? get_theme_color(0, 0) : ChannelColor));
#ifndef WIN32
	refresh();
#else
	lv_obj_refresh_style(obj, LV_OBJ_PART_MAIN, LV_STYLE_PROP_ALL);
#endif
}

double Oscilloscope::getChannelColor()
{
	if (cst == NULL) return ChannelColor;
	double c = (double)lv_color_to32(cst->color);
	return c == get_theme_color(0, 0) ? -c : c;
}

void Oscilloscope::setLeftPadSize(int size)
{
	PadSizeL = size;
	refreshText();
}

void Oscilloscope::setRightPadSize(int size)
{
	PadSizeR = size;
	refreshText();
}

void Oscilloscope::setTopPadSize(int size)
{
	PadSizeT = size;
	refreshText();
}

void Oscilloscope::setBottomPadSize(int size)
{
	PadSizeB = size;
	refreshText();
}

void Oscilloscope::setHMITextFont(int fontId)
{
	if (obj == NULL) return;
	set_font_id(obj, fontId);
}

void Oscilloscope::setTextFont(lv_font_t* font)
{
	if (obj == NULL) return;
	lv_obj_set_style_local_text_font(obj, LV_CHART_PART_BG, LV_STATE_DEFAULT, font);
	lv_obj_set_style_local_text_font(obj, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, font);
	lv_obj_set_style_local_text_font(obj, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, font);
	lv_obj_set_style_local_text_font(obj, LV_CHART_PART_CURSOR, LV_STATE_DEFAULT, font);
	Font = font;
}

void Oscilloscope::sinWaveDemo(double offset)
{
	int scale = Magnification * 9, shift = 0;
	lv_ll_t* ll_p = &((lv_chart_ext_t*)lv_obj_get_ext_attr(obj))->series_ll;
	const double pi = 3.1415926;
	int16_t cnt = lv_chart_get_point_count(obj);
	lv_coord_t y = 0;
	lv_chart_series_t* s = (lv_chart_series_t*)_lv_ll_get_tail(ll_p);
	for (int count = 0; s != NULL; s = (lv_chart_series_t*)_lv_ll_get_prev(ll_p, s), count++)
	{
		double f = offset;
		for (int i = 0; i < cnt && count < 4; i++)
		{
			if (count == 0) y = (lv_coord_t)(round(sin(f * pi / 180) * scale) + shift);
			if (count == 1) y = (lv_coord_t)(round(cos(f * pi / 180) * scale) + shift);
			if (count == 2) y = (lv_coord_t)(round(-sin(f * pi / 180) * scale) + shift);
			if (count == 3) y = (lv_coord_t)(round(-cos(f * pi / 180) * scale) + shift);
			lv_chart_set_point_id(obj, s, y <= MinValue ? MinValue : (y >= MaxValue ? MaxValue : y), i);
			f += 360.0 / (cnt - 1);
		}
	}
	lv_chart_refresh(obj);
}

void Oscilloscope::refreshText()
{
	int pad;
	char* buf, * p;
	if (!IsReady) return;

	lv_point_t size;
	lv_draw_label_dsc_t label_dsc;
	lv_draw_label_dsc_init(&label_dsc);
	lv_obj_init_draw_label_dsc(obj, LV_CHART_PART_BG, &label_dsc);

	if (ticktextX != NULL)
	{
		pad = 0;
		buf = strdup(ticktextX);
		for (p = strtok(buf, "\n"); p != NULL; p = strtok(NULL, "\n"))
		{
			_lv_txt_get_size(&size, p, label_dsc.font, label_dsc.letter_space, label_dsc.line_space, LV_COORD_MAX, LV_TXT_FLAG_CENTER);
			if (pad < size.y) pad = size.y;
		}
		free(buf);
		lv_obj_set_style_local_pad_bottom(obj, LV_CHART_PART_BG, LV_STATE_DEFAULT, PadSizeB >= 0 ? PadSizeB : (strlen(ticktextX) == 0 ? 8 : pad + 2 * (LV_DPI / 10)));
		lv_obj_set_style_local_pad_top(obj, LV_CHART_PART_BG, LV_STATE_DEFAULT, PadSizeT >= 0 ? PadSizeT : (strlen(ticktextX) == 0 ? 8 : 2 * (LV_DPI / 10)));
		lv_chart_set_x_tick_texts(obj, ticktextX, IntervalTicksX + 1, LV_CHART_AXIS_DRAW_LAST_TICK);
		lv_chart_set_x_tick_length(obj, strlen(ticktextX) == 0 ? 0 : TickSize * 5, strlen(ticktextX) == 0 ? 0 : TickSize * 3);
	}

	if (ticktextY != NULL)
	{
		pad = 0;
		buf = strdup(ticktextY);
		for (p = strtok(buf, "\n"); p != NULL; p = strtok(NULL, "\n"))
		{
			_lv_txt_get_size(&size, p, label_dsc.font, label_dsc.letter_space, label_dsc.line_space, LV_COORD_MAX, LV_TXT_FLAG_CENTER);
			if (pad < size.x) pad = size.x;
		}
		free(buf);
		lv_obj_set_style_local_pad_left(obj, LV_CHART_PART_BG, LV_STATE_DEFAULT, PadSizeL >= 0 ? PadSizeL : (strlen(ticktextY) == 0 ? 8 : pad + 2 * (LV_DPI / 10)));
		lv_obj_set_style_local_pad_right(obj, LV_CHART_PART_BG, LV_STATE_DEFAULT, PadSizeR >= 0 ? PadSizeR : (strlen(ticktextY) == 0 ? 8 : 2 * (LV_DPI / 10)));
		lv_chart_set_y_tick_texts(obj, ticktextY, IntervalTicksY + 1, LV_CHART_AXIS_DRAW_LAST_TICK);
		lv_chart_set_y_tick_length(obj, strlen(ticktextY) == 0 ? 0 : TickSize * 5, strlen(ticktextY) == 0 ? 0 : TickSize * 3);
	}
	lv_obj_refresh_style(obj, LV_OBJ_PART_MAIN, LV_STYLE_PROP_ALL);
	return;
}

void Oscilloscope::insertSample(int val)
{
	sampleBuf[sBufPtr++] = val;
	if (sBufPtr >= SampleBufferSize)
	{
		sBufPtr = 0;
		sBufFull = true;
	}
}

void Oscilloscope::insertSample(int val, unsigned long long us)
{
	long i, cnt, div;
	if (!sBufFull && sBufPtr == 0)
	{
		rtTimer = us;
		insertSample(val);
	}
	else if ((cnt = ((us - rtTimer) / SampleingCycle)) > 0)
	{
		rtTimer += (cnt * SampleingCycle);
		div = val - sampleBuf[(sBufPtr > 0 ? sBufPtr : SampleBufferSize) - 1];

		for (i = (cnt > SampleBufferSize ? SampleBufferSize : cnt) - 1; i >= 0; i--)
			insertSample(val - (div * i / cnt));
	}
}

Oscilloscope osc;