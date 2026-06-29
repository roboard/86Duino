#include "lv_console.h"
#include "lv_freetype.h"


using namespace LVGL86;
#define LV_CONSOLE_BG_START_COLOR	0x808080
#define LV_CONSOLE_BG_COLOR			0x252525
#define LV_CONSOLE_TEXT_COLOR		0xF0F0F0

typedef struct Console::Impl
{
	char* buf;
	char* sp;
	char* ep;
	char* lnlp;

	uint32_t lineLength;
	uint32_t maxLineCnt;
	uint32_t bufSize;
	uint32_t bufLineCnt;

	bool autoRefreshMode;
	bool editedFlag;
	uint64_t refreshTimer;
	int FPS;

	void write(unsigned char c);
	void write(unsigned char* str, size_t str_size);
	void moveBufData();
	void newLineCheck(unsigned char);

} CS_Impl_t;

void Console::Impl::write(unsigned char c)
{
	if (ep + 1 >= buf + bufSize)
		moveBufData();
	
	newLineCheck(c);

	if ((*ep = c) == '\n')
	{
		char* p = ep;
		if (--p >= sp && ((*p & 0xF8) == 0xF0 || (*p & 0xF0) == 0xE0 || (*p & 0xE0) == 0xC0))
			memmove(p + 1, p, sizeof(unsigned char) * (ep - p));
		else if (--p >= sp && ((*p & 0xF8) == 0xF0 || (*p & 0xF0) == 0xE0))
			memmove(p + 1, p, sizeof(unsigned char) * (ep - p));
		else if (--p >= sp && ((*p & 0xF8) == 0xF0))
			memmove(p + 1, p, sizeof(unsigned char) * (ep - p));
		else
			p = ep;
		*p = '\n';
		lnlp = p + 1;
	}
	*++ep = '\0';
}

void Console::Impl::write(unsigned char* str, size_t str_size)
{
	unsigned char* src;
	size_t size;

	if (str_size < bufSize / 2)
	{
		size = str_size;
		src = str;
		if (ep + size + (size / (lineLength - 1)) >= buf + bufSize)
			moveBufData();
	}
	else
	{
		sp = buf;
		ep = buf;
		lnlp = buf;
		bufLineCnt = 0;
		size = bufSize / 2;
		src = str + str_size - size;
	}

	int n = (size + 7) / 8;

	switch (size % 8) {
	case 0: do {
		newLineCheck(*src); *ep++ = *src++;
	case 7:      newLineCheck(*src); *ep++ = *src++;
	case 6:      newLineCheck(*src); *ep++ = *src++;
	case 5:      newLineCheck(*src); *ep++ = *src++;
	case 4:      newLineCheck(*src); *ep++ = *src++;
	case 3:      newLineCheck(*src); *ep++ = *src++;
	case 2:      newLineCheck(*src); *ep++ = *src++;
	case 1:      newLineCheck(*src); *ep++ = *src++;
	} while (--n > 0);
	}
	*ep = '\0';
}

void Console::Impl::moveBufData()
{
	int cnt = ep - sp;
	int n = (cnt + 7) / 8;
	ep = buf;
	lnlp -= (sp - buf);

	switch (cnt % 8) {
	case 0: do {
		*ep++ = *sp++;
	case 7:      *ep++ = *sp++;
	case 6:      *ep++ = *sp++;
	case 5:      *ep++ = *sp++;
	case 4:      *ep++ = *sp++;
	case 3:      *ep++ = *sp++;
	case 2:      *ep++ = *sp++;
	case 1:      *ep++ = *sp++;
	} while (--n > 0);
	}

	sp = buf;
	*ep = '\0';
}

void Console::Impl::newLineCheck(unsigned char c)
{
	if (c == '\n')
	{
		if (++bufLineCnt >= maxLineCnt)
		{
			sp = strchr(sp, '\n') + 1;
			--bufLineCnt;
		}
		if (ep > sp && *(ep - 1) == '\r')  // remove carriage return '\r'
			--ep;
		lnlp = ep + 1;
	}
	else if (ep - lnlp + 1 >= lineLength)
		write('\n');
}

// Console class
Console::Console(bool _autoRefreshMode)
{
	obj = NULL;

	pimpl = (CS_Impl_t*)malloc(sizeof(CS_Impl_t));
	pimpl->buf = NULL;
	pimpl->autoRefreshMode = _autoRefreshMode;
	pimpl->FPS = 60;
}

Console::~Console()
{
	if (pimpl)
	{
		if (pimpl->buf)
			free(pimpl->buf);
		free(pimpl);
	}
	if (!obj) return;

	lv_textarea_set_text(obj, ""); // clear all text
	lv_obj_del(obj);
	obj = NULL;
}

lv_obj_t* Console::begin(lv_obj_t* parent, int _x, int _y, int _width, int _height, int8_t _font, uint16_t maxLineLen, uint16_t maxLineCnt, double textColor, double bgColor, bool _enable)
{
	if (!parent)
		obj = lv_textarea_create(lv_scr_act(), NULL);
	else
		obj = lv_textarea_create(parent, NULL);

	lv_textarea_ext_t* ext = (lv_textarea_ext_t*)lv_obj_get_ext_attr(obj);

	lv_label_set_long_mode(ext->label, LV_LABEL_LONG_EXPAND);
	lv_page_set_scrollable_fit2(obj, LV_FIT_MAX, LV_FIT_TIGHT);



	pimpl->lineLength = maxLineLen + 1;
	pimpl->maxLineCnt = maxLineCnt;
	pimpl->bufSize = pimpl->lineLength * pimpl->maxLineCnt * 2;
	pimpl->buf = (char*)malloc(pimpl->bufSize * sizeof(char));
	memset(pimpl->buf, '\0', pimpl->bufSize * sizeof(char));
	pimpl->sp = pimpl->buf;
	pimpl->ep = pimpl->buf;
	pimpl->lnlp = pimpl->buf;
	pimpl->bufLineCnt = 0;

	lv_textarea_set_scrollbar_mode(obj, LV_SCRLBAR_MODE_OFF);

	if (textColor == -1)
		resetTextColor();
	else
		setTextColor(textColor);

	if (bgColor == -1)
		resetBgColor();
	else
		setBgColor(bgColor);

	lv_obj_set_style_local_bg_grad_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(LV_CONSOLE_BG_START_COLOR));

	lv_textarea_set_text(obj, ""); // clear all text
	lv_textarea_set_cursor_hidden(obj, true);

	setXY(_x, _y);
	setSize(_width, _height);
	setHMITextFont(_font);
	setEnable(_enable);
	return obj;
}

void Console::refresh()
{
#ifndef WIN32
	if (obj == NULL) return;
	pimpl->refreshTimer = millis();
	if (!pimpl->editedFlag) return;
	lv_textarea_set_text(obj, (const char*)pimpl->sp);
	pimpl->editedFlag = false;
#endif
}

bool Console::refreshTimeUp()
{
#ifndef WIN32
	return (millis() - pimpl->refreshTimer) * pimpl->FPS >= 1000;
#else
	return false;
#endif
}

void Console::enableGradient()
{
	if (obj == NULL) return;
	lv_obj_set_style_local_bg_grad_dir(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
	lv_obj_set_style_local_bg_main_stop(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
	lv_obj_set_style_local_bg_grad_stop(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 250);
}

void Console::disableGradient()
{
	if (obj == NULL) return;
	lv_obj_set_style_local_bg_grad_dir(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
}

void Console::setBgStartColor(uint32_t c)
{
	if (obj == NULL) return;
	lv_obj_set_style_local_bg_grad_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(c));
}

void Console::setBgColor(uint32_t c)
{
	if (obj == NULL) return;
	lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(c));
}

void Console::setTextColor(uint32_t c)
{
	if (obj == NULL) return;
	lv_obj_set_style_local_text_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(c));
}

bool Console::isDefaultBgStartColor()
{
	if (obj == NULL) return false;
	return lv_obj_get_style_bg_grad_color(obj, LV_OBJ_PART_MAIN).full == lv_color_hex(LV_CONSOLE_BG_START_COLOR).full;
}

bool Console::isDefaultBgColor()
{
	if (obj == NULL) return false;
	return lv_obj_get_style_bg_color(obj, LV_OBJ_PART_MAIN).full == lv_color_hex(LV_CONSOLE_BG_COLOR).full;
}

bool Console::isDefaultTextColor()
{
	if (obj == NULL) return false;
	return lv_obj_get_style_text_color(obj, LV_OBJ_PART_MAIN).full == lv_color_hex(LV_CONSOLE_TEXT_COLOR).full;
}

void Console::resetBgStartColor()
{
	if (obj == NULL) return;
	lv_obj_set_style_local_bg_grad_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(LV_CONSOLE_BG_START_COLOR));
}

void Console::resetBgColor()
{
	if (obj == NULL) return;
	lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(LV_CONSOLE_BG_COLOR));
}

void Console::resetTextColor()
{
	if (obj == NULL) return;
	lv_obj_set_style_local_text_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(LV_CONSOLE_TEXT_COLOR));
}

uint32_t Console::getBgStartColor()
{
	if (obj == NULL) return 0;
	return lv_color_to32(lv_obj_get_style_bg_grad_color(obj, LV_OBJ_PART_MAIN));
}

uint32_t Console::getBgColor()
{
	if (obj == NULL) return 0;
	return lv_color_to32(lv_obj_get_style_bg_color(obj, LV_OBJ_PART_MAIN));
}

uint32_t Console::getTextColor()
{
	if (obj == NULL) return 0;
	return lv_color_to32(lv_obj_get_style_text_color(obj, LV_OBJ_PART_MAIN));
}

void Console::setXY(int x, int y)
{
	if (obj == NULL) return;
	X = x;
	Y = y;
	lv_obj_set_pos(obj, X, Y);
}

void Console::setSize(int w, int h)
{
	if (obj == NULL) return;
	Width = w;
	Height = h;
	lv_obj_set_width(obj, Width);
	lv_obj_set_height(obj, Height);
}

void Console::setHMIColor(int colorId)
{
	if (obj == NULL) return;
	set_color_id(obj, colorId);
}

void Console::setHMITextFont(int fontId)
{
	if (obj == NULL) return;
	set_font_id(obj, fontId);
}

void Console::setEnable(bool enable)
{
	if (obj == NULL) return;
	Enable = enable;
	void (*enable_cb)(lv_obj_t*, lv_state_t) = Enable ? lv_obj_clear_state : lv_obj_add_state;
	lv_obj_t* child = NULL;
	enable_cb(obj, LV_STATE_DISABLED);
	while (child = lv_obj_get_child(obj, child))
		enable_cb(child, LV_STATE_DISABLED);
	child = NULL;
	while (child = lv_list_get_next_btn(obj, child))
		enable_cb(child, LV_STATE_DISABLED);
}

void Console::scrollbarON() {
	if (!obj) return;
	lv_textarea_set_scrollbar_mode(obj, LV_SCRLBAR_MODE_AUTO);
}

void Console::scrollbarOFF() {
	if (!obj) return;
	lv_textarea_set_scrollbar_mode(obj, LV_SCRLBAR_MODE_OFF);
}

size_t Console::write(unsigned char c) {
	if (!obj || c == '\0') return 0;

	pimpl->write(c);
	if (pimpl->autoRefreshMode)
		lv_textarea_set_text(obj, (const char*)pimpl->sp);
	else
		pimpl->editedFlag = true;
	return 1;
}

// If all text length > bufSize, it will be match as bufSize
size_t Console::write(const unsigned char* str, size_t str_size) {
	if (!obj || str == NULL) return 0;

	pimpl->write((unsigned char*)str, str_size);
	if (pimpl->autoRefreshMode)
		lv_textarea_set_text(obj, (const char*)pimpl->sp);
	else
		pimpl->editedFlag = true;
	return str_size;
}

void Console::fflush(void) {
}

int Console::available(void) {
	return -1;
}

int Console::peek(void) {
	return -1;
}

int Console::read(void) {
	return -1;
}

#ifndef WIN32
void Console::printf(const char* fmt, ...)
{
	if (fmt == NULL) return;
	va_list args;
	va_start(args, fmt);
	char* s = _lv_txt_set_text_vfmt(fmt, args);
	print(s);
	if (s != NULL)
		lv_mem_free(s);
	va_end(args);
}

size_t Console::printTo(Print& p) const
{
	return p.print(pimpl->sp);
}
#endif

Console console;