
#ifndef LV_THEME_THEME_TOOLS86_H
#define LV_THEME_THEME_TOOLS86_H

#include "src/lv_obj.h"

#ifdef __cplusplus
extern "C" {
#endif
enum {
    SCREEN,
    BG,
    BGCLICK,
    BGSEC,
    BTN,
    PADINNER,
    PADSMALL,
    ARCINDIC,
    ARCBG,
    ARCKNOB,
    BARBG,
    BARINDIC,
    BTNMBG,
    BTNMBTN,
    CALENDARBG,
    CALENDARDATENUMS,
    CALENDARHEADER,
    CALENDARDAYNAMES,
    CPICKERBG,
    CPICKERINDIC,
    CHARTBG,
    CHARTSERIESBG,
    CHARTSERIES,
    CBBG,
    CBBULLET,
    DDLISTMAIN,
    DDLISTPAGE,
    DDLISTSEL,
    GAUGEMAIN,
    GAUGESTRONG,
    GAUGENEEDLE,
    KBBG,
    KBBTN,
    LED,
    LMETER,
    LISTBG,
    LISTBTN,
    MBOXBG,
    MBOXBTNBG,
    MBOXBTN,
    PAGEBG,
    SB,
    EDGEFLASH,
    ROLLERBG,
    ROLLERSEL,
    SLIDERKNOB,
    SLIDERBG,
    SPINBOXBG,
    SPINBOXCCURSOR,
    SPINBOXBTN,
    SWKNOB,
    TABLEBG,
    TABLECELL,
    TABVIEWBG,
    TABVIEWBTNS,
    TABVIEWBTNSBG,
    TABVIEWINDIC,
    TABVIEWPAGESCRL,
    TILEVIEWBG,
    TEXTAREABG,
    TEXTAREACURSOR,
    TEXTAREAPLACEHOLDER,
    WINBG,
    WINHEADER,
    WINBTN,
    CONTBG,
    TABLECELLROW,
    TABLECELLCOL
};

static int v7_theme_ID = 0;
static int v6_theme_ID = 1;

void registerThemeTools(int themeID, int (*func1)(lv_obj_t*, uint8_t), int (*func2)(lv_obj_t*), void (*func3)(lv_obj_t*, uint8_t, int), void (*func4)(lv_obj_t*, int), int (*func5)(lv_obj_t*, int), int (*func6)(lv_obj_t*), double (*func7)(int, int), lv_font_t* (*func8)(int index));
int clearPreColorStyle(lv_obj_t* obj);

int get_font_id_part(lv_obj_t* obj, uint8_t part);
int get_font_id(lv_obj_t* obj);
void set_font_id_part(lv_obj_t* obj, uint8_t part, int id);
int set_font_id(lv_obj_t* obj, int id);

int set_color_id(lv_obj_t* obj, int groupid);
int get_color_id(lv_obj_t* obj);
double get_theme_color(int group, int order);
lv_font_t* get_theme_font(int index);

void setThemeID(int id);
int getPreThemeID();

lv_style_t* get_origin_style(int number);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_THEME_THEME_TOOLS86_H*/