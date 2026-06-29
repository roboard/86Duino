#include "lv_filebrowser.h"
#ifndef WIN32
#include <dirent.h>
#include <iconv.h>
#endif
using namespace LVGL86;

#define LV_SYMBOL_PREVFOLDER           "\xe2\x86\xa9" /*08617, 0x21A9*/
#define MAX_PATH_LENGTH	1024
static int maxFilterCount = 3;
static int _cp_locale = -1;
static char resultBuffer[MAX_PATH_LENGTH];
#ifdef WIN32
const char* previewItems[4][6] = { { "demo", "gcode", "image.jpg", "sound.wav", "font.ttf", NULL },
									{"..", "data", "demo.ino", "myhmi.cpp", "myhmi.h", NULL },
									{"..", "line.gcode", "arc.gcode", "circle.gcode", NULL },
									{"..", "_project.hpj", NULL } };
const char* previewSymbols[4][6] = { { LV_SYMBOL_DIRECTORY, LV_SYMBOL_DIRECTORY, LV_SYMBOL_FILE, LV_SYMBOL_FILE, LV_SYMBOL_FILE, NULL },
									 { LV_SYMBOL_PREVFOLDER, LV_SYMBOL_DIRECTORY, LV_SYMBOL_FILE, LV_SYMBOL_FILE, LV_SYMBOL_FILE, NULL },
									 { LV_SYMBOL_PREVFOLDER, LV_SYMBOL_FILE, LV_SYMBOL_FILE, LV_SYMBOL_FILE, NULL },
									 { LV_SYMBOL_PREVFOLDER, LV_SYMBOL_FILE, NULL } };
#endif


typedef struct FileBrowser::Impl
{
	FileBrowser* fb;
	char* path, ** filterList;
	int basePathLen;
	int Spacing;
	int MaxDisplayFiles;
	void refresh();
	void eventHandler(lv_obj_t*, lv_event_t);
	void focusFolder(char*);
	void focusFile(char*);
} FB_Impl_t;

// https://en.wikipedia.org/wiki/Matching_wildcards
static bool __isMatchingWildcards(char* s, char* p)
{
	char* sp = s, * pp = p, * match = NULL, * star = NULL;
	if (s == NULL || p == NULL) return false;
	for (; *sp != '\0'; ++pp)
		if (*pp != '\0' && *pp == '*')
		{
			match = sp;
			star = pp;
		}
		else if ((*pp != '\0' && *pp == '?') || *pp == *sp)
			++sp;
		else if (star != NULL)
		{
			sp = ++match;
			pp = star;
		}
		else
			return false;
	while (*pp != '\0' && *pp == '*')
		++pp;
	return *pp == '\0';
}

static void __list_btn_event_handler(lv_obj_t* btn, lv_event_t e)
{
	((FB_Impl_t*)get_list_btn_user_data(btn))->eventHandler(btn, e);
}

static int code_convert(char* from_charset, char* to_charset, char* inbuf, size_t inlen, char* outbuf, size_t outlen)
{
#ifndef WIN32
	iconv_t cd;
	int rc;
	char** pin = &inbuf;
	char** pout = &outbuf;

	cd = iconv_open(to_charset, from_charset);
	if (cd == 0) return -1;
	memset(outbuf, 0, outlen);
	if (iconv(cd, pin, &inlen, pout, &outlen) == -1) return -1;
	iconv_close(cd);
#endif
	return 0;
}

static int cp_to_utf8(char* inbuf, size_t inlen, char* outbuf, size_t outlen)
{
	switch (_cp_locale)
	{
#ifndef WIN32
	case 437:	return code_convert("cp437", "utf-8", inbuf, inlen, outbuf, outlen);
		//	case 720:	return code_convert("cp720", "utf-8", inbuf, inlen, outbuf, outlen);
	case 737:	return code_convert("cp737", "utf-8", inbuf, inlen, outbuf, outlen);
	case 775:	return code_convert("cp775", "utf-8", inbuf, inlen, outbuf, outlen);
	case 850:	return code_convert("cp850", "utf-8", inbuf, inlen, outbuf, outlen);
	case 852:	return code_convert("cp852", "utf-8", inbuf, inlen, outbuf, outlen);
	case 855:	return code_convert("cp855", "utf-8", inbuf, inlen, outbuf, outlen);
	case 857:	return code_convert("cp857", "utf-8", inbuf, inlen, outbuf, outlen);
	case 858:	return code_convert("cp858", "utf-8", inbuf, inlen, outbuf, outlen);
	case 860:	return code_convert("cp860", "utf-8", inbuf, inlen, outbuf, outlen);
	case 861:	return code_convert("cp861", "utf-8", inbuf, inlen, outbuf, outlen);
	case 862:	return code_convert("cp862", "utf-8", inbuf, inlen, outbuf, outlen);
	case 863:	return code_convert("cp863", "utf-8", inbuf, inlen, outbuf, outlen);
	case 864:	return code_convert("cp864", "utf-8", inbuf, inlen, outbuf, outlen);
	case 865:	return code_convert("cp865", "utf-8", inbuf, inlen, outbuf, outlen);
	case 866:	return code_convert("cp866", "utf-8", inbuf, inlen, outbuf, outlen);
	case 869:	return code_convert("cp869", "utf-8", inbuf, inlen, outbuf, outlen);
	case 874:	return code_convert("cp874", "utf-8", inbuf, inlen, outbuf, outlen);
	case 932:	return code_convert("cp932", "utf-8", inbuf, inlen, outbuf, outlen);
	case 936:	return code_convert("cp936", "utf-8", inbuf, inlen, outbuf, outlen);
	case 949:	return code_convert("cp949", "utf-8", inbuf, inlen, outbuf, outlen);
	case 950:	return code_convert("cp950", "utf-8", inbuf, inlen, outbuf, outlen);
	case 1258:	return code_convert("cp1258", "utf-8", inbuf, inlen, outbuf, outlen);
#endif
	default:
		outlen = inlen;
		strcpy(outbuf, inbuf);
	}
	return 0;
}

static int utf8_to_cp(char* inbuf, size_t inlen, char* outbuf, size_t outlen)
{
	switch (_cp_locale)
	{
#ifndef WIN32
	case 437:	return code_convert("utf-8", "cp437", inbuf, inlen, outbuf, outlen);
		//	case 720:	return code_convert("utf-8", "cp720", inbuf, inlen, outbuf, outlen);
	case 737:	return code_convert("utf-8", "cp737", inbuf, inlen, outbuf, outlen);
	case 775:	return code_convert("utf-8", "cp775", inbuf, inlen, outbuf, outlen);
	case 850:	return code_convert("utf-8", "cp850", inbuf, inlen, outbuf, outlen);
	case 852:	return code_convert("utf-8", "cp852", inbuf, inlen, outbuf, outlen);
	case 855:	return code_convert("utf-8", "cp855", inbuf, inlen, outbuf, outlen);
	case 857:	return code_convert("utf-8", "cp857", inbuf, inlen, outbuf, outlen);
	case 858:	return code_convert("utf-8", "cp858", inbuf, inlen, outbuf, outlen);
	case 860:	return code_convert("utf-8", "cp860", inbuf, inlen, outbuf, outlen);
	case 861:	return code_convert("utf-8", "cp861", inbuf, inlen, outbuf, outlen);
	case 862:	return code_convert("utf-8", "cp862", inbuf, inlen, outbuf, outlen);
	case 863:	return code_convert("utf-8", "cp863", inbuf, inlen, outbuf, outlen);
	case 864:	return code_convert("utf-8", "cp864", inbuf, inlen, outbuf, outlen);
	case 865:	return code_convert("utf-8", "cp865", inbuf, inlen, outbuf, outlen);
	case 866:	return code_convert("utf-8", "cp866", inbuf, inlen, outbuf, outlen);
	case 869:	return code_convert("utf-8", "cp869", inbuf, inlen, outbuf, outlen);
	case 874:	return code_convert("utf-8", "cp874", inbuf, inlen, outbuf, outlen);
	case 932:	return code_convert("utf-8", "cp932", inbuf, inlen, outbuf, outlen);
	case 936:	return code_convert("utf-8", "cp936", inbuf, inlen, outbuf, outlen);
	case 949:	return code_convert("utf-8", "cp949", inbuf, inlen, outbuf, outlen);
	case 950:	return code_convert("utf-8", "cp950", inbuf, inlen, outbuf, outlen);
	case 1258:	return code_convert("utf-8", "cp1258", inbuf, inlen, outbuf, outlen);
#endif
	default:
		outlen = inlen;
		strcpy(outbuf, inbuf);
	}
	return 0;
}

void FileBrowser::Impl::refresh()
{
#ifndef WIN32
	DIR* d;
	struct dirent* dir = NULL;
	lv_obj_t* btn;
	int cnt = 0;
	list_clean(fb->obj);
	char outbuf[MAX_PATH_LENGTH];
	void (*enable_cb)(lv_obj_t*, lv_state_t) = fb->Enable ? lv_obj_clear_state : lv_obj_add_state;

	if (!(d = opendir(path)))
	{
		for (int i = 0; i < 10; i++)
		{
			btn = lv_list_add_btn(fb->obj, LV_SYMBOL_WARNING, "Path Not Exist");

			lv_obj_set_style_local_pad_top(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Spacing);
			lv_obj_set_style_local_pad_bottom(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Spacing);

			set_color_id(btn, fb->getHMIColor());
			set_font_id(btn, fb->getHMITextFont());
			lv_obj_add_state(btn, LV_STATE_DISABLED);
		}
		return;
	}

	while ((dir = readdir(d)) != NULL)
	{
		cp_to_utf8(dir->d_name, MAX_PATH_LENGTH, outbuf, sizeof(outbuf));
		if (!strcmp(dir->d_name, "."))
			continue;
		else if (!strcmp(dir->d_name, ".."))
		{
			if (strlen(path) <= basePathLen)
				continue;
			btn = lv_list_add_btn(fb->obj, LV_SYMBOL_PREVFOLDER, dir->d_name);
		}
		else if (dir->d_type == DT_DIR)
			btn = lv_list_add_btn(fb->obj, LV_SYMBOL_DIRECTORY, outbuf);
		else
		{
			bool usingFilter = false;
			bool flag = false;
			for (int i = 0; i < maxFilterCount; i++)
				if (filterList[i])
				{
					flag |= __isMatchingWildcards(outbuf, filterList[i]);
					usingFilter = true;
				}
			if (!flag && usingFilter) continue;

			btn = lv_list_add_btn(fb->obj, LV_SYMBOL_FILE, outbuf);
		}
		lv_obj_set_style_local_pad_top(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Spacing);
		lv_obj_set_style_local_pad_bottom(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Spacing);
		set_color_id(btn, fb->getHMIColor());
		set_font_id(btn, fb->getHMITextFont());
		enable_cb(btn, LV_STATE_DISABLED);


		set_list_btn_user_data(btn, this);
		set_list_btn_event(btn, __list_btn_event_handler);
		if (++cnt > MaxDisplayFiles)
			break;
	}
	closedir(d);
#else
	int i = 0, j = 0;
	void (*enable_cb)(lv_obj_t*, lv_state_t) = fb->Enable ? lv_obj_clear_state : lv_obj_add_state;
	list_clean(fb->obj);
	if (strstr(path, previewItems[1][1]))
		i = 3;
	else if (strstr(path, previewItems[0][1]))
		i = 2;
	else if (strstr(path, previewItems[0][0]))
		i = 1;
	else
		i = 0;

	lv_obj_t* btn;
	for (; previewItems[i][j] != NULL; ++j)
	{
		btn = lv_list_add_btn(fb->obj, previewSymbols[i][j], previewItems[i][j]);

		lv_obj_set_style_local_pad_top(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Spacing);
		lv_obj_set_style_local_pad_bottom(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Spacing);

		set_color_id(btn, fb->getHMIColor());
		set_font_id(btn, fb->getHMITextFont());
		enable_cb(btn, LV_STATE_DISABLED);
		set_list_btn_user_data(btn, this);
		set_list_btn_event(btn, __list_btn_event_handler);
		if (i == 0 && j == 2)
			lv_obj_set_state(btn, LV_STATE_CHECKED);
	}
#endif
}

void FileBrowser::Impl::eventHandler(lv_obj_t* obj, lv_event_t e)
{
	if (e != LV_EVENT_CLICKED) return;
	char* symbol, * btntxt, * ptr;
	char outbuf[MAX_PATH_LENGTH];
	lv_obj_t* img = lv_list_get_btn_img(obj);
	if (img != NULL)
	{
		symbol = (char*)lv_img_get_src(img);
		btntxt = (char*)lv_list_get_btn_text(obj);
#ifndef WIN32
		utf8_to_cp((char*)lv_list_get_btn_text(obj), MAX_PATH_LENGTH, outbuf, sizeof(outbuf));
#else
		strcpy(outbuf, (char*)lv_list_get_btn_text(obj));
#endif
		ptr = strrchr(path, '\\');
		if (!strcmp(symbol, (char*)LV_SYMBOL_PREVFOLDER) && strlen(path) > basePathLen)
		{
			*ptr = '\0';
			ptr = strrchr(path, '\\') + 1;
			strcpy(outbuf, ptr);
			*ptr = '\0';

			refresh();
			focusFile(outbuf);
		}
		else if (!strcmp(symbol, (char*)LV_SYMBOL_DIRECTORY) && strlen(ptr + 1) + strlen(outbuf) + 2 <= MAX_PATH_LENGTH)
		{
			*(ptr + 1) = '\0';
			strcat(path, outbuf);
			strcat(path, "\\");
			refresh();
		}
		else if (!strcmp(symbol, (char*)LV_SYMBOL_FILE) && strlen(ptr + 1) + strlen(outbuf) + 1 <= MAX_PATH_LENGTH)
		{
			*(ptr + 1) = '\0';
			strcat(path, outbuf);
		}
	}
}

void FileBrowser::Impl::focusFolder(char* p)
{
	char tmp[MAX_PATH_LENGTH];
	lv_obj_t* child, * label;
	int skip = -1;
	if (p == NULL) return;
	if (strlen(p) == 0) return;

	utf8_to_cp(p, MAX_PATH_LENGTH, tmp, sizeof(tmp));

	for (int i = 0; tmp[i] != '\0'; i++)
	{
		if (tmp[i] == '/') 
			tmp[i] = '\\';
		if (skip < 0 && tmp[i] != '\\')
			skip = i;
	}
	memmove(tmp, tmp + skip, strlen(tmp));

	if (tmp[strlen(tmp) - 1] != '\\')
		strcat(tmp, "\\");

	path[basePathLen] = '\0';
	strcat(path, tmp);
#ifndef WIN32
	DIR* d;
	if (d = opendir(path))
		closedir(d);
	else
		path[basePathLen] = '\0';
#endif
	refresh();
}

void FileBrowser::Impl::focusFile(char* fileName)
{
	lv_obj_t* btn, * label, * img;
	char* symbol;
	if (fileName == NULL) return;
	if (strlen(fileName) == 0) return;
	btn = NULL;
	while (btn = lv_list_get_next_btn(fb->obj, btn))
	{
		if (strcmp(fileName, (char*)lv_list_get_btn_text(btn)) == 0) {
			lv_list_focus_btn(fb->obj, btn);

			img = lv_list_get_btn_img(btn);
			symbol = (char*)lv_img_get_src(img);
			if (!strcmp(symbol, (char*)LV_SYMBOL_FILE) && !is_list_btn_selected(btn))
				lv_event_send(btn, LV_EVENT_CLICKED, NULL);
			break;
		}
	}
}


FileBrowser::FileBrowser()
{
	X = 320, Y = 100, Width = 260, Height = 390, Enable = true;

	pimpl = (FB_Impl_t*)malloc(sizeof(FB_Impl_t));
	pimpl->fb = this;
	pimpl->path = (char*)malloc(sizeof(char) * MAX_PATH_LENGTH);
	pimpl->filterList = (char**)malloc(sizeof(char*) * maxFilterCount);
	memset(pimpl->filterList, NULL, sizeof(char*) * maxFilterCount);
	pimpl->Spacing = 15;
	pimpl->MaxDisplayFiles = 128;
}

FileBrowser::~FileBrowser()
{
	if (pimpl)
	{
		if (pimpl->path) free(pimpl->path);
		if (pimpl->filterList)
		{
			for (int i = 0; i < maxFilterCount; i++)
				if (pimpl->filterList[i])
					free(pimpl->filterList[i]);
			free(pimpl->filterList);
		}
		free(pimpl);
	}
	if (obj) list_destroy(obj);
	obj = NULL;
}

lv_obj_t* FileBrowser::begin(lv_obj_t* parent, int _x, int _y, int _width, int _height, int8_t _color, int8_t _font, int _spacing, int _maxdisplayfiles, bool _enable)
{
	if (parent == NULL)
		obj = lv_list_create(lv_scr_act(), NULL);
	else
		obj = lv_list_create(parent, NULL);

	set_list_mode(obj, MODE_ONESEL);

	setXY(_x, _y);
	setSize(_width, _height);
	setHMIColor(_color);
	setHMITextFont(_font);
	setEnable(_enable);
#ifndef WIN32
	if (!SD.getDiskPath(SD.getBank(), pimpl->path))
		strcpy(pimpl->path, "C:\\");
#else
	strcpy(pimpl->path, "C:\\");
#endif
	pimpl->Spacing = _spacing;
	pimpl->MaxDisplayFiles = _maxdisplayfiles;
	pimpl->basePathLen = strlen(pimpl->path);
	pimpl->refresh();
	return obj;
}

void FileBrowser::setFilter(char* filter, int index)
{
	if (index < 0 || index > maxFilterCount) return;

	if (pimpl->filterList[index])
		free(pimpl->filterList[index]);
	pimpl->filterList[index] = (char*)strdup(filter);
	pimpl->refresh();
}

void FileBrowser::removeFilter(int index)
{
	if (index < 0)
	{
		for (int i = 0; i < maxFilterCount; i++)
		{
			if (pimpl->filterList[i])
				free(pimpl->filterList[i]);
			pimpl->filterList[i] = NULL;
		}
	}
	else if (index < maxFilterCount)
	{
		if (pimpl->filterList[index])
			free(pimpl->filterList[index]);
		pimpl->filterList[index] = NULL;
	}
	pimpl->refresh();
}

const char* FileBrowser::getCurrentFolderPath(bool fullPath, char* buf)
{
	char* ptr = strrchr(pimpl->path, '\\') + 1;
	*ptr = '\0';
	cp_to_utf8(pimpl->path + (fullPath ? 0 : pimpl->basePathLen), MAX_PATH_LENGTH, resultBuffer, sizeof(resultBuffer));
	if (buf)
		strcpy(buf, resultBuffer);
	return resultBuffer;
}

const char* FileBrowser::getSelectedFilePath(bool fullPath, char* buf, int index)
{
	int i = 0;
	lv_obj_t* btn = NULL;
	while (btn = lv_list_get_next_btn(obj, btn))
	{
		lv_obj_t* img = lv_list_get_btn_img(btn);
		char* symbol = (char*)lv_img_get_src(img);
		if (index < 0 && is_list_btn_selected(btn))
		{
			if (!strcmp(symbol, (char*)LV_SYMBOL_FILE))
			{
				*(strrchr(pimpl->path, '\\') + 1) = '\0';
				cp_to_utf8(pimpl->path, MAX_PATH_LENGTH, resultBuffer, sizeof(resultBuffer));
				strcat(resultBuffer, (char*)lv_list_get_btn_text(btn));
				if (buf)
					strcpy(buf, resultBuffer + (fullPath ? 0 : pimpl->basePathLen));
				return resultBuffer + (fullPath ? 0 : pimpl->basePathLen);
			}
			break;
		}
		else if (strcmp(symbol, (char*)LV_SYMBOL_PREVFOLDER))
		{
			if (index == i)
			{
				*(strrchr(pimpl->path, '\\') + 1) = '\0';
				cp_to_utf8(pimpl->path, MAX_PATH_LENGTH, resultBuffer, sizeof(resultBuffer));
				strcat(resultBuffer, (char*)lv_list_get_btn_text(btn));

				if (!strcmp(symbol, (char*)LV_SYMBOL_DIRECTORY))
					strcat(resultBuffer, "\\");
				if (buf)
					strcpy(buf, resultBuffer + (fullPath ? 0 : pimpl->basePathLen));
				return resultBuffer + (fullPath ? 0 : pimpl->basePathLen);
			}
			i++;
		}
	}
	memset(resultBuffer, '\0', sizeof(resultBuffer));
	if (buf)
		strcpy(buf, resultBuffer);
	if (index >= 0)
		return NULL;
	return resultBuffer;
}

void FileBrowser::setSelectedFilePath(char* fileName)
{
	pimpl->focusFile(fileName);
}

void FileBrowser::setCurrentFolderPath(char* path)
{
	pimpl->focusFolder(path);
}

void FileBrowser::setXY(int x, int y)
{
	if (obj == NULL) return;
	X = x;
	Y = y;
	lv_obj_set_pos(obj, X, Y);
}

void FileBrowser::setSize(int w, int h)
{
	if (obj == NULL) return;
	Width = w;
	Height = h;
	lv_obj_set_width(obj, Width);
	lv_obj_set_height(obj, Height);
}

void FileBrowser::setHMIColor(int colorId)
{
	if (obj == NULL) return;
	set_color_id(obj, colorId);
}

void FileBrowser::setHMITextFont(int fontId)
{
	if (obj == NULL) return;
	set_font_id(obj, fontId);
}

void FileBrowser::setEnable(bool enable)
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

void FileBrowser::setSpacing(int _spacing)
{
	pimpl->Spacing = _spacing;
	pimpl->refresh();
}

void FileBrowser::setDisk(int bank)
{
#ifndef WIN32
	SD.getDiskPath(bank, pimpl->path);
	pimpl->basePathLen = strlen(pimpl->path);
#endif
	pimpl->refresh();
}

void FileBrowser::_setDoslfnCodepage(int codepage)
{
	switch (codepage)
	{
		case 437:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp437uni.tbl");	_cp_locale = codepage;	break;
		// case 720:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp720uni.tbl");	_cp_locale = codepage;	break;
		case 737:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp737uni.tbl");	_cp_locale = codepage;	break;
		case 775:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp775uni.tbl");	_cp_locale = codepage;	break;
		case 850:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp850uni.tbl");	_cp_locale = codepage;	break;
		case 852:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp852uni.tbl");	_cp_locale = codepage;	break;
		case 855:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp855uni.tbl");	_cp_locale = codepage;	break;
		case 857:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp857uni.tbl");	_cp_locale = codepage;	break;
		case 858:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp858uni.tbl");	_cp_locale = codepage;	break;
		case 860:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp860uni.tbl");	_cp_locale = codepage;	break;
		case 861:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp861uni.tbl");	_cp_locale = codepage;	break;
		case 862:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp862uni.tbl");	_cp_locale = codepage;	break;
		case 863:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp863uni.tbl");	_cp_locale = codepage;	break;
		case 864:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp864uni.tbl");	_cp_locale = codepage;	break;
		case 865:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp865uni.tbl");	_cp_locale = codepage;	break;
		case 866:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp866uni.tbl");	_cp_locale = codepage;	break;
		case 869:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp869uni.tbl");	_cp_locale = codepage;	break;
		case 874:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp874uni.tbl");	_cp_locale = codepage;	break;
		case 932:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp932uni.tbl");	_cp_locale = codepage;	break;
		case 936:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp936uni.tbl");	_cp_locale = codepage;	break;
		case 949:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp949uni.tbl");	_cp_locale = codepage;	break;
		case 950:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\cp950uni.tbl");	_cp_locale = codepage;	break;
		case 1258:	system("doslfn /u");	system("doslfn /z:a:\\fdos\\bin\\c1258uni.tbl");	_cp_locale = codepage;	break;
	}
}

FileBrowser fileBrowser;