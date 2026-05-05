#include "lv_textlist.h"

using namespace LVGL86;

#define TL_MAX_ROW	10000
#define TL_SPLICING_OFFSET -2
#ifndef WIN32
#define TL_CELL_BUFFER_SIZE 256
#else
#define TL_CELL_BUFFER_SIZE 16
#endif

void removeStyleProp(lv_obj_t* obj, uint8_t part, lv_style_property_t prop);
static bool lv_style_prop_exist(const lv_style_t* style, lv_style_property_t prop);

typedef struct TextList::Impl
{
	TextList* tl;
	lv_obj_t* page;
	lv_obj_t* cont;
	lv_obj_t** table;
	bool lnVisiable;
	uint16_t lnSpacing;
	uint32_t lnHead, lnTail;
	uint32_t lastHighlight;
	uint32_t vrStart, vrEnd;
	void tableInit(lv_obj_t*, bool);
	void setLineRange(uint32_t, uint32_t);
	void lineNumberMaker(uint8_t);

	void setFontID(int8_t);
	void refreshContSize();
	void refreshHeight();
	void refreshWidth();

	uint32_t getLineCount() { return lnTail - lnHead + 1; }
	uint32_t getTableCount() { return getLineCount() / TL_MAX_ROW + 1; }
	bool isAvailable(uint32_t line) { return line >= lnHead && line <= lnTail; }
	void idSearch(uint32_t line, uint32_t& tableID, uint32_t& cellID) { tableID = (line - lnHead) / TL_MAX_ROW; cellID = line - (lnHead + TL_MAX_ROW * tableID); }
} TL_Impl_t;

void TextList::Impl::tableInit(lv_obj_t* parent, bool _lnVisiable)
{
	page = parent;
	lnVisiable = _lnVisiable;
	cont = lv_obj_create(page, NULL);

	lv_obj_set_style_local_border_width(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_opa(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_add_state(cont, LV_STATE_DISABLED);
	lv_obj_set_width(cont, lv_page_get_width_fit(page));
}

void TextList::Impl::setLineRange(uint32_t head, uint32_t tail)
{
	uint16_t oldTableCnt = table ? getTableCount() : 0;
	lnHead = vrStart = head;
	lnTail = vrEnd = tail;
	uint16_t newTableCnt = getTableCount();
	uint16_t i;

	for (i = newTableCnt; i < oldTableCnt; i++)
		lv_obj_del(table[i]);

	if (newTableCnt > oldTableCnt)
		table = (lv_obj_t**)lv_mem_realloc(table, newTableCnt * sizeof(lv_obj_t*));

	for (i = 0; i < newTableCnt; i++)
	{
		if (i >= oldTableCnt)
		{
			if (i)
			{
				table[i] = lv_qtable_create(cont, table[0]);
				lv_obj_align(table[i], table[i - 1], LV_ALIGN_OUT_BOTTOM_LEFT, 0, TL_SPLICING_OFFSET);
			}
			else
			{
				table[i] = lv_qtable_create(cont, NULL);

				lv_obj_set_style_local_border_width(table[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
				lv_obj_set_style_local_pad_all(table[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
				lv_obj_set_style_local_pad_inner(table[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
				lv_obj_set_style_local_bg_opa(table[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

				for (int j = LV_QTABLE_PART_CELL1; j <= LV_QTABLE_PART_CELL16; j++)
				{
					lv_obj_set_style_local_border_side(table[i], j, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM + LV_BORDER_SIDE_TOP);
					lv_obj_set_style_local_pad_bottom(table[i], j, LV_STATE_DEFAULT, lnSpacing);
					lv_obj_set_style_local_pad_top(table[i], j, LV_STATE_DEFAULT, lnSpacing);
					lv_obj_set_style_local_pad_left(table[i], j, LV_STATE_DEFAULT, 5);
					lv_obj_set_style_local_pad_right(table[i], j, LV_STATE_DEFAULT, 5);
				}
			}
			lv_obj_add_state(table[i], LV_STATE_DISABLED);
		}

		lineNumberMaker(i);
	}
	refreshWidth();
	refreshHeight();
}

void TextList::Impl::lineNumberMaker(uint8_t id)
{
	lv_qtable_ext_t* ext = (lv_qtable_ext_t*)lv_obj_get_ext_attr(table[id]);

	uint32_t head, tail;
	head = lnHead + TL_MAX_ROW * id;
	tail = lnHead + TL_MAX_ROW * (id + 1) - 1;
	tail = tail > lnTail ? lnTail : tail;

	lv_qtable_set_col_cnt(table[id], 2);
	lv_qtable_set_row_cnt(table[id], (tail - head + 1));

	uint8_t bufgap = floor(log10(lnTail)) + 3;
	bool flag0 = lv_qtable_set_col_buf(table[id], 0, ext->row_cnt * bufgap);
	bool flag1 = lv_qtable_set_col_buf(table[id], 1, ext->row_cnt * TL_CELL_BUFFER_SIZE);
	if (!flag0 || !flag1) return;
	for (uint32_t row = 0; row < ext->row_cnt; row += 2)
	{
		lv_qtable_set_cell_value_fmt(table[id], row, 0, "%lu", row + head);
		lv_qtable_set_cell_align(table[id], row, 0, LV_LABEL_ALIGN_RIGHT);
		lv_qtable_set_cell_type(table[id], row, 0, LV_QTABLE_PART_CELL1);
#ifdef WIN32
		if (row < 99)
		{
			lv_qtable_set_cell_value_fmt(table[id], row, 1, "line%lu text", row + head);
			lv_qtable_set_cell_align(table[id], row, 1, LV_LABEL_ALIGN_LEFT);
		}
#endif
		lv_qtable_set_cell_type(table[id], row, 1, LV_QTABLE_PART_CELL3);
	}

	for (uint32_t row = 1; row < ext->row_cnt; row += 2)
	{
		lv_qtable_set_cell_value_fmt(table[id], row, 0, "%lu", row + head);
		lv_qtable_set_cell_align(table[id], row, 0, LV_LABEL_ALIGN_RIGHT);
		lv_qtable_set_cell_type(table[id], row, 0, LV_QTABLE_PART_CELL2);

#ifdef WIN32
		if (row < 99)
		{
			lv_qtable_set_cell_value_fmt(table[id], row, 1, "line%lu text", row + head);
			lv_qtable_set_cell_align(table[id], row, 1, LV_LABEL_ALIGN_LEFT);
		}
#endif
		lv_qtable_set_cell_type(table[id], row, 1, LV_QTABLE_PART_CELL4);
	}
}

void TextList::Impl::setFontID(int8_t FontID)
{
	if (!table) return;
	uint16_t i, tableCnt = getTableCount();
	for (i = 0; i < tableCnt; i++)
		set_font_id(table[i], FontID);
	refreshWidth();
}

void TextList::Impl::refreshContSize()
{
	if (!table) return;
	uint16_t i, tableCnt = getTableCount();
	lv_obj_set_width(cont, lv_page_get_width_fit(page));
	for (i = 0; i < tableCnt; i++)
		if (!lnVisiable)
		{
			lv_qtable_set_col_width(table[i], 0, 0);
			lv_qtable_set_col_width(table[i], 1, lv_obj_get_width(cont) - 1);
		}
		else
		{
			int col_w = lv_qtable_get_col_width(table[i], 0);
			lv_qtable_set_col_width(table[i], 0, col_w);
			lv_qtable_set_col_width(table[i], 1, lv_obj_get_width(cont) - col_w - 1);
		}
}

void TextList::Impl::refreshHeight()
{
	if (!table) return;
	uint32_t stid, etid, srid, erid;
	idSearch(vrStart, stid, srid);
	idSearch(vrEnd, etid, erid);

	uint16_t tableCnt = getTableCount();
	uint32_t i;
	int64_t dist;
	lv_qtable_ext_t* ext;

	dist = 0;
	ext = (lv_qtable_ext_t*)lv_obj_get_ext_attr(table[stid]);
	for (i = 0; i < stid; ++i)
		dist -= lv_obj_get_height(table[i]) + TL_SPLICING_OFFSET;
	for (i = 0; i < srid; i++)
		dist -= ext->row_h[i];
	lv_obj_set_y(table[0], dist);

	for (i = 1; i < tableCnt; ++i)
		lv_obj_align(table[i], table[i - 1], LV_ALIGN_OUT_BOTTOM_LEFT, 0, TL_SPLICING_OFFSET);

	uint32_t contHeight = 0;
	if (stid != etid)
	{
		for (i = stid + 1; i < etid; ++i)
			contHeight += lv_obj_get_height(table[i]) + TL_SPLICING_OFFSET;

		ext = (lv_qtable_ext_t*)lv_obj_get_ext_attr(table[stid]);
		if (srid < ext->row_cnt / 2)
		{
			contHeight += lv_obj_get_height(table[stid]) + TL_SPLICING_OFFSET;
			for (i = 0; i < srid; i++)
				contHeight -= ext->row_h[i];
		}
		else
			for (i = srid; i < ext->row_cnt; i++)
				contHeight += ext->row_h[i];

		ext = (lv_qtable_ext_t*)lv_obj_get_ext_attr(table[etid]);
		if (erid > ext->row_cnt / 2)
		{
			contHeight += lv_obj_get_height(table[etid]) + TL_SPLICING_OFFSET;
			for (i = erid + 1; i < ext->row_cnt; i++)
				contHeight -= ext->row_h[i];
		}
		else
			for (i = 0; i <= erid; i++)
				contHeight += ext->row_h[i];
	}
	else if (erid - srid + 1 > ext->row_cnt / 2)
	{
		contHeight += lv_obj_get_height(table[stid]) + TL_SPLICING_OFFSET;
		for (i = 0; i < srid; i++)
			contHeight -= ext->row_h[i];
		for (i = erid + 1; i < ext->row_cnt; i++)
			contHeight -= ext->row_h[i];
	}
	else
	{
		for (i = srid; i <= erid; i++)
			contHeight += ext->row_h[i];
	}
	lv_obj_set_height(cont, contHeight);

	lv_obj_set_height(lv_page_get_scrollable(page), contHeight);
}

void TextList::Impl::refreshWidth()
{
	if (!table) return;
	uint16_t i, tableCnt = getTableCount();
	if (!lnVisiable)
	{
		for (i = 0; i < tableCnt; i++)
		{
			lv_qtable_set_col_width(table[i], 0, 0);
			lv_qtable_set_col_width(table[i], 1, lv_obj_get_width(cont) - 1);
		}
		return;
	}
	char txt[8];
	sprintf(txt, "%lu", lnTail);
	lv_point_t size = { 0, 0 };
	lv_style_int_t letter_space = lv_obj_get_style_text_letter_space(table[0], LV_QTABLE_PART_CELL1);
	lv_style_int_t line_space = lv_obj_get_style_text_line_space(table[0], LV_QTABLE_PART_CELL1);
	lv_style_int_t cell_left = lv_obj_get_style_pad_left(table[0], LV_QTABLE_PART_CELL1);
	lv_style_int_t cell_right = lv_obj_get_style_pad_right(table[0], LV_QTABLE_PART_CELL1);
	const lv_font_t* font = lv_obj_get_style_text_font(table[0], LV_QTABLE_PART_CELL1);
	_lv_txt_get_size(&size, txt, font, letter_space, line_space, LV_COORD_MAX, LV_TXT_FLAG_CENTER);

	int col_w = size.x + cell_left + cell_right;
	if (lv_qtable_get_col_width(table[0], 0) == col_w) return;
	for (i = 0; i < tableCnt; i++)
	{
		lv_qtable_set_col_width(table[i], 0, col_w);
		lv_qtable_set_col_width(table[i], 1, lv_obj_get_width(cont) - col_w - 1);
	}
}

TextList::TextList()
{
	X = 0, Y = 0, Width = 260, Height = 300, Enable = true;
	pimpl = (TL_Impl_t*)malloc(sizeof(TL_Impl_t));
	pimpl->tl = this;
	pimpl->cont = NULL;
	pimpl->table = NULL;
	pimpl->lnVisiable = false;
	pimpl->lnSpacing = 0;

	pimpl->lnHead = 1, pimpl->lnTail = 99;
	pimpl->lastHighlight = 0;
	pimpl->vrStart = 1, pimpl->vrEnd = 99;
}

TextList::~TextList()
{
	if (pimpl)
	{
		lv_mem_free(pimpl->table);
		free(pimpl);
	}
	if (obj) lv_obj_del(obj);
	obj = NULL;
}

lv_obj_t* TextList::begin(lv_obj_t* parent, int _x, int _y, int _width, int _height, bool _enable, bool _lineNumEnable, uint32_t _lineNumHead, uint32_t _lineNumTail, uint16_t _lineSpacingGap)
{
	if (parent == NULL)
		obj = lv_page_create(lv_scr_act(), NULL);
	else
		obj = lv_page_create(parent, NULL);

	lv_obj_set_style_local_border_width(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_pad_all(obj, LV_PAGE_PART_BG, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_pad_all(obj, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 0);

	setXY(_x, _y);
	setSize(_width, _height);
	pimpl->tableInit(obj, _lineNumEnable);
	setEnable(_enable);
	setLineRange(_lineNumHead, _lineNumTail);
	setLineSpacing(_lineSpacingGap);
	return obj;
}

void TextList::print(uint32_t lineNumber, char* text)
{
	if (!pimpl->isAvailable(lineNumber)) return;

	uint32_t tid, rid;
	pimpl->idSearch(lineNumber, tid, rid);
	lv_qtable_set_cell_value(pimpl->table[tid], rid, 1, text);
	pimpl->refreshHeight();
}

void TextList::clear()
{
	lv_qtable_ext_t* ext;
	uint32_t i, tableCnt = pimpl->getTableCount(), row;
	for (i = 0; i < tableCnt; i++)
	{
		ext = (lv_qtable_ext_t*)lv_obj_get_ext_attr(pimpl->table[i]);
		for (row = 0; row < ext->row_cnt; row++)
			lv_qtable_set_cell_value(pimpl->table[i], row, 1, "");
	}
	pimpl->refreshHeight();
}

void TextList::focus(uint32_t lineNumber, bool alignCenter)
{
	if (lineNumber < pimpl->vrStart || lineNumber > pimpl->vrEnd) return;
	uint32_t i, stid, etid, srid, erid;
	int32_t dist = 0;
	lv_qtable_ext_t* ext;

	pimpl->idSearch(pimpl->vrStart, stid, srid);
	pimpl->idSearch(lineNumber, etid, erid);

	if (stid == etid)
	{
		ext = (lv_qtable_ext_t*)lv_obj_get_ext_attr(pimpl->table[stid]);
		for (i = srid; i < erid; i++)
			dist -= ext->row_h[i];
	}
	else
	{
		ext = (lv_qtable_ext_t*)lv_obj_get_ext_attr(pimpl->table[stid]);
		for (i = srid; i < ext->row_cnt; i++)
			dist -= ext->row_h[i];
		for (i = stid + 1; i < etid; ++i)
			dist -= lv_obj_get_height(pimpl->table[i]) + TL_SPLICING_OFFSET;
		ext = (lv_qtable_ext_t*)lv_obj_get_ext_attr(pimpl->table[etid]);
		for (i = 0; i < erid; i++)
			dist -= ext->row_h[i];
	}
	if (alignCenter)
		dist += (lv_obj_get_height(obj) - ext->row_h[i]) / 2 + 1 - lv_obj_get_style_border_width(obj, LV_CHART_PART_BG);

	lv_obj_set_y(lv_page_get_scrollable(obj), dist);
}

void TextList::highlight(uint32_t lineNumber)
{
	uint32_t tid, rid;
	if (pimpl->isAvailable(pimpl->lastHighlight))
	{
		pimpl->idSearch(pimpl->lastHighlight, tid, rid);
		lv_qtable_set_cell_type(pimpl->table[tid], rid, 0, rid % 2 == 0 ? LV_QTABLE_PART_CELL1 : LV_QTABLE_PART_CELL2);
		lv_qtable_set_cell_type(pimpl->table[tid], rid, 1, rid % 2 == 0 ? LV_QTABLE_PART_CELL3 : LV_QTABLE_PART_CELL4);
	}
	if (pimpl->isAvailable(lineNumber))
	{
		pimpl->idSearch(lineNumber, tid, rid);
		lv_qtable_set_cell_type(pimpl->table[tid], rid, 0, LV_QTABLE_PART_CELL5);
		lv_qtable_set_cell_type(pimpl->table[tid], rid, 1, LV_QTABLE_PART_CELL6);
		pimpl->lastHighlight = lineNumber;
	}
}

void TextList::setViewRange(uint32_t startLine, uint32_t endLine)
{
	if (!pimpl->isAvailable(startLine)) return;
	if (!pimpl->isAvailable(endLine)) return;
	if (startLine > endLine) return;
	pimpl->vrStart = startLine;
	pimpl->vrEnd = endLine;
	pimpl->refreshHeight();
}

void TextList::openFile(const char* path, int skipline)
{
	char buf[1024], * ptr, c = 0;
	uint32_t tid, rid;
	uint32_t i, eofLine = 0;
	strcpy(buf, path);
	FILE* fp = fopen(buf, "r");
	if (!fp)
	{
#ifndef WIN32
		SD.getDiskPath(SD.getBank(), buf);
#else
		strcpy(buf, "C:\\");
#endif
		strcat(buf, path);
		fp = fopen(buf, "r");
		if (!fp)
			return;
	}

	for (i = 0; i < skipline; c = fgetc(fp))
	{
		if (c == '\n')
			i++;
		else if (c == EOF)
			return;
	}

	for (i = pimpl->lnHead; i <= pimpl->lnTail; i++)
	{
		pimpl->idSearch(i, tid, rid);
		ptr = buf;
		if (c != EOF)
			eofLine = i;
		while ((c = fgetc(fp)) != EOF)
		{
			if (c == '\r')
				continue;
			if (c == '\n')
				break;

			*ptr++ = c;
			if (ptr >= buf + sizeof(buf) - 10)	// keep for UTF-8
			{
				*ptr++ = '.';
				*ptr++ = '.';
				*ptr++ = '.';
				while ((c = fgetc(fp)) != EOF && c != '\n');
				break;
			}
		}

		*ptr = '\0';
		if (buf != ptr)
			lv_qtable_set_cell_value(pimpl->table[tid], rid, 1, buf);
		else if (strlen(lv_qtable_get_cell_value(pimpl->table[tid], rid, 1)))
			lv_qtable_set_cell_value(pimpl->table[tid], rid, 1, "");
	}
	fclose(fp);
	lv_obj_set_y(lv_page_get_scrollable(obj), 0);
	setViewRange(pimpl->lnHead, eofLine);
}

void TextList::openFile(int bank, const char* path, int skipline)
{
#ifndef WIN32
	int size = strlen(SD.getDiskPath(bank)) + strlen(path) + 1;
	char* fullpath = (char*)malloc(sizeof(char) * size);
	memset(fullpath, '\0', sizeof(char) * size);
	SD.getDiskPath(bank, fullpath);
#else
	int size = strlen(path) + 5;
	char* fullpath = (char*)malloc(sizeof(char) * size);
	memset(fullpath, '\0', sizeof(char) * size);
	strcpy(fullpath, "C:\\");
#endif
	strcat(fullpath, path);
	openFile(fullpath, skipline);
	free(fullpath);
}

const char* TextList::getLine(uint32_t lineNumber, char* buf)
{
	if (!pimpl->isAvailable(lineNumber)) return "";

	uint32_t tid, rid;
	pimpl->idSearch(lineNumber, tid, rid);
	const char* line = lv_qtable_get_cell_value(pimpl->table[tid], rid, 1);
	if (buf)
		strcpy(buf, line);
	return line;
}

uint32_t TextList::getViewRangeStart()
{
	return pimpl->vrStart;
}

uint32_t TextList::getViewRangeEnd()
{
	return pimpl->vrEnd;
}

void TextList::setXY(int x, int y)
{
	if (obj == NULL) return;
	X = x;
	Y = y;
	lv_obj_set_pos(obj, X, Y);
}

void TextList::setSize(int w, int h)
{
	if (obj == NULL) return;
	Width = w;
	Height = h;
	lv_obj_set_width(obj, w);
	lv_obj_set_height(obj, h);
	pimpl->refreshContSize();
}

void TextList::setEnable(bool enable)
{
	if (obj == NULL) return;
	Enable = enable;
	void (*enable_cb)(lv_obj_t*, lv_state_t) = Enable ? lv_obj_clear_state : lv_obj_add_state;
	enable_cb(obj, LV_STATE_DISABLED);
}

void TextList::setLineRange(uint32_t head, uint32_t tail)
{
	pimpl->setLineRange(head, tail);
}

void TextList::setLineNumVisiable(bool visiable)
{
	if (obj == NULL) return;
	if (pimpl->lnVisiable == visiable) return;
	pimpl->lnVisiable = visiable;
	pimpl->refreshWidth();
}

void TextList::setFocusLineColor(double color)
{
	if (obj == NULL) return;
	uint16_t i, tableCnt = pimpl->getTableCount();
	for (i = 0; i < tableCnt; i++)
	{
		if (color < 0)
		{
			lv_obj_set_style_local_bg_color(pimpl->table[i], LV_QTABLE_PART_CELL5, LV_STATE_DEFAULT, lv_color_hex((uint32_t)get_theme_color(0, 0)));
			lv_obj_set_style_local_bg_color(pimpl->table[i], LV_QTABLE_PART_CELL6, LV_STATE_DEFAULT, lv_color_hex((uint32_t)get_theme_color(0, 0)));
		}
		else
		{
			lv_obj_set_style_local_bg_color(pimpl->table[i], LV_QTABLE_PART_CELL5, LV_STATE_DEFAULT, lv_color_hex((uint32_t)color));
			lv_obj_set_style_local_bg_color(pimpl->table[i], LV_QTABLE_PART_CELL6, LV_STATE_DEFAULT, lv_color_hex((uint32_t)color));
		}
	}
}

void TextList::setOddLineColor(double color)
{
	if (obj == NULL) return;
	uint16_t i, tableCnt = pimpl->getTableCount();
	for (i = 0; i < tableCnt; i++)
	{
		if (color < 0)
		{
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL1, LV_STYLE_BG_COLOR);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL3, LV_STYLE_BG_COLOR);
		}
		else
		{
			lv_obj_set_style_local_bg_color(pimpl->table[i], LV_QTABLE_PART_CELL1, LV_STATE_DEFAULT, lv_color_hex((uint32_t)color));
			lv_obj_set_style_local_bg_color(pimpl->table[i], LV_QTABLE_PART_CELL3, LV_STATE_DEFAULT, lv_color_hex((uint32_t)color));
		}
	}
}

void TextList::setEvenLineColor(double color)
{
	if (obj == NULL) return;
	uint16_t i, tableCnt = pimpl->getTableCount();
	for (i = 0; i < tableCnt; i++)
	{
		if (color < 0)
		{
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL2, LV_STYLE_BG_COLOR);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL4, LV_STYLE_BG_COLOR);
		}
		else
		{
			lv_obj_set_style_local_bg_color(pimpl->table[i], LV_QTABLE_PART_CELL2, LV_STATE_DEFAULT, lv_color_hex((uint32_t)color));
			lv_obj_set_style_local_bg_color(pimpl->table[i], LV_QTABLE_PART_CELL4, LV_STATE_DEFAULT, lv_color_hex((uint32_t)color));
		}
	}
}

void TextList::setNumColor(double color)
{
	if (obj == NULL) return;
	uint16_t i, tableCnt = pimpl->getTableCount();
	for (i = 0; i < tableCnt; i++)
	{
		if (color < 0)
		{
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL1, LV_STYLE_TEXT_COLOR);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL2, LV_STYLE_TEXT_COLOR);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL5, LV_STYLE_TEXT_COLOR);
		}
		else
		{
			lv_obj_set_style_local_text_color(pimpl->table[i], LV_QTABLE_PART_CELL1, LV_STATE_DEFAULT, lv_color_hex((uint32_t)color));
			lv_obj_set_style_local_text_color(pimpl->table[i], LV_QTABLE_PART_CELL2, LV_STATE_DEFAULT, lv_color_hex((uint32_t)color));
			lv_obj_set_style_local_text_color(pimpl->table[i], LV_QTABLE_PART_CELL5, LV_STATE_DEFAULT, lv_color_hex((uint32_t)color));
		}
	}
}

void TextList::setTextColor(double color)
{
	if (obj == NULL) return;
	uint16_t i, tableCnt = pimpl->getTableCount();
	for (i = 0; i < tableCnt; i++)
	{
		if (color < 0)
		{
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL3, LV_STYLE_TEXT_COLOR);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL4, LV_STYLE_TEXT_COLOR);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL6, LV_STYLE_TEXT_COLOR);
		}
		else
		{
			lv_obj_set_style_local_text_color(pimpl->table[i], LV_QTABLE_PART_CELL3, LV_STATE_DEFAULT, lv_color_hex((uint32_t)color));
			lv_obj_set_style_local_text_color(pimpl->table[i], LV_QTABLE_PART_CELL4, LV_STATE_DEFAULT, lv_color_hex((uint32_t)color));
			lv_obj_set_style_local_text_color(pimpl->table[i], LV_QTABLE_PART_CELL6, LV_STATE_DEFAULT, lv_color_hex((uint32_t)color));
		}
	}
}

void TextList::setNumFont(lv_font_t* font)
{
	if (obj == NULL) return;
	uint16_t i, tableCnt = pimpl->getTableCount();
	for (i = 0; i < tableCnt; i++)
	{
		if (font == NULL)
		{
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL1, LV_STYLE_TEXT_FONT);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL2, LV_STYLE_TEXT_FONT);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL5, LV_STYLE_TEXT_FONT);
		}
		else
		{
			lv_obj_set_style_local_text_font(pimpl->table[i], LV_QTABLE_PART_CELL1, LV_STATE_DEFAULT, font);
			lv_obj_set_style_local_text_font(pimpl->table[i], LV_QTABLE_PART_CELL2, LV_STATE_DEFAULT, font);
			lv_obj_set_style_local_text_font(pimpl->table[i], LV_QTABLE_PART_CELL5, LV_STATE_DEFAULT, font);
		}
	}
	pimpl->refreshWidth();
}

void TextList::setTextFont(lv_font_t* font)
{
	if (obj == NULL) return;
	uint16_t i, tableCnt = pimpl->getTableCount();
	for (i = 0; i < tableCnt; i++)
	{
		if (font == NULL)
		{
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL3, LV_STYLE_TEXT_FONT);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL4, LV_STYLE_TEXT_FONT);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL6, LV_STYLE_TEXT_FONT);
		}
		else
		{
			lv_obj_set_style_local_text_font(pimpl->table[i], LV_QTABLE_PART_CELL3, LV_STATE_DEFAULT, font);
			lv_obj_set_style_local_text_font(pimpl->table[i], LV_QTABLE_PART_CELL4, LV_STATE_DEFAULT, font);
			lv_obj_set_style_local_text_font(pimpl->table[i], LV_QTABLE_PART_CELL6, LV_STATE_DEFAULT, font);
		}
	}
	pimpl->refreshWidth();
}

void TextList::setLineSpacing(uint16_t val)
{
	if (obj == NULL) return;
	if (pimpl->lnSpacing == val) return;
	uint16_t tableCnt = pimpl->getTableCount();
	pimpl->lnSpacing = val;
	for (uint16_t i = 0; i < tableCnt; i++)
		for (int j = LV_QTABLE_PART_CELL1; j <= LV_QTABLE_PART_CELL16; j++)
		{
			lv_obj_set_style_local_pad_bottom(pimpl->table[i], j, LV_STATE_DEFAULT, pimpl->lnSpacing);
			lv_obj_set_style_local_pad_top(pimpl->table[i], j, LV_STATE_DEFAULT, pimpl->lnSpacing);
		}
	pimpl->refreshHeight();
}

void TextList::setTextIndent(int val)
{
	if (obj == NULL) return;
	uint16_t tableCnt = pimpl->getTableCount();
	for (uint16_t i = 0; i < tableCnt; i++)
	{
		if (val < 0)
		{
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL1, LV_STYLE_PAD_RIGHT);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL2, LV_STYLE_PAD_RIGHT);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL5, LV_STYLE_PAD_RIGHT);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL3, LV_STYLE_PAD_LEFT);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL4, LV_STYLE_PAD_LEFT);
			removeStyleProp(pimpl->table[i], LV_QTABLE_PART_CELL6, LV_STYLE_PAD_LEFT);
		}
		else
		{
			lv_obj_set_style_local_pad_right(pimpl->table[i], LV_QTABLE_PART_CELL1, LV_STATE_DEFAULT, val);
			lv_obj_set_style_local_pad_right(pimpl->table[i], LV_QTABLE_PART_CELL2, LV_STATE_DEFAULT, val);
			lv_obj_set_style_local_pad_right(pimpl->table[i], LV_QTABLE_PART_CELL5, LV_STATE_DEFAULT, val);
			lv_obj_set_style_local_pad_left(pimpl->table[i], LV_QTABLE_PART_CELL3, LV_STATE_DEFAULT, val);
			lv_obj_set_style_local_pad_left(pimpl->table[i], LV_QTABLE_PART_CELL4, LV_STATE_DEFAULT, val);
			lv_obj_set_style_local_pad_left(pimpl->table[i], LV_QTABLE_PART_CELL6, LV_STATE_DEFAULT, val);
		}
	}
	pimpl->refreshWidth();
}

void TextList::setBorderSide(lv_border_side_t val)
{
	if (obj == NULL) return;
	uint16_t tableCnt = pimpl->getTableCount();
	for (uint16_t i = 0; i < tableCnt; i++)
		for (int j = LV_QTABLE_PART_CELL1; j <= LV_QTABLE_PART_CELL16; j++)
			lv_obj_set_style_local_border_side(pimpl->table[i], j, LV_STATE_DEFAULT, val);
}
bool TextList::getLineNumVisiable()
{
	return pimpl->lnVisiable;
}

double TextList::getOddLineColor()
{
	if (obj == NULL) return -1;
	lv_style_list_t* list = lv_obj_get_style_list(pimpl->table[0], LV_QTABLE_PART_CELL1);
	lv_style_t* style;
	int minus = -1;
	if (list->has_local)
	{
		style = lv_style_list_get_local_style(list);
		minus = (lv_style_prop_exist(style, LV_STYLE_BG_COLOR) ? 1 : -1);
	}
	return (double)lv_color_to32(lv_obj_get_style_bg_color(pimpl->table[0], LV_QTABLE_PART_CELL1)) * minus;
}

double TextList::getEvenLineColor()
{
	if (obj == NULL) return -1;
	lv_style_list_t* list = lv_obj_get_style_list(pimpl->table[0], LV_QTABLE_PART_CELL2);
	lv_style_t* style;
	int minus = -1;
	if (list->has_local)
	{
		style = lv_style_list_get_local_style(list);
		minus = (lv_style_prop_exist(style, LV_STYLE_BG_COLOR) ? 1 : -1);
	}
	return (double)lv_color_to32(lv_obj_get_style_bg_color(pimpl->table[0], LV_QTABLE_PART_CELL2)) * minus;
}

double TextList::getNumColor()
{
	if (obj == NULL) return -1;
	lv_style_list_t* list = lv_obj_get_style_list(pimpl->table[0], LV_QTABLE_PART_CELL1);
	lv_style_t* style;
	int minus = -1;
	if (list->has_local)
	{
		style = lv_style_list_get_local_style(list);
		minus = (lv_style_prop_exist(style, LV_STYLE_TEXT_COLOR) ? 1 : -1);
	}
	return (double)lv_color_to32(lv_obj_get_style_text_color(pimpl->table[0], LV_QTABLE_PART_CELL1)) * minus;
}

double TextList::getTextColor()
{
	if (obj == NULL) return -1;
	lv_style_list_t* list = lv_obj_get_style_list(pimpl->table[0], LV_QTABLE_PART_CELL3);
	lv_style_t* style;
	int minus = -1;
	if (list->has_local)
	{
		style = lv_style_list_get_local_style(list);
		minus = (lv_style_prop_exist(style, LV_STYLE_TEXT_COLOR) ? 1 : -1);
	}
	return (double)lv_color_to32(lv_obj_get_style_text_color(pimpl->table[0], LV_QTABLE_PART_CELL3)) * minus;
}

uint16_t TextList::getLineSpacing()
{
	return pimpl->lnSpacing;
}

int TextList::getTextIndent()
{
	if (obj == NULL) return -1;
	return lv_obj_get_style_pad_right(pimpl->table[0], LV_QTABLE_PART_CELL1);
}

lv_border_side_t TextList::getBorderSide()
{
	return lv_obj_get_style_border_side(pimpl->table[0], LV_TABLE_PART_CELL1);
}

void removeStyleProp(lv_obj_t* obj, uint8_t part, lv_style_property_t prop)
{
	lv_style_list_t* list = lv_obj_get_style_list(obj, part);
	if (list->has_local)
	{
		lv_style_remove_prop(lv_style_list_get_local_style(list), prop);
		lv_obj_refresh_style(obj, part, LV_STYLE_PROP_ALL);
	}
}

static lv_style_property_t get_style_prop(const lv_style_t* style, size_t idx)
{
	lv_style_property_t prop;
	uint8_t* prop_p = (uint8_t*)&prop;
	prop_p[0] = style->map[idx];
	prop_p[1] = style->map[idx + 1];
	return prop;
}

static size_t get_prop_size(uint8_t prop_id)
{
	prop_id &= 0xF;
	size_t size = sizeof(lv_style_property_t);
	if (prop_id < LV_STYLE_ID_COLOR) size += sizeof(lv_style_int_t);
	else if (prop_id < LV_STYLE_ID_OPA) size += sizeof(lv_color_t);
	else if (prop_id < LV_STYLE_ID_PTR) size += sizeof(lv_opa_t);
	else size += sizeof(const void*);
	return size;
}

static uint8_t get_style_prop_id(const lv_style_t* style, size_t idx)
{
	return get_style_prop(style, idx) & 0xFF;
}

static uint8_t get_style_prop_attr(const lv_style_t* style, size_t idx)
{
	return ((get_style_prop(style, idx) >> 8) & 0xFFU);
}

static size_t get_next_prop_index(uint8_t prop_id, size_t idx)
{
	return idx + get_prop_size(prop_id);
}

static bool lv_style_prop_exist(const lv_style_t* style, lv_style_property_t prop)
{
	LV_ASSERT_STYLE(style);

	if (style->map == NULL) return false;

	uint8_t id_to_find = prop & 0xFF;
	lv_style_attr_t attr;
	attr = (prop >> 8) & 0xFF;

	int16_t weight = -1;
	size_t id_guess = -1;

	size_t i = 0;

	uint8_t prop_id;
	while ((prop_id = get_style_prop_id(style, i)) != _LV_STYLE_CLOSING_PROP) {
		if (prop_id == id_to_find) {
			lv_style_attr_t attr_i;
			attr_i = get_style_prop_attr(style, i);

			/*If the state perfectly matches return this property*/
			if (LV_STYLE_ATTR_GET_STATE(attr_i) == LV_STYLE_ATTR_GET_STATE(attr)) {
				return true;
			}
			/* Be sure the property not specifies other state than the requested.
			 * E.g. For HOVER+PRESS, HOVER only is OK, but HOVER+FOCUS not*/
			else if ((LV_STYLE_ATTR_GET_STATE(attr_i) & (~LV_STYLE_ATTR_GET_STATE(attr))) == 0) {
				/* Use this property if it describes better the requested state than the current candidate.
				 * E.g. for HOVER+FOCUS+PRESS prefer HOVER+FOCUS over FOCUS*/
				if (LV_STYLE_ATTR_GET_STATE(attr_i) > weight) {
					weight = LV_STYLE_ATTR_GET_STATE(attr_i);
					id_guess = i;
				}
			}
		}

		i = get_next_prop_index(prop_id, i);
	}

	return false;
}
TextList textList;