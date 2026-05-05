#include "src/lv_theme.h"
#include "lv_theme_tools.h"

int preThemeID = -1;
int nowThemeID = -1;

typedef struct {
    int (*_get_font_id_part)(lv_obj_t* obj, uint8_t part);
    int (*_get_font_id)(lv_obj_t* obj);
    void (*_set_font_id_part)(lv_obj_t* obj, uint8_t part, int id);
    int (*_set_font_id)(lv_obj_t* obj, int id);
    int (*_set_color_id)(lv_obj_t* obj, int groupid);
    int (*_get_color_id)(lv_obj_t* obj);
    double (*_get_theme_color)(int group, int order);
    lv_font_t* (*_get_theme_font)(int index);
} THEMEOPS;

static THEMEOPS theme[6]; // allow register 6 themes: 0 ~ 5

void registerThemeTools(int themeID, int (*func1)(lv_obj_t*, uint8_t), int (*func2)(lv_obj_t*), void (*func3)(lv_obj_t*, uint8_t, int ), void (*func4)(lv_obj_t*, int), int (*func5)(lv_obj_t*, int), int (*func6)(lv_obj_t*), double (*func7)(int, int), lv_font_t* (*func8)(int index)) {

    if (themeID > 5) return;
    
    theme[themeID]._get_font_id_part = func1;
	theme[themeID]._get_font_id      = func2;
	theme[themeID]._set_font_id_part = func3;
	theme[themeID]._set_font_id      = func4;
	theme[themeID]._set_color_id     = func5;
	theme[themeID]._get_color_id     = func6;
    theme[themeID]._get_theme_color  = func7;
    theme[themeID]._get_theme_font   = func8;
    
    nowThemeID = themeID;
}

// only use preThemeID
int clearPreColorStyle(lv_obj_t* obj) {
    int groupid;
    
    if (preThemeID == -1) return -1;
    if ((groupid = theme[preThemeID]._set_color_id(obj, -1)) != -1) return groupid;
    
    return -1;
}

// only use preThemeID
int clearPreFontStyle(lv_obj_t* obj) {
    int fontid;
    
    if (preThemeID == -1) return -1;
    if ((fontid = theme[preThemeID]._set_font_id(obj, -1)) != -1) return fontid;
    
    return -1;
}

int getPreThemeID() {
    return preThemeID;
}

void setThemeID(int id) {
    preThemeID = id; // here, preThemeID = nowThemeID
}

int get_font_id_part(lv_obj_t* obj, uint8_t part) {
	if (nowThemeID == -1 || theme[nowThemeID]._get_font_id_part == NULL) return -1;

    return theme[nowThemeID]._get_font_id_part(obj, part);
}

int get_font_id(lv_obj_t* obj) {
	if (nowThemeID == -1 || theme[nowThemeID]._get_font_id == NULL) return -1;
    return theme[nowThemeID]._get_font_id(obj);
}

void set_font_id_part(lv_obj_t* obj, uint8_t part, int id) {
	if (nowThemeID == -1 || theme[nowThemeID]._set_font_id_part == NULL) return;
    
    theme[nowThemeID]._set_font_id_part(obj, part, id);
}

int set_font_id(lv_obj_t* obj, int id){
	if (nowThemeID == -1 || theme[nowThemeID]._set_font_id == NULL) return -1;
    
    return theme[nowThemeID]._set_font_id(obj, id);
}

int whichSpecialCase(lv_obj_t* obj);
void specialCase(int n, int themeID, lv_obj_t* obj, int groupid);
int set_color_id(lv_obj_t* obj, int groupid) {
	if (nowThemeID == -1 || theme[nowThemeID]._set_color_id == NULL) return -1;
    
    int n = whichSpecialCase(obj);
    if (n > 0) {
        specialCase(n, nowThemeID, obj, groupid);
        return 0;
    }
    
    return theme[nowThemeID]._set_color_id(obj, groupid);
}

int get_color_id(lv_obj_t* obj) {
	if (nowThemeID == -1 || theme[nowThemeID]._get_color_id == NULL)  return -1;
    
    return theme[nowThemeID]._get_color_id(obj);
}

double get_theme_color(int group, int order) {
    if (nowThemeID == -1 || theme[nowThemeID]._get_theme_color == NULL)  return -1;
    
    return theme[nowThemeID]._get_theme_color(group, order);
}

lv_font_t* get_theme_font(int index) {
    if (nowThemeID == -1 || theme[nowThemeID]._get_theme_font == NULL)  return NULL;
    
    return theme[nowThemeID]._get_theme_font(index);
}


lv_style_t* get_origin_style(int number) {
    lv_theme_t* theme = lv_theme_get_act();
    lv_style_t** _style = (lv_style_t**)theme->user_data;
    return _style[number];
}

int whichSpecialCase(lv_obj_t* obj) {
    if (LV_DEBUG_IS_OBJ(obj, "lv_list")) return 1;
    return 0;
}

void specialCase(int n, int themeID, lv_obj_t* obj, int groupid) {
    if (n == 1) { // list
        theme[themeID]._set_color_id(obj, groupid);
        lv_obj_t* btn = NULL;
        while (btn = lv_list_get_next_btn(obj, btn))
            theme[themeID]._set_color_id(btn, groupid);
    }
}
