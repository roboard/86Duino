/**
 * @file lv_theme_lvgl6.c
 *
 */

/*********************
*      INCLUDES
*********************/
#include "lvgl.h"
#include "src/lv_gc.h"
#include "lv_theme_tools.h"

 /*********************
 *      DEFINES
 *********************/
#define COLOR_MY_GREY     (LV_COLOR_MAKE(0x61, 0x61, 0x61))

 /**********************
 *      TYPEDEFS
 **********************/
static lv_style_t def;
static lv_style_t panel;
static lv_style_t btn_rel;
static lv_style_t lv_style_transp;
static lv_style_t lv_style_transp_tight;

typedef struct {
    lv_style_t scr;
    lv_style_t bg;
    lv_style_t bg_click;
    lv_style_t bg_sec;
    lv_style_t btn;
    lv_style_t pad_inner;
    lv_style_t pad_small;
    lv_style_t arc_indic;
    lv_style_t arc_bg;
    lv_style_t arc_knob;
    lv_style_t bar_bg;
    lv_style_t bar_indic;
    lv_style_t btnm_bg, btnm_btn;
    lv_style_t cont_bg; // 86Duino
    lv_style_t calendar_bg, calendar_date_nums, calendar_header, calendar_daynames;
    lv_style_t cpicker_bg, cpicker_indic;
    lv_style_t chart_bg, chart_series_bg, chart_series;
    lv_style_t cb_bg, cb_bullet;
    lv_style_t ddlist_main, ddlist_page, ddlist_sel;
    lv_style_t gauge_main, gauge_strong, gauge_needle;
    lv_style_t kb_bg, kb_btn;
    lv_style_t led;
    lv_style_t lmeter;
    lv_style_t list_bg, list_btn;
    lv_style_t mbox_bg, mbox_btn_bg, mbox_btn;
    lv_style_t page_bg, sb;
    lv_style_t edge_flash;
    lv_style_t roller_bg, roller_sel;
    lv_style_t slider_knob, slider_bg;
    lv_style_t spinbox_bg, spinbox_cursor, spinbox_btn;
    lv_style_t sw_knob;
    lv_style_t table_bg, table_cell, table_cell_row, table_cell_col;
    lv_style_t tabview_bg, tabview_btns, tabview_btns_bg, tabview_indic, tabview_page_scrl;
    lv_style_t tileview_bg;
    lv_style_t ta_bg, ta_cursor, ta_placeholder;
    lv_style_t win_bg, win_header, win_btn;
    lv_style_t qtable_bg, qtable_cell;
} theme_styles_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void theme_apply(lv_theme_t* th, lv_obj_t* obj, lv_theme_style_t name);
static void style_init_reset(lv_style_t* style);
static lv_color_t* theme_colors = NULL;

#define FONTSIZE    9
static lv_font_t** theme_fonts = NULL;
static lv_style_t _font[FONTSIZE];
static lv_style_t* style_font[FONTSIZE];

#define CL_STYLE_SIZE    512
static lv_style_t _color[CL_STYLE_SIZE];
static struct _group {
    lv_obj_t* obj;
    int groupid;
    int themeid;
    lv_style_t* style;
} style_color[CL_STYLE_SIZE];
static int scCount = 0;

#define PALETTESIZE    39
lv_color_t _palette[PALETTESIZE];
double     _org_color[PALETTESIZE];

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_theme_t theme;
static theme_styles_t* styles;

static bool inited = false;

/**********************
 *      MACROS
 **********************/

 /**********************
  *   STATIC FUNCTIONS
  **********************/

static lv_color_t recalculate_hsv(lv_color_t mcolor, uint8_t ratio, uint8_t default_s, uint8_t default_v)
{
    lv_color_hsv_t hsv = lv_color_to_hsv(mcolor);
    return lv_color_hsv_to_rgb(hsv.h,
        default_s + (hsv.s - default_s) * ratio / 100,
        default_v + (hsv.v - default_v) * ratio / 100);
}

static void basic_init(void)
{
    style_init_reset(&lv_style_transp);
    lv_style_set_bg_opa(&lv_style_transp, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_bg_color(&lv_style_transp, LV_STATE_DEFAULT, theme_colors[35]);
    lv_style_set_bg_grad_color(&lv_style_transp, LV_STATE_DEFAULT, theme_colors[35]);
    lv_style_set_bg_grad_dir(&lv_style_transp, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
    lv_style_set_radius(&lv_style_transp, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_left(&lv_style_transp, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_right(&lv_style_transp, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_top(&lv_style_transp, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_bottom(&lv_style_transp, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_inner(&lv_style_transp, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_border_color(&lv_style_transp, LV_STATE_DEFAULT, theme_colors[24]);
    lv_style_set_border_opa(&lv_style_transp, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_border_width(&lv_style_transp, LV_STATE_DEFAULT, 0);
    lv_style_set_border_side(&lv_style_transp, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL);
    lv_style_set_shadow_color(&lv_style_transp, LV_STATE_DEFAULT, theme_colors[31]);
    lv_style_set_shadow_width(&lv_style_transp, LV_STATE_DEFAULT, 0);
    lv_style_set_text_opa(&lv_style_transp, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&lv_style_transp, LV_STATE_DEFAULT, theme_colors[27]);
    lv_style_set_text_sel_color(&lv_style_transp, LV_STATE_DEFAULT, theme_colors[30]);
    lv_style_set_text_font(&lv_style_transp, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_text_letter_space(&lv_style_transp, LV_STATE_DEFAULT, 0);
    lv_style_set_text_line_space(&lv_style_transp, LV_STATE_DEFAULT, 2);
    lv_style_set_image_opa(&lv_style_transp, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_image_recolor(&lv_style_transp, LV_STATE_DEFAULT, theme_colors[26]);
    lv_style_set_line_opa(&lv_style_transp, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_line_color(&lv_style_transp, LV_STATE_DEFAULT, theme_colors[26]);
    lv_style_set_line_width(&lv_style_transp, LV_STATE_DEFAULT, 2);
    lv_style_set_line_rounded(&lv_style_transp, LV_STATE_DEFAULT, 0);
    lv_style_set_value_font(&lv_style_transp, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&lv_style_transp_tight);
    lv_style_copy(&lv_style_transp_tight, &lv_style_transp);
    lv_style_set_pad_left(&lv_style_transp_tight, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&lv_style_transp_tight, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&lv_style_transp_tight, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&lv_style_transp_tight, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_inner(&lv_style_transp_tight, LV_STATE_DEFAULT, 0);

    style_init_reset(&def); // color will be fixed
    lv_style_set_bg_opa(&def, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&def, LV_STATE_DEFAULT, theme_colors[35]);
    lv_style_set_bg_grad_color(&def, LV_STATE_DEFAULT, theme_colors[33]);
    lv_style_set_bg_grad_dir(&def, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
    lv_style_set_radius(&def, LV_STATE_DEFAULT, LV_DPI / 15);
    lv_style_set_pad_left(&def, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_right(&def, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_top(&def, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_bottom(&def, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_inner(&def, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_border_color(&def, LV_STATE_DEFAULT, theme_colors[28]);
    lv_style_set_border_opa(&def, LV_STATE_DEFAULT, LV_OPA_30);
    lv_style_set_border_width(&def, LV_STATE_DEFAULT, LV_DPI / 50 >= 1 ? LV_DPI / 50 : 1);
    lv_style_set_border_side(&def, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL);
    lv_style_set_shadow_color(&def, LV_STATE_DEFAULT, theme_colors[31]);
    lv_style_set_shadow_width(&def, LV_STATE_DEFAULT, 0);
    lv_style_set_text_color(&def, LV_STATE_DEFAULT, theme_colors[27]);
    lv_style_set_text_sel_color(&def, LV_STATE_DEFAULT, theme_colors[30]);
    lv_style_set_text_letter_space(&def, LV_STATE_DEFAULT, 0);
    lv_style_set_text_line_space(&def, LV_STATE_DEFAULT, 2);
    lv_style_set_image_recolor(&def, LV_STATE_DEFAULT, theme_colors[26]);
    lv_style_set_line_color(&def, LV_STATE_DEFAULT, theme_colors[26]);
    lv_style_set_line_width(&def, LV_STATE_DEFAULT, 2);
    lv_style_set_line_rounded(&def, LV_STATE_DEFAULT, 0);
    lv_style_set_value_color(&def, LV_STATE_DEFAULT, theme_colors[27]);
    lv_style_set_text_sel_bg_color(&def, LV_STATE_DEFAULT, theme_colors[30]);
    lv_style_set_value_font(&def, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_text_font(&def, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->bg);
    lv_style_set_bg_opa(&styles->bg, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->bg, LV_STATE_DEFAULT, theme_colors[36]);
    lv_style_set_bg_grad_color(&styles->bg, LV_STATE_DEFAULT, theme_colors[36]);
    lv_style_set_bg_grad_dir(&styles->bg, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
    lv_style_set_radius(&styles->bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_left(&styles->bg, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_right(&styles->bg, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_top(&styles->bg, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_bottom(&styles->bg, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_inner(&styles->bg, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_border_color(&styles->bg, LV_STATE_DEFAULT, theme_colors[24]);
    lv_style_set_border_opa(&styles->bg, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_border_width(&styles->bg, LV_STATE_DEFAULT, 0);
    lv_style_set_border_side(&styles->bg, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL);
    lv_style_set_border_post(&styles->bg, LV_STATE_DEFAULT, true);
    lv_style_set_shadow_color(&styles->bg, LV_STATE_DEFAULT, theme_colors[31]);
    lv_style_set_shadow_width(&styles->bg, LV_STATE_DEFAULT, 0);
    lv_style_set_text_color(&styles->bg, LV_STATE_DEFAULT, theme_colors[34]);
    lv_style_set_text_sel_color(&styles->bg, LV_STATE_DEFAULT, theme_colors[30]);
    lv_style_set_text_letter_space(&styles->bg, LV_STATE_DEFAULT, 0);
    lv_style_set_text_line_space(&styles->bg, LV_STATE_DEFAULT, 2);
    lv_style_set_image_recolor(&styles->bg, LV_STATE_DEFAULT, theme_colors[34]);
    lv_style_set_line_color(&styles->bg, LV_STATE_DEFAULT, theme_colors[26]);
    lv_style_set_line_width(&styles->bg, LV_STATE_DEFAULT, 2);
    lv_style_set_line_rounded(&styles->bg, LV_STATE_DEFAULT, 0);
    lv_style_set_value_color(&styles->bg, LV_STATE_DEFAULT, theme_colors[34]);
    lv_style_set_text_sel_bg_color(&styles->bg, LV_STATE_DEFAULT, theme_colors[30]);
    lv_style_set_value_font(&styles->bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_text_font(&styles->bg, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->scr);
    lv_style_copy(&styles->scr, &styles->bg);
    lv_style_set_pad_left(&styles->scr, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&styles->scr, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&styles->scr, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&styles->scr, LV_STATE_DEFAULT, 0);

    style_init_reset(&styles->sb);
    lv_style_copy(&styles->sb, &def);
    lv_style_set_bg_color(&styles->sb, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 30, 60));
    lv_style_set_bg_grad_color(&styles->sb, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 30, 60));
    lv_style_set_border_width(&styles->sb, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_inner(&styles->sb, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_left(&styles->sb, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&styles->sb, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&styles->sb, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&styles->sb, LV_STATE_DEFAULT, 0);
    lv_style_set_radius(&styles->sb, LV_STATE_DEFAULT, LV_DPI / 30);
    lv_style_set_bg_opa(&styles->sb, LV_STATE_DEFAULT, LV_OPA_COVER);

    style_init_reset(&panel);
    lv_style_copy(&panel, &styles->bg);
    lv_style_set_bg_color(&panel, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 11, 18));
    lv_style_set_bg_grad_color(&panel, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 11, 18));
    lv_style_set_radius(&panel, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_border_color(&panel, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 25));
    lv_style_set_border_width(&panel, LV_STATE_DEFAULT, 1);
    lv_style_set_border_opa(&panel, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_pad_left(&panel, LV_STATE_DEFAULT, LV_DPI / 10);
    lv_style_set_pad_right(&panel, LV_STATE_DEFAULT, LV_DPI / 10);
    lv_style_set_pad_top(&panel, LV_STATE_DEFAULT, LV_DPI / 10);
    lv_style_set_pad_bottom(&panel, LV_STATE_DEFAULT, LV_DPI / 10);
    lv_style_set_line_color(&panel, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 20, 40));
    lv_style_set_line_width(&panel, LV_STATE_DEFAULT, 1);

    style_init_reset(&styles->pad_small);
    lv_style_int_t pad_small_value = lv_disp_get_size_category(NULL) <= LV_DISP_MEDIUM_LIMIT ? LV_DPX(10) : LV_DPX(20);
    lv_style_set_pad_left(&styles->pad_small, LV_STATE_DEFAULT, pad_small_value);
    lv_style_set_pad_right(&styles->pad_small, LV_STATE_DEFAULT, pad_small_value);
    lv_style_set_pad_top(&styles->pad_small, LV_STATE_DEFAULT, pad_small_value);
    lv_style_set_pad_bottom(&styles->pad_small, LV_STATE_DEFAULT, pad_small_value);
    lv_style_set_pad_inner(&styles->pad_small, LV_STATE_DEFAULT, pad_small_value);

    // Note: This style is uesd on imgbtn for us now, because it is empty before.
    style_init_reset(&styles->bg_sec); // for imgbtn obj
    lv_style_set_text_color(&styles->bg_sec, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    // lv_style_set_image_recolor_opa(&styles->bg_sec, LV_STATE_PRESSED, LV_OPA_30);
    // lv_style_set_image_recolor(&styles->bg_sec, LV_STATE_PRESSED, LV_COLOR_BLACK);
    lv_style_set_image_recolor(&styles->bg_sec, LV_STATE_DISABLED, COLOR_MY_GREY);
    lv_style_set_image_recolor(&styles->bg_sec, LV_STATE_DISABLED | LV_STATE_CHECKED, COLOR_MY_GREY);
    lv_style_set_image_recolor_opa(&styles->bg_sec, LV_STATE_DISABLED, LV_OPA_50);
    lv_style_set_image_recolor_opa(&styles->bg_sec, LV_STATE_DISABLED | LV_STATE_CHECKED, LV_OPA_50);

    // Note: bg_click is empty.
    style_init_reset(&styles->bg_click); // it is empty
}

static void cont_init(void)
{
#if LV_USE_CONT != 0
    style_init_reset(&styles->cont_bg);
    lv_style_copy(&styles->cont_bg, &styles->bg);
    lv_style_set_bg_color(&styles->cont_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[36], 0, 6, 38));
    lv_style_set_bg_grad_dir(&styles->cont_bg, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
    lv_style_set_radius(&styles->cont_bg, LV_STATE_DEFAULT, LV_DPI / 20);
#endif
}

static void btn_init(void)
{
#if LV_USE_BTN != 0
    style_init_reset(&btn_rel);
    lv_style_copy(&btn_rel, &def);
    lv_style_set_bg_color(&btn_rel, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 40));
    lv_style_set_bg_grad_color(&btn_rel, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 20));
    lv_style_set_border_color(&btn_rel, LV_STATE_DEFAULT, theme_colors[25]);
    lv_style_set_border_width(&btn_rel, LV_STATE_DEFAULT, 1);
    lv_style_set_border_opa(&btn_rel, LV_STATE_DEFAULT, LV_OPA_70);
    lv_style_set_pad_left(&btn_rel, LV_STATE_DEFAULT, LV_DPI / 4);
    lv_style_set_pad_right(&btn_rel, LV_STATE_DEFAULT, LV_DPI / 4);
    lv_style_set_pad_top(&btn_rel, LV_STATE_DEFAULT, LV_DPI / 8);
    lv_style_set_pad_bottom(&btn_rel, LV_STATE_DEFAULT, LV_DPI / 8);
    // btn_rel.body.shadow.type    = LV_SHADOW_BOTTOM;
    lv_style_set_shadow_color(&btn_rel, LV_STATE_DEFAULT, theme_colors[25]);
    lv_style_set_shadow_width(&btn_rel, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_text_color(&btn_rel, LV_STATE_DEFAULT, theme_colors[34]);
    lv_style_set_image_recolor(&btn_rel, LV_STATE_DEFAULT, theme_colors[34]);

    style_init_reset(&styles->btn);
    lv_style_copy(&styles->btn, &btn_rel);

    lv_style_set_bg_color(&styles->btn, LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 10, 30));
    lv_style_set_bg_grad_color(&styles->btn, LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 10, 10));

    lv_style_set_bg_color(&styles->btn, LV_STATE_CHECKED, recalculate_hsv(theme_colors[0], 0, 10, 20));
    lv_style_set_bg_grad_color(&styles->btn, LV_STATE_CHECKED, recalculate_hsv(theme_colors[0], 0, 10, 40));
    lv_style_set_shadow_width(&styles->btn, LV_STATE_CHECKED, LV_DPI / 20);
    lv_style_set_text_color(&styles->btn, LV_STATE_CHECKED, theme_colors[34]);
    lv_style_set_image_recolor(&styles->btn, LV_STATE_CHECKED, theme_colors[34]);

    lv_style_set_bg_color(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 10, 10));
    lv_style_set_bg_grad_color(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 10, 30));
    lv_style_set_shadow_width(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, LV_DPI / 30);
    lv_style_set_text_color(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, theme_colors[34]);
    lv_style_set_image_recolor(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, theme_colors[34]);

    lv_style_set_bg_color(&styles->btn, LV_STATE_DISABLED, recalculate_hsv(theme_colors[0], 0, 10, 20));
    lv_style_set_bg_grad_color(&styles->btn, LV_STATE_DISABLED, recalculate_hsv(theme_colors[0], 0, 10, 20));
    lv_style_set_shadow_width(&styles->btn, LV_STATE_DISABLED, 0);
    lv_style_set_text_color(&styles->btn, LV_STATE_DISABLED, theme_colors[32]);
    lv_style_set_image_recolor(&styles->btn, LV_STATE_DISABLED, theme_colors[32]);

#endif
}

static void label_init(void)
{
#if LV_USE_LABEL != 0

#endif
}

static void bar_init(void)
{
#if LV_USE_BAR
    uint8_t sv_ratio = lv_color_to_hsv(theme_colors[37]).v;
    style_init_reset(&styles->bar_bg);
    lv_style_copy(&styles->bar_bg, &panel);
    lv_style_set_pad_left(&styles->bar_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&styles->bar_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&styles->bar_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&styles->bar_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_radius(&styles->bar_bg, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    // 86Duino
    lv_style_set_line_color(&styles->bar_bg, LV_STATE_DEFAULT, theme_colors[26]);
    lv_style_set_bg_color(&styles->bar_bg, LV_STATE_DISABLED, lv_color_mix(theme_colors[26], COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_grad_color(&styles->bar_bg, LV_STATE_DISABLED, lv_color_mix(theme_colors[26], COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_border_width(&styles->bar_bg, LV_STATE_DISABLED, 0);
    // lv_style_set_border_color(&styles->bar_bg, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[0], 0, 10, 25), COLOR_MY_GREY, LV_OPA_50));

    style_init_reset(&styles->bar_indic);
    lv_style_copy(&styles->bar_indic, &def);
    lv_style_set_bg_color(&styles->bar_indic, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], sv_ratio, 80, 70));
    lv_style_set_bg_grad_color(&styles->bar_indic, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], sv_ratio, 80, 70));
    lv_style_set_border_color(&styles->bar_indic, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 20, 15));
    lv_style_set_border_width(&styles->bar_indic, LV_STATE_DEFAULT, 1);
    lv_style_set_border_opa(&styles->bar_indic, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_radius(&styles->bar_indic, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_pad_left(&styles->bar_indic, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&styles->bar_indic, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&styles->bar_indic, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&styles->bar_indic, LV_STATE_DEFAULT, 0);
    lv_style_set_text_color(&styles->bar_indic, LV_STATE_DEFAULT, theme_colors[34]);
    lv_style_set_value_color(&styles->bar_indic, LV_STATE_DEFAULT, theme_colors[34]);
    lv_style_set_image_recolor(&styles->bar_indic, LV_STATE_DEFAULT, theme_colors[34]);
    // 86Duino
    lv_style_set_bg_color(&styles->bar_indic, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[0], 0, 80, 70), COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_color(&styles->bar_indic, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[0], 0, 80, 70), COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_grad_color(&styles->bar_indic, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[0], 0, 80, 70), COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_grad_color(&styles->bar_indic, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[0], 0, 80, 70), COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_border_width(&styles->bar_indic, LV_STATE_DISABLED, 0);
    // lv_style_set_border_color(&styles->bar_indic, LV_STATE_DISABLED, COLOR_BTN_BORDER_INA);
#endif
}

static void img_init(void)
{
#if LV_USE_IMG != 0

#endif
}

static void line_init(void)
{
#if LV_USE_LINE != 0

#endif
}

static void led_init(void)
{
#if LV_USE_LED != 0
    uint8_t sv_ratio = lv_color_to_hsv(theme_colors[37]).v;
    uint8_t grad_sv_ratio = lv_color_to_hsv(theme_colors[38]).v;
    style_init_reset(&styles->led);
    lv_style_copy(&styles->led, &def);
    lv_style_set_shadow_width(&styles->led, LV_STATE_DEFAULT, LV_DPX(15));
    lv_style_set_radius(&styles->led, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_border_width(&styles->led, LV_STATE_DEFAULT, 2);
    lv_style_set_border_opa(&styles->led, LV_STATE_DEFAULT, LV_OPA_30);
    lv_style_set_bg_color(&styles->led, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], sv_ratio, 100, 100));
    lv_style_set_bg_grad_color(&styles->led, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], grad_sv_ratio, 100, 40));
    lv_style_set_border_color(&styles->led, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], sv_ratio, 60, 60));
    lv_style_set_shadow_color(&styles->led, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], sv_ratio, 100, 100));
    lv_style_set_shadow_spread(&styles->led, LV_STATE_DEFAULT, LV_DPX(6));
    lv_style_set_text_font(&styles->led, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->led, LV_STATE_DEFAULT, theme_fonts[0]);
#endif
}

static void slider_init(void)
{
#if LV_USE_SLIDER != 0
    style_init_reset(&styles->slider_knob);
    lv_style_copy(&styles->slider_knob, &def);
    lv_style_set_bg_color(&styles->slider_knob, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 40));
    lv_style_set_bg_grad_color(&styles->slider_knob, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 20));
    lv_style_set_border_color(&styles->slider_knob, LV_STATE_DEFAULT, theme_colors[25]);
    lv_style_set_border_width(&styles->slider_knob, LV_STATE_DEFAULT, 1);
    lv_style_set_border_opa(&styles->slider_knob, LV_STATE_DEFAULT, LV_OPA_70);
    lv_style_set_pad_left(&styles->slider_knob, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_pad_right(&styles->slider_knob, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_pad_bottom(&styles->slider_knob, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_pad_top(&styles->slider_knob, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_shadow_color(&styles->slider_knob, LV_STATE_DEFAULT, theme_colors[25]);
    lv_style_set_shadow_width(&styles->slider_knob, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_text_color(&styles->slider_knob, LV_STATE_DEFAULT, theme_colors[34]);
    lv_style_set_value_color(&styles->slider_knob, LV_STATE_DEFAULT, theme_colors[34]);
    lv_style_set_image_recolor(&styles->slider_knob, LV_STATE_DEFAULT, theme_colors[34]);
    lv_style_set_radius(&styles->slider_knob, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    // 86Duino
    lv_style_set_bg_color(&styles->slider_knob, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[0], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_color(&styles->slider_knob, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[0], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_grad_color(&styles->slider_knob, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[0], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_grad_color(&styles->slider_knob, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[0], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_shadow_width(&styles->slider_knob, LV_STATE_DISABLED, 0);
    lv_style_set_border_color(&styles->slider_knob, LV_STATE_DISABLED, lv_color_mix(theme_colors[25], COLOR_MY_GREY, LV_OPA_50));

    style_init_reset(&styles->slider_bg);
    lv_style_copy(&styles->slider_bg, &styles->bar_bg);
    lv_style_set_value_color(&styles->slider_bg, LV_STATE_DISABLED, lv_color_mix(theme_colors[32], COLOR_MY_GREY, LV_OPA_50));
#endif
}

static void switch_init(void)
{
#if LV_USE_SWITCH != 0
    style_init_reset(&styles->sw_knob);
    lv_style_copy(&styles->sw_knob, &styles->slider_knob);
    lv_style_set_pad_left(&styles->sw_knob, LV_STATE_DEFAULT, LV_DPX(6));
    lv_style_set_pad_right(&styles->sw_knob, LV_STATE_DEFAULT, LV_DPX(6));
    lv_style_set_pad_bottom(&styles->sw_knob, LV_STATE_DEFAULT, LV_DPX(6));
    lv_style_set_pad_top(&styles->sw_knob, LV_STATE_DEFAULT, LV_DPX(6));
#endif
}

static void linemeter_init(void)
{
#if LV_USE_LINEMETER != 0
    style_init_reset(&styles->lmeter);
    lv_style_copy(&styles->lmeter, &styles->bg);
    lv_style_set_radius(&styles->lmeter, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_pad_left(&styles->lmeter, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_pad_right(&styles->lmeter, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_pad_top(&styles->lmeter, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_pad_inner(&styles->lmeter, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_scale_width(&styles->lmeter, LV_STATE_DEFAULT, LV_DPX(15));

    lv_style_set_line_color(&styles->lmeter, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 70));
    lv_style_set_scale_grad_color(&styles->lmeter, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 95, 90));
    lv_style_set_scale_end_color(&styles->lmeter, LV_STATE_DEFAULT, theme_colors[29]);
    lv_style_set_line_width(&styles->lmeter, LV_STATE_DEFAULT, LV_DPX(2));
    lv_style_set_scale_end_line_width(&styles->lmeter, LV_STATE_DEFAULT, LV_DPX(2));
    lv_style_set_text_color(&styles->lmeter, LV_STATE_DEFAULT, theme_colors[34]);

    // lv_style_set_bg_color			(&styles->lmeter, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 70));
    // lv_style_set_bg_grad_color		(&styles->lmeter, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 95, 90));
    // lv_style_set_pad_left			(&styles->lmeter, LV_STATE_DEFAULT, LV_DPI / 10);
    // lv_style_set_pad_inner			(&styles->lmeter, LV_STATE_DEFAULT, LV_DPI / 10);
    // lv_style_set_border_color		(&styles->lmeter, LV_STATE_DEFAULT, lv_color_hex3(0x333));
    // lv_style_set_line_color			(&styles->lmeter, LV_STATE_DEFAULT, lv_color_hex3(0x555));
    // lv_style_set_line_width 		    (&styles->lmeter, LV_STATE_DEFAULT, 1);
#endif
}

static void gauge_init(void)
{
#if LV_USE_GAUGE != 0
    style_init_reset(&styles->gauge_main);
    lv_style_copy(&styles->gauge_main, &def);
    lv_style_set_bg_color(&styles->gauge_main, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[35], 0, 6, 38));
    lv_style_set_bg_grad_color(&styles->gauge_main, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[35], 0, 6, 38));
    lv_style_set_line_color(&styles->gauge_main, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 70));
    lv_style_set_scale_grad_color(&styles->gauge_main, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 70));
    lv_style_set_scale_end_color(&styles->gauge_main, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 80, 75));
    lv_style_set_line_width(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(5));
    lv_style_set_scale_end_line_width(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(4));
    lv_style_set_scale_end_border_width(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(8));
    lv_style_set_pad_left(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_pad_right(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_pad_top(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_pad_inner(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_scale_width(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(15));
    lv_style_set_radius(&styles->gauge_main, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_text_color(&styles->gauge_main, LV_STATE_DEFAULT, theme_colors[34]);
    lv_style_set_value_color(&styles->gauge_main, LV_STATE_DEFAULT, theme_colors[34]);

    style_init_reset(&styles->gauge_strong);
    lv_style_copy(&styles->gauge_strong, &def); // check
    lv_style_set_line_color(&styles->gauge_strong, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 70));
    lv_style_set_scale_grad_color(&styles->gauge_strong, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 70));
    lv_style_set_scale_end_color(&styles->gauge_strong, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 80, 75));
    lv_style_set_line_width(&styles->gauge_strong, LV_STATE_DEFAULT, LV_DPX(8));
    lv_style_set_scale_end_line_width(&styles->gauge_strong, LV_STATE_DEFAULT, LV_DPX(8));
    lv_style_set_scale_width(&styles->gauge_strong, LV_STATE_DEFAULT, LV_DPX(25));
    lv_style_set_text_color(&styles->gauge_strong, LV_STATE_DEFAULT, theme_colors[34]);
    lv_style_set_value_color(&styles->gauge_strong, LV_STATE_DEFAULT, theme_colors[34]);

    style_init_reset(&styles->gauge_needle);
    lv_style_copy(&styles->gauge_needle, &styles->gauge_main); // check
    lv_style_set_line_color(&styles->gauge_needle, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_line_width(&styles->gauge_needle, LV_STATE_DEFAULT, LV_DPX(4));
    lv_style_set_bg_opa(&styles->gauge_needle, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->gauge_needle, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 40)); // 86Duino
    lv_style_set_bg_grad_color(&styles->gauge_needle, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 20)); // 86Duino
    lv_style_set_radius(&styles->gauge_needle, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_size(&styles->gauge_needle, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_pad_inner(&styles->gauge_needle, LV_STATE_DEFAULT, LV_DPX(10));
#endif
}

static void arc_init(void)
{
#if LV_USE_ARC != 0
    uint8_t sv_ratio = lv_color_to_hsv(theme_colors[37]).v;
    style_init_reset(&styles->arc_indic);
    // lv_style_copy(&styles->arc_indic, &def); // check
    lv_style_set_line_color(&styles->arc_indic, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], sv_ratio, 80, 70));
    lv_style_set_line_width(&styles->arc_indic, LV_STATE_DEFAULT, LV_DPX(25));
    lv_style_set_line_rounded(&styles->arc_indic, LV_STATE_DEFAULT, true);
    // 86Duino
    lv_style_set_line_color(&styles->arc_indic, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[0], sv_ratio, 80, 70), COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_line_color(&styles->arc_indic, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[0], sv_ratio, 80, 70), COLOR_MY_GREY, LV_OPA_50));

    style_init_reset(&styles->arc_bg);
    lv_style_copy(&styles->arc_bg, &styles->bg); // check
    // lv_style_set_line_color(&styles->arc_bg, LV_STATE_DEFAULT, COLOR_BG_SEC);
    lv_style_set_line_width(&styles->arc_bg, LV_STATE_DEFAULT, LV_DPX(25));
    lv_style_set_line_rounded(&styles->arc_bg, LV_STATE_DEFAULT, true);
    lv_style_set_text_font(&styles->arc_bg, LV_STATE_DEFAULT, theme_fonts[1]);
    lv_style_set_value_font(&styles->arc_bg, LV_STATE_DEFAULT, theme_fonts[1]);
    // 86Duino
    lv_style_set_line_color(&styles->arc_bg, LV_STATE_DISABLED, lv_color_mix(theme_colors[26], COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_value_color(&styles->arc_bg, LV_STATE_DISABLED, lv_color_mix(theme_colors[32], COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_color(&styles->arc_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[36], 0, 6, 38));
    lv_style_set_bg_grad_dir(&styles->arc_bg, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
    lv_style_set_radius(&styles->arc_bg, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_left(&styles->arc_bg, LV_STATE_DEFAULT, LV_DPX(30));
    lv_style_set_pad_right(&styles->arc_bg, LV_STATE_DEFAULT, LV_DPX(30));
    lv_style_set_pad_top(&styles->arc_bg, LV_STATE_DEFAULT, LV_DPX(30));
    lv_style_set_pad_bottom(&styles->arc_bg, LV_STATE_DEFAULT, LV_DPX(30));
    lv_style_set_pad_inner(&styles->arc_bg, LV_STATE_DEFAULT, LV_DPX(30));

    style_init_reset(&styles->arc_knob);
    lv_style_copy(&styles->arc_knob, &styles->slider_knob);
    // lv_style_set_radius(&styles->arc_knob, LV_STATE_DEFAULT,   LV_RADIUS_CIRCLE);
    // lv_style_set_pad_top(&styles->arc_knob, LV_STATE_DEFAULT,  LV_DPX(6)); // LV_DPX(6) is switch knob size
    // lv_style_set_pad_bottom(&styles->arc_knob, LV_STATE_DEFAULT,  LV_DPX(6));
    // lv_style_set_pad_left(&styles->arc_knob, LV_STATE_DEFAULT,    LV_DPX(6));
    // lv_style_set_pad_right(&styles->arc_knob, LV_STATE_DEFAULT,   LV_DPX(6));

#endif
}

static void spinner_init(void)
{
#if LV_USE_SPINNER != 0
#endif
}

static void chart_init(void)
{
#if LV_USE_CHART
    style_init_reset(&styles->chart_bg);
    lv_style_copy(&styles->chart_bg, &styles->bg);
    lv_style_set_bg_color(&styles->chart_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[36], 0, 6, 38));
    lv_style_set_bg_grad_dir(&styles->chart_bg, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
    lv_style_set_radius(&styles->chart_bg, LV_STATE_DEFAULT, LV_DPI / 20);
    // lv_style_set_border_width(&styles->chart_bg, LV_STATE_DEFAULT, LV_DPX(2));
    lv_style_set_text_font(&styles->chart_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->chart_bg, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->chart_series_bg);
    lv_style_copy(&styles->chart_series_bg, &styles->bg); // we use v.7 setting as below
    lv_style_set_bg_opa(&styles->chart_series_bg, LV_STATE_DEFAULT, LV_OPA_TRANSP); // 86Duino
    lv_style_set_line_width(&styles->chart_series_bg, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_line_dash_width(&styles->chart_series_bg, LV_STATE_DEFAULT, LV_DPX(10));
    lv_style_set_line_dash_gap(&styles->chart_series_bg, LV_STATE_DEFAULT, LV_DPX(10));
    lv_style_set_line_color(&styles->chart_series_bg, LV_STATE_DEFAULT, theme_colors[24]);
    lv_style_set_text_font(&styles->chart_series_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->chart_series_bg, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->chart_series);
    lv_style_copy(&styles->chart_series, &styles->bg); // we use v.7 setting as below 
    lv_style_set_line_width(&styles->chart_series, LV_STATE_DEFAULT, LV_DPX(3));
    lv_style_set_size(&styles->chart_series, LV_STATE_DEFAULT, LV_DPX(4));
    lv_style_set_pad_inner(&styles->chart_series, LV_STATE_DEFAULT, LV_DPX(2));     /*Space between columns*/
    lv_style_set_radius(&styles->chart_series, LV_STATE_DEFAULT, LV_DPX(1));

#endif
}

static void calendar_init(void)
{
#if LV_USE_CALENDAR
    style_init_reset(&styles->calendar_bg); // here use the second bg 
    lv_style_copy(&styles->calendar_bg, &styles->bg);
    lv_style_set_bg_color(&styles->calendar_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 40));
    lv_style_set_bg_grad_color(&styles->calendar_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 40));
    lv_style_set_border_color(&styles->calendar_bg, LV_STATE_DEFAULT, theme_colors[28]);
    lv_style_set_border_width(&styles->calendar_bg, LV_STATE_DEFAULT, 1);
    lv_style_set_radius(&styles->calendar_bg, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_left(&styles->calendar_bg, LV_STATE_DEFAULT, LV_DPI / 10);
    lv_style_set_pad_right(&styles->calendar_bg, LV_STATE_DEFAULT, LV_DPI / 10);
    lv_style_set_pad_top(&styles->calendar_bg, LV_STATE_DEFAULT, LV_DPI / 10);
    lv_style_set_pad_bottom(&styles->calendar_bg, LV_STATE_DEFAULT, LV_DPI / 10);

    style_init_reset(&styles->calendar_header);
    lv_style_copy(&styles->calendar_header, &styles->bg);
    lv_style_set_bg_color(&styles->calendar_header, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 20));
    lv_style_set_bg_grad_color(&styles->calendar_header, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 20));
    lv_style_set_radius(&styles->calendar_header, LV_STATE_DEFAULT, 0);
    lv_style_set_border_width(&styles->calendar_header, LV_STATE_DEFAULT, 1);
    lv_style_set_border_color(&styles->calendar_header, LV_STATE_DEFAULT, theme_colors[28]);
    lv_style_set_pad_left(&styles->calendar_header, LV_STATE_DEFAULT, LV_DPI / 10);
    lv_style_set_pad_right(&styles->calendar_header, LV_STATE_DEFAULT, LV_DPI / 10);
    lv_style_set_pad_top(&styles->calendar_header, LV_STATE_DEFAULT, LV_DPI / 10);
    lv_style_set_pad_bottom(&styles->calendar_header, LV_STATE_DEFAULT, LV_DPI / 10);

    style_init_reset(&styles->calendar_daynames); // week
    lv_style_copy(&styles->calendar_daynames, &panel);
    lv_style_set_pad_left(&styles->calendar_daynames, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_right(&styles->calendar_daynames, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_bottom(&styles->calendar_daynames, LV_STATE_DEFAULT, LV_DPI / 25);
    lv_style_set_text_font(&styles->calendar_daynames, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->calendar_daynames, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->calendar_date_nums);
    lv_style_copy(&styles->calendar_date_nums, &styles->bg);
    lv_style_set_bg_color(&styles->calendar_date_nums, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 40));
    lv_style_set_bg_grad_color(&styles->calendar_date_nums, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 40));
    lv_style_set_bg_color(&styles->calendar_date_nums, LV_STATE_CHECKED, recalculate_hsv(theme_colors[0], 0, 80, 70));
    lv_style_set_bg_color(&styles->calendar_date_nums, LV_STATE_FOCUSED, recalculate_hsv(theme_colors[0], 0, 80, 70));
    lv_style_set_radius(&styles->calendar_date_nums, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_pad_left(&styles->calendar_date_nums, LV_STATE_DEFAULT, LV_DPI / 14);
    lv_style_set_pad_right(&styles->calendar_date_nums, LV_STATE_DEFAULT, LV_DPI / 14);
    lv_style_set_pad_top(&styles->calendar_date_nums, LV_STATE_DEFAULT, LV_DPI / 14);
    lv_style_set_pad_bottom(&styles->calendar_date_nums, LV_STATE_DEFAULT, LV_DPI / 14);
    lv_style_set_bg_opa(&styles->calendar_date_nums, LV_STATE_CHECKED, LV_OPA_40);
    lv_style_set_bg_opa(&styles->calendar_date_nums, LV_STATE_PRESSED, LV_OPA_20);
    lv_style_set_bg_opa(&styles->calendar_date_nums, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_style_set_text_color(&styles->calendar_date_nums, LV_STATE_CHECKED, recalculate_hsv(theme_colors[0], 0, 40, 80));
    lv_style_set_text_color(&styles->calendar_date_nums, LV_STATE_DISABLED, recalculate_hsv(theme_colors[0], 0, 0, 60));
    lv_style_set_text_color(&styles->calendar_date_nums, LV_STATE_FOCUSED, theme_colors[35]);
    lv_style_set_text_font(&styles->calendar_date_nums, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->calendar_date_nums, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_border_width(&styles->calendar_date_nums, LV_STATE_CHECKED, 2);
    lv_style_set_border_side(&styles->calendar_date_nums, LV_STATE_CHECKED, LV_BORDER_SIDE_LEFT);
    lv_style_set_border_color(&styles->calendar_date_nums, LV_STATE_CHECKED, recalculate_hsv(theme_colors[0], 0, 80, 70));
    lv_style_set_pad_inner(&styles->calendar_date_nums, LV_STATE_DEFAULT, LV_DPX(3));
#endif
}

static void cpicker_init(void) // v.6 have not this object, so we directly use v.7
{
#if LV_USE_CPICKER
    style_init_reset(&styles->cpicker_bg);
    lv_style_set_scale_width(&styles->cpicker_bg, LV_STATE_DEFAULT, LV_DPX(30));
    lv_style_set_bg_opa(&styles->cpicker_bg, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->cpicker_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 11, 30));
    lv_style_set_pad_inner(&styles->cpicker_bg, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_radius(&styles->cpicker_bg, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_text_font(&styles->cpicker_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->cpicker_bg, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->cpicker_indic);
    lv_style_set_radius(&styles->cpicker_indic, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_bg_color(&styles->cpicker_indic, LV_STATE_DEFAULT, theme_colors[24]);
    lv_style_set_bg_opa(&styles->cpicker_indic, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_border_width(&styles->cpicker_indic, LV_STATE_DEFAULT, 2);
    lv_style_set_border_color(&styles->cpicker_indic, LV_STATE_DEFAULT, theme_colors[31]);
    lv_style_set_border_color(&styles->cpicker_indic, LV_STATE_FOCUSED, theme_colors[24]);
    lv_style_set_border_color(&styles->cpicker_indic, LV_STATE_EDITED, theme_colors[25]);
    lv_style_set_pad_left(&styles->cpicker_indic, LV_STATE_DEFAULT, LV_DPX(13));
    lv_style_set_pad_right(&styles->cpicker_indic, LV_STATE_DEFAULT, LV_DPX(13));
    lv_style_set_pad_top(&styles->cpicker_indic, LV_STATE_DEFAULT, LV_DPX(13));
    lv_style_set_pad_bottom(&styles->cpicker_indic, LV_STATE_DEFAULT, LV_DPX(13));
    lv_style_set_text_font(&styles->cpicker_indic, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->cpicker_indic, LV_STATE_DEFAULT, theme_fonts[0]);
#endif
}

static void checkbox_init(void)
{
#if LV_USE_CHECKBOX != 0
    uint8_t sv_ratio = lv_color_to_hsv(theme_colors[37]).v;
    style_init_reset(&styles->cb_bg);
    lv_style_copy(&styles->cb_bg, &styles->bg);
    lv_style_set_bg_opa(&styles->cb_bg, LV_STATE_DEFAULT, LV_OPA_0);
    lv_style_set_text_color(&styles->cb_bg, LV_STATE_DISABLED, lv_color_mix(theme_colors[32], COLOR_MY_GREY, LV_OPA_50));

    style_init_reset(&styles->cb_bullet); // v.6 have not this object, so we directly use v.7
    lv_style_copy(&styles->cb_bullet, &def);
    lv_style_set_pad_left(&styles->cb_bullet, LV_STATE_DEFAULT, LV_DPX(3));
    lv_style_set_pad_right(&styles->cb_bullet, LV_STATE_DEFAULT, LV_DPX(3));
    lv_style_set_pad_top(&styles->cb_bullet, LV_STATE_DEFAULT, LV_DPX(3));
    lv_style_set_pad_bottom(&styles->cb_bullet, LV_STATE_DEFAULT, LV_DPX(3));
    lv_style_set_pad_inner(&styles->cb_bullet, LV_STATE_DEFAULT, LV_DPI / 40);

    lv_style_set_radius(&styles->cb_bullet, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_bg_color(&styles->cb_bullet, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 95));
    lv_style_set_bg_grad_color(&styles->cb_bullet, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 95));
    lv_style_set_border_color(&styles->cb_bullet, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 50));
    lv_style_set_border_width(&styles->cb_bullet, LV_STATE_DEFAULT, 2);
    lv_style_set_text_font(&styles->cb_bullet, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->cb_bullet, LV_STATE_DEFAULT, theme_fonts[0]);

    lv_style_set_bg_color(&styles->cb_bullet, LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 10, 80));
    lv_style_set_bg_grad_color(&styles->cb_bullet, LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 10, 80));
    lv_style_set_border_color(&styles->cb_bullet, LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 10, 20));
    lv_style_set_border_width(&styles->cb_bullet, LV_STATE_PRESSED, 1);

    lv_style_set_bg_color(&styles->cb_bullet, LV_STATE_CHECKED, recalculate_hsv(theme_colors[0], sv_ratio, 80, 90));
    lv_style_set_bg_grad_color(&styles->cb_bullet, LV_STATE_CHECKED, recalculate_hsv(theme_colors[0], sv_ratio, 80, 90));
    lv_style_set_border_color(&styles->cb_bullet, LV_STATE_CHECKED, recalculate_hsv(theme_colors[0], 0, 80, 50));

    lv_style_set_bg_color(&styles->cb_bullet, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], sv_ratio, 80, 70));
    lv_style_set_bg_grad_color(&styles->cb_bullet, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], sv_ratio, 80, 70));
    lv_style_set_border_color(&styles->cb_bullet, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 80, 30));
    lv_style_set_border_width(&styles->cb_bullet, LV_STATE_CHECKED | LV_STATE_PRESSED, 1);

    // 86Duino
    lv_style_set_bg_color(&styles->cb_bullet, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[0], 0, 10, 95), COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_grad_color(&styles->cb_bullet, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[0], 0, 10, 95), COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_color(&styles->cb_bullet, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[0], 0, 80, 90), COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_grad_color(&styles->cb_bullet, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[0], 0, 80, 90), COLOR_MY_GREY, LV_OPA_50));
    
    // ORIGINAL:
    // lv_style_set_bg_color			(&styles->cb_bullet, LV_STATE_DISABLED, lv_color_hex3(0x777));
    // lv_style_set_bg_grad_color		(&styles->cb_bullet, LV_STATE_DISABLED, lv_color_hex3(0x777));
    lv_style_set_border_width(&styles->cb_bullet, LV_STATE_DISABLED, 0);
#endif
}

static void btnmatrix_init(void)
{
    style_init_reset(&styles->btnm_bg);
    lv_style_copy(&styles->btnm_bg, &btn_rel);
    lv_style_set_pad_left(&styles->btnm_bg, LV_STATE_DEFAULT, 1);
    lv_style_set_pad_right(&styles->btnm_bg, LV_STATE_DEFAULT, 1);
    lv_style_set_pad_top(&styles->btnm_bg, LV_STATE_DEFAULT, 1);
    lv_style_set_pad_bottom(&styles->btnm_bg, LV_STATE_DEFAULT, 1);
    lv_style_set_pad_inner(&styles->btnm_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_border_width(&styles->btnm_bg, LV_STATE_DEFAULT, 1);

    style_init_reset(&styles->btnm_btn);
    lv_style_copy(&styles->btnm_btn, &styles->btn);
    lv_style_set_pad_left(&styles->btnm_btn, LV_STATE_DEFAULT, 2);
    lv_style_set_pad_right(&styles->btnm_btn, LV_STATE_DEFAULT, 2);
    lv_style_set_pad_top(&styles->btnm_btn, LV_STATE_DEFAULT, 2);
    lv_style_set_pad_bottom(&styles->btnm_btn, LV_STATE_DEFAULT, 2);
    lv_style_set_pad_inner(&styles->btnm_btn, LV_STATE_DEFAULT, 0);
    lv_style_set_border_width(&styles->btnm_btn, LV_STATE_DEFAULT, 1);
    lv_style_set_border_side(&styles->btnm_btn, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL | LV_BORDER_SIDE_INTERNAL);
    lv_style_set_radius(&styles->btnm_btn, LV_STATE_DEFAULT, 3);

    lv_style_set_border_side(&styles->btnm_btn, LV_STATE_PRESSED, LV_BORDER_SIDE_FULL | LV_BORDER_SIDE_INTERNAL);
    lv_style_set_border_width(&styles->btnm_btn, LV_STATE_PRESSED, 1);
    lv_style_set_radius(&styles->btnm_btn, LV_STATE_PRESSED, 3);

    lv_style_set_border_side(&styles->btnm_btn, LV_STATE_CHECKED, LV_BORDER_SIDE_FULL | LV_BORDER_SIDE_INTERNAL);
    lv_style_set_border_width(&styles->btnm_btn, LV_STATE_CHECKED, 1);
    lv_style_set_radius(&styles->btnm_btn, LV_STATE_CHECKED, 3);

    lv_style_set_bg_color(&styles->btnm_btn, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 10, 30));
    lv_style_set_bg_grad_color(&styles->btnm_btn, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 10, 10));
    lv_style_set_border_side(&styles->btnm_btn, LV_STATE_CHECKED | LV_STATE_PRESSED, LV_BORDER_SIDE_FULL | LV_BORDER_SIDE_INTERNAL);
    lv_style_set_border_width(&styles->btnm_btn, LV_STATE_CHECKED | LV_STATE_PRESSED, 1);
    lv_style_set_radius(&styles->btnm_btn, LV_STATE_CHECKED | LV_STATE_PRESSED, 3);

    lv_style_set_border_side(&styles->btnm_btn, LV_STATE_DISABLED, LV_BORDER_SIDE_FULL | LV_BORDER_SIDE_INTERNAL);
    lv_style_set_border_width(&styles->btnm_btn, LV_STATE_DISABLED, 1);
    lv_style_set_radius(&styles->btnm_btn, LV_STATE_DISABLED, 3);
}

static void keyboard_init(void)
{
#if LV_USE_KEYBOARD
    style_init_reset(&styles->kb_bg);
    lv_style_copy(&styles->kb_bg, &styles->bg);

    style_init_reset(&styles->kb_btn);
    lv_style_copy(&styles->kb_btn, &styles->btn);
#endif
}

static void msgbox_init(void)
{
#if LV_USE_MSGBOX
    style_init_reset(&styles->mbox_bg);
    lv_style_copy(&styles->mbox_bg, &styles->bg);
    lv_style_set_bg_color(&styles->mbox_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 30, 30));
    lv_style_set_bg_grad_color(&styles->mbox_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 30, 30));
    lv_style_set_border_color(&styles->mbox_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 11, 20));
    lv_style_set_border_width(&styles->mbox_bg, LV_STATE_DEFAULT, 1);
    lv_style_set_shadow_width(&styles->mbox_bg, LV_STATE_DEFAULT, LV_DPX(40));
    lv_style_set_shadow_color(&styles->mbox_bg, LV_STATE_DEFAULT, theme_colors[25]);
    lv_style_set_radius(&styles->mbox_bg, LV_STATE_DEFAULT, LV_DPI / 20);

    style_init_reset(&styles->mbox_btn_bg);
    lv_style_copy(&styles->mbox_btn_bg, &lv_style_transp);

    style_init_reset(&styles->mbox_btn);
    lv_style_copy(&styles->mbox_btn, &styles->btn);
#endif
}

static void page_init(void)
{
#if LV_USE_PAGE
    style_init_reset(&styles->page_bg);
    lv_style_copy(&styles->page_bg, &panel);

    style_init_reset(&styles->pad_inner); // v.7 have not "page_scrl" style, we take "pad_inner" instead
    lv_style_copy(&styles->pad_inner, &styles->bg);
    lv_style_set_bg_color(&styles->pad_inner, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 40));
    lv_style_set_bg_grad_color(&styles->pad_inner, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 40));
    lv_style_set_border_color(&styles->pad_inner, LV_STATE_DEFAULT, theme_colors[28]);
    lv_style_set_border_width(&styles->pad_inner, LV_STATE_DEFAULT, 1);
    lv_style_set_radius(&styles->pad_inner, LV_STATE_DEFAULT, LV_DPI / 20);
    lv_style_set_text_font(&styles->pad_inner, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->pad_inner, LV_STATE_DEFAULT, theme_fonts[0]);

#if  LV_USE_ANIMATION
    style_init_reset(&styles->edge_flash);
    lv_style_set_bg_opa(&styles->edge_flash, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->edge_flash, LV_STATE_DEFAULT, lv_color_hex3(0x888));
#endif
#endif
}

static void textarea_init(void)
{
#if LV_USE_TEXTAREA
    style_init_reset(&styles->ta_bg);
    lv_style_copy(&styles->ta_bg, &panel);
    lv_style_set_bg_color(&styles->ta_bg, LV_STATE_DISABLED, recalculate_hsv(theme_colors[36], 0, 6, 38));
    lv_style_set_bg_grad_dir(&styles->ta_bg, LV_STATE_DISABLED, LV_GRAD_DIR_NONE);
    // lv_style_set_bg_color(&styles->ta_bg, LV_STATE_DISABLED, lv_color_mix(theme_colors[26], COLOR_MY_GREY, LV_OPA_50));
    // lv_style_set_bg_grad_color(&styles->ta_bg, LV_STATE_DISABLED, lv_color_mix(theme_colors[26], COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_text_color(&styles->ta_bg, LV_STATE_DISABLED, lv_color_mix(theme_colors[32], COLOR_MY_GREY, LV_OPA_50));

    style_init_reset(&styles->ta_cursor);
    //lv_style_copy(&styles->ta_cursor, &panel);
    lv_style_set_border_width(&styles->ta_cursor, LV_STATE_DEFAULT, LV_DPX(2));
    lv_style_set_pad_left(&styles->ta_cursor, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_border_side(&styles->ta_cursor, LV_STATE_DEFAULT, LV_BORDER_SIDE_LEFT);
    lv_style_set_text_font(&styles->ta_cursor, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->ta_cursor, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->ta_placeholder);
    //lv_style_copy(&styles->ta_placeholder, &panel);
    lv_style_set_text_font(&styles->ta_placeholder, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->ta_placeholder, LV_STATE_DEFAULT, theme_fonts[0]);
#endif
}

static void spinbox_init(void)
{
#if LV_USE_SPINBOX
    style_init_reset(&styles->spinbox_bg);
    lv_style_copy(&styles->spinbox_bg, &styles->bg);
    lv_style_set_text_color(&styles->spinbox_bg, LV_STATE_DISABLED, lv_color_mix(theme_colors[32], COLOR_MY_GREY, LV_OPA_50));

    style_init_reset(&styles->spinbox_cursor);
    lv_style_copy(&styles->spinbox_cursor, &panel);

    style_init_reset(&styles->spinbox_btn);
    lv_style_copy(&styles->spinbox_btn, &styles->btn);
#endif
}

static void list_init(void)
{
#if LV_USE_LIST != 0
    style_init_reset(&styles->list_bg);
    lv_style_copy(&styles->list_bg, &panel);
    lv_style_set_pad_top(&styles->list_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&styles->list_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_left(&styles->list_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&styles->list_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_inner(&styles->list_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_text_font(&styles->list_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->list_bg, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->list_btn);
    lv_style_copy(&styles->list_btn, &styles->bg);
    lv_style_set_bg_opa(&styles->list_btn, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_border_side(&styles->list_btn, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_color(&styles->list_btn, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 5));
    lv_style_set_border_width(&styles->list_btn, LV_STATE_DEFAULT, 1);
    lv_style_set_radius(&styles->list_btn, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_text_color(&styles->list_btn, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 5, 80));
    lv_style_set_image_recolor(&styles->list_btn, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 5, 80));
    lv_style_set_pad_top(&styles->list_btn, LV_STATE_DEFAULT, LV_DPI / 6);
    lv_style_set_pad_bottom(&styles->list_btn, LV_STATE_DEFAULT, LV_DPI / 6);
    lv_style_set_pad_left(&styles->list_btn, LV_STATE_DEFAULT, LV_DPI / 8);
    lv_style_set_pad_right(&styles->list_btn, LV_STATE_DEFAULT, LV_DPI / 8);

    lv_style_set_bg_opa(&styles->list_btn, LV_STATE_PRESSED, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->list_btn, LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 10, 5));
    lv_style_set_bg_grad_color(&styles->list_btn, LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 10, 5));
    lv_style_set_border_color(&styles->list_btn, LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 10, 5));
    lv_style_set_border_width(&styles->list_btn, LV_STATE_PRESSED, 0);
    lv_style_set_pad_top(&styles->list_btn, LV_STATE_PRESSED, LV_DPI / 6);
    lv_style_set_pad_bottom(&styles->list_btn, LV_STATE_PRESSED, LV_DPI / 6);
    lv_style_set_pad_left(&styles->list_btn, LV_STATE_PRESSED, LV_DPI / 8);
    lv_style_set_pad_right(&styles->list_btn, LV_STATE_PRESSED, LV_DPI / 8);
    lv_style_set_text_color(&styles->list_btn, LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 5, 80));
    lv_style_set_image_recolor(&styles->list_btn, LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 5, 80));

    lv_style_set_bg_opa(&styles->list_btn, LV_STATE_CHECKED, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->list_btn, LV_STATE_CHECKED, recalculate_hsv(theme_colors[0], 0, 80, 70));
    lv_style_set_bg_grad_color(&styles->list_btn, LV_STATE_CHECKED, recalculate_hsv(theme_colors[0], 0, 80, 70));
    lv_style_set_border_color(&styles->list_btn, LV_STATE_CHECKED, recalculate_hsv(theme_colors[0], 0, 60, 40));
    lv_style_set_radius(&styles->list_btn, LV_STATE_CHECKED, LV_DPX(7));

    lv_style_set_bg_color(&styles->list_btn, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 80, 60));
    lv_style_set_bg_grad_color(&styles->list_btn, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 80, 60));

    lv_style_set_border_color(&styles->list_btn, LV_STATE_FOCUSED, theme_colors[0]);
    lv_style_set_border_side(&styles->list_btn, LV_STATE_FOCUSED, LV_BORDER_SIDE_FULL);

    lv_style_set_outline_color(&styles->list_btn, LV_STATE_FOCUSED, theme_colors[0]);
    lv_style_set_outline_width(&styles->list_btn, LV_STATE_FOCUSED, LV_DPX(1));

    lv_style_set_bg_color(&styles->list_btn, LV_STATE_DISABLED, recalculate_hsv(theme_colors[0], 0, 10, 20));
    lv_style_set_bg_grad_color(&styles->list_btn, LV_STATE_DISABLED, recalculate_hsv(theme_colors[0], 0, 10, 20));
    lv_style_set_shadow_width(&styles->list_btn, LV_STATE_DISABLED, 0);
    lv_style_set_text_color(&styles->list_btn, LV_STATE_DISABLED, theme_colors[32]);

    lv_style_set_text_font(&styles->list_btn, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->list_btn, LV_STATE_DEFAULT, theme_fonts[0]);
#endif
}

static void ddlist_init(void)
{
#if LV_USE_DROPDOWN != 0
    style_init_reset(&styles->ddlist_main);
    lv_style_copy(&styles->ddlist_main, &btn_rel);
    lv_style_set_text_line_space(&styles->ddlist_main, LV_STATE_DEFAULT, LV_DPI / 8);
    lv_style_set_pad_top(&styles->ddlist_main, LV_STATE_DEFAULT, LV_DPI / 8);
    lv_style_set_pad_bottom(&styles->ddlist_main, LV_STATE_DEFAULT, LV_DPI / 8);
    lv_style_set_pad_left(&styles->ddlist_main, LV_STATE_DEFAULT, LV_DPI / 8);
    lv_style_set_pad_right(&styles->ddlist_main, LV_STATE_DEFAULT, LV_DPI / 8);
    lv_style_set_radius(&styles->ddlist_main, LV_STATE_DEFAULT, LV_DPI / 30);
    lv_style_set_bg_color(&styles->ddlist_main, LV_STATE_DISABLED, recalculate_hsv(theme_colors[36], 0, 6, 38));
    lv_style_set_bg_grad_dir(&styles->ddlist_main, LV_STATE_DISABLED, LV_GRAD_DIR_NONE);
    lv_style_set_text_color(&styles->ddlist_main, LV_STATE_DISABLED, lv_color_mix(theme_colors[32], COLOR_MY_GREY, LV_OPA_50));

    style_init_reset(&styles->ddlist_page);
    lv_style_copy(&styles->ddlist_page, &styles->ddlist_main);
    lv_style_set_text_font(&styles->ddlist_page, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->ddlist_page, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->ddlist_sel);
    lv_style_copy(&styles->ddlist_sel, &btn_rel);
    lv_style_set_bg_color(&styles->ddlist_sel, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 20, 50));
    lv_style_set_bg_grad_color(&styles->ddlist_sel, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 20, 50));
    lv_style_set_radius(&styles->ddlist_sel, LV_STATE_DEFAULT, 0);
    lv_style_set_text_font(&styles->ddlist_sel, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->ddlist_sel, LV_STATE_DEFAULT, theme_fonts[0]);
#endif
}

static void roller_init(void)
{
#if LV_USE_ROLLER != 0
    style_init_reset(&styles->roller_bg);
    lv_style_copy(&styles->roller_bg, &styles->ddlist_page);
    lv_style_set_bg_color(&styles->roller_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 20));
    lv_style_set_bg_grad_color(&styles->roller_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 40));
    lv_style_set_text_color(&styles->roller_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 5, 70));
    lv_style_set_text_opa(&styles->roller_bg, LV_STATE_DEFAULT, LV_OPA_60);
    lv_style_set_text_font(&styles->roller_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->roller_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_bg_color(&styles->roller_bg, LV_STATE_DISABLED, recalculate_hsv(theme_colors[36], 0, 6, 38));
    lv_style_set_bg_grad_dir(&styles->roller_bg, LV_STATE_DISABLED, LV_GRAD_DIR_NONE);
    lv_style_set_text_color(&styles->roller_bg, LV_STATE_DISABLED, lv_color_mix(theme_colors[32], COLOR_MY_GREY, LV_OPA_50));

    style_init_reset(&styles->roller_sel);
    lv_style_copy(&styles->roller_sel, &btn_rel);
    lv_style_set_text_line_space(&styles->roller_sel, LV_STATE_DEFAULT, LV_DPI / 8);
    lv_style_set_pad_top(&styles->roller_sel, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&styles->roller_sel, LV_STATE_DEFAULT, 0);
    lv_style_set_bg_color(&styles->roller_sel, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 20, 50));
    lv_style_set_bg_grad_color(&styles->roller_sel, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 20, 50));
    lv_style_set_radius(&styles->roller_sel, LV_STATE_DEFAULT, 0);
    lv_style_set_text_font(&styles->roller_sel, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->roller_sel, LV_STATE_DEFAULT, theme_fonts[0]);
#endif
}

static void tabview_init(void)
{
#if LV_USE_TABVIEW != 0
    style_init_reset(&styles->tabview_bg);
    lv_style_copy(&styles->tabview_bg, &styles->bg);
#endif
}

static void tileview_init(void)
{
#if LV_USE_TILEVIEW != 0
    style_init_reset(&styles->tileview_bg);
    lv_style_copy(&styles->tileview_bg, &styles->tabview_page_scrl);
#endif
}

static void table_init(void)
{
#if LV_USE_TABLE != 0
    style_init_reset(&styles->table_bg);
    lv_style_copy(&styles->table_bg, &styles->bg);
    lv_style_set_bg_grad_dir(&styles->table_bg, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
    lv_style_set_pad_left(&styles->table_bg, LV_STATE_DEFAULT, LV_DPX(32));
    lv_style_set_pad_right(&styles->table_bg, LV_STATE_DEFAULT, LV_DPX(32));
    lv_style_set_pad_top(&styles->table_bg, LV_STATE_DEFAULT, LV_DPX(32));
    lv_style_set_pad_bottom(&styles->table_bg, LV_STATE_DEFAULT, LV_DPX(32));
    lv_style_set_pad_inner(&styles->table_bg, LV_STATE_DEFAULT, LV_DPX(30));
    lv_style_set_bg_color(&styles->table_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[36], 0, 6, 38));
    lv_style_set_radius(&styles->table_bg, LV_STATE_DEFAULT, LV_DPI / 20);

    style_init_reset(&styles->table_cell);
    lv_style_copy(&styles->table_cell, &panel);
    lv_style_set_bg_color(&styles->table_cell, LV_STATE_DEFAULT, lv_color_hex(0x7D8485));
    lv_style_set_bg_grad_dir(&styles->table_cell, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
    lv_style_set_radius(&styles->table_cell, LV_STATE_DEFAULT, 0);
    lv_style_set_border_width(&styles->table_cell, LV_STATE_DEFAULT, 1);
    lv_style_set_pad_left(&styles->table_cell, LV_STATE_DEFAULT, 5);
    lv_style_set_pad_right(&styles->table_cell, LV_STATE_DEFAULT, 5);
    lv_style_set_pad_top(&styles->table_cell, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&styles->table_cell, LV_STATE_DEFAULT, 0);
    lv_style_set_text_font(&styles->table_cell, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->table_cell, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->table_cell_row);
    lv_style_copy(&styles->table_cell_row, &styles->table_cell);
    lv_style_set_bg_color(&styles->table_cell_row, LV_STATE_DEFAULT, lv_color_hex(0x2B3031));

    style_init_reset(&styles->table_cell_col);
    lv_style_copy(&styles->table_cell_col, &styles->table_cell);
    lv_style_set_bg_color(&styles->table_cell_col, LV_STATE_DEFAULT, lv_color_hex(0x494F50));
#endif
}

static void win_init(void)
{
#if LV_USE_WIN != 0
    style_init_reset(&styles->win_bg);
    lv_style_copy(&styles->win_bg, &styles->bg);
    lv_style_set_border_color(&styles->win_bg, LV_STATE_DEFAULT, theme_colors[28]);
    lv_style_set_border_width(&styles->win_bg, LV_STATE_DEFAULT, 1);

    style_init_reset(&styles->win_header);
    lv_style_copy(&styles->win_header, &styles->win_bg);
    lv_style_set_bg_color(&styles->win_header, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 20));
    lv_style_set_bg_grad_color(&styles->win_header, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[0], 0, 10, 20));
    lv_style_set_radius(&styles->win_header, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_left(&styles->win_header, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&styles->win_header, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&styles->win_header, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&styles->win_header, LV_STATE_DEFAULT, 0);

    style_init_reset(&styles->win_btn);
    lv_style_copy(&styles->win_btn, &lv_style_transp);
    lv_style_set_text_color(&styles->win_btn, LV_STATE_DEFAULT, theme_colors[34]);
    lv_style_set_value_color(&styles->win_btn, LV_STATE_DEFAULT, theme_colors[34]);

    lv_style_set_bg_color(&styles->win_btn, LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 10, 10));
    lv_style_set_bg_grad_color(&styles->win_btn, LV_STATE_PRESSED, recalculate_hsv(theme_colors[0], 0, 10, 10));
    lv_style_set_text_color(&styles->win_btn, LV_STATE_PRESSED, theme_colors[32]);
    lv_style_set_image_recolor(&styles->win_btn, LV_STATE_PRESSED, theme_colors[32]);
#endif
}

static void qtable_init(void)
{
#if LV_USE_QTABLE != 0
    style_init_reset(&styles->qtable_bg);
    lv_style_copy(&styles->qtable_bg, &styles->bg);
    lv_style_set_bg_grad_dir(&styles->qtable_bg, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
    lv_style_set_pad_left(&styles->qtable_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&styles->qtable_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&styles->qtable_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&styles->qtable_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_inner(&styles->qtable_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_bg_color(&styles->qtable_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[36], 0, 6, 38));
    lv_style_set_radius(&styles->qtable_bg, LV_STATE_DEFAULT, LV_DPI / 20);

    style_init_reset(&styles->qtable_cell);
    lv_style_copy(&styles->qtable_cell, &panel);
    lv_style_set_bg_color(&styles->qtable_cell, LV_STATE_DEFAULT, lv_color_hex(0x7D8485));
    lv_style_set_bg_grad_dir(&styles->qtable_cell, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
    lv_style_set_radius(&styles->qtable_cell, LV_STATE_DEFAULT, 0);
    lv_style_set_border_width(&styles->qtable_cell, LV_STATE_DEFAULT, 1);
    lv_style_set_pad_left(&styles->qtable_cell, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&styles->qtable_cell, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&styles->qtable_cell, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&styles->qtable_cell, LV_STATE_DEFAULT, 0);
    lv_style_set_text_font(&styles->qtable_cell, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->qtable_cell, LV_STATE_DEFAULT, theme_fonts[0]);
#endif
}

static void tabview_win_shared_init(void)
{
#if LV_USE_TABVIEW || LV_USE_WIN
    style_init_reset(&styles->tabview_btns_bg);
    lv_style_copy(&styles->tabview_btns_bg, &lv_style_transp);
    lv_style_set_text_font(&styles->tabview_btns_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->tabview_btns_bg, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->tabview_btns);
    lv_style_copy(&styles->tabview_btns, &styles->btn);
    lv_style_set_text_font(&styles->tabview_btns, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->tabview_btns, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->tabview_indic);
    lv_style_copy(&styles->tabview_indic, &lv_style_transp);

    style_init_reset(&styles->tabview_page_scrl);
    lv_style_copy(&styles->tabview_page_scrl, &lv_style_transp_tight);
#endif
}


/**********************
 *   GLOBAL FUNCTIONS
 **********************/
#include <stdio.h>
#define TH_STYLE_SIZE    (71)
static lv_style_t* theme_styles[TH_STYLE_SIZE];
static lv_style_t** prepare_theme_style_ptrs(void) {
    theme_styles[0] = &styles->scr;
    theme_styles[1] = &styles->bg;
    theme_styles[2] = &styles->bg_click;
    theme_styles[3] = &styles->bg_sec;
    theme_styles[4] = &styles->btn;
    theme_styles[5] = &styles->pad_inner;
    theme_styles[6] = &styles->pad_small;
    theme_styles[7] = &styles->arc_indic;
    theme_styles[8] = &styles->arc_bg;
    theme_styles[9] = &styles->arc_knob;
    theme_styles[10] = &styles->bar_bg;
    theme_styles[11] = &styles->bar_indic;
    theme_styles[12] = &styles->btnm_bg;
    theme_styles[13] = &styles->btnm_btn;
    theme_styles[14] = &styles->calendar_bg;
    theme_styles[15] = &styles->calendar_date_nums;
    theme_styles[16] = &styles->calendar_header;
    theme_styles[17] = &styles->calendar_daynames;
    theme_styles[18] = &styles->cpicker_bg;
    theme_styles[19] = &styles->cpicker_indic;
    theme_styles[20] = &styles->chart_bg;
    theme_styles[21] = &styles->chart_series_bg;
    theme_styles[22] = &styles->chart_series;
    theme_styles[23] = &styles->cb_bg;
    theme_styles[24] = &styles->cb_bullet;
    theme_styles[25] = &styles->ddlist_main;
    theme_styles[26] = &styles->ddlist_page;
    theme_styles[27] = &styles->ddlist_sel;
    theme_styles[28] = &styles->gauge_main;
    theme_styles[29] = &styles->gauge_strong;
    theme_styles[30] = &styles->gauge_needle;
    theme_styles[31] = &styles->kb_bg;
    theme_styles[32] = &styles->kb_btn;
    theme_styles[33] = &styles->led;
    theme_styles[34] = &styles->lmeter;
    theme_styles[35] = &styles->list_bg;
    theme_styles[36] = &styles->list_btn;
    theme_styles[37] = &styles->mbox_bg;
    theme_styles[38] = &styles->mbox_btn_bg;
    theme_styles[39] = &styles->mbox_btn;
    theme_styles[40] = &styles->page_bg;
    theme_styles[41] = &styles->sb;
    theme_styles[42] = &styles->edge_flash;
    theme_styles[43] = &styles->roller_bg;
    theme_styles[44] = &styles->roller_sel;
    theme_styles[45] = &styles->slider_knob;
    theme_styles[46] = &styles->slider_bg;
    theme_styles[47] = &styles->spinbox_bg;
    theme_styles[48] = &styles->spinbox_cursor;
    theme_styles[49] = &styles->spinbox_btn;
    theme_styles[50] = &styles->sw_knob;
    theme_styles[51] = &styles->table_bg;
    theme_styles[52] = &styles->table_cell;
    theme_styles[53] = &styles->tabview_bg;
    theme_styles[54] = &styles->tabview_btns;
    theme_styles[55] = &styles->tabview_btns_bg;
    theme_styles[56] = &styles->tabview_indic;
    theme_styles[57] = &styles->tabview_page_scrl;
    theme_styles[58] = &styles->tileview_bg;
    theme_styles[59] = &styles->ta_bg;
    theme_styles[60] = &styles->ta_cursor;
    theme_styles[61] = &styles->ta_placeholder;
    theme_styles[62] = &styles->win_bg;
    theme_styles[63] = &styles->win_header;
    theme_styles[64] = &styles->win_btn;
    theme_styles[65] = &styles->cont_bg;
    theme_styles[66] = &styles->table_cell_row;
    theme_styles[67] = &styles->table_cell_col;
    theme_styles[68] = &styles->qtable_bg;
    theme_styles[69] = &styles->qtable_cell;
    theme_styles[70] = NULL;
    return theme_styles;
}

static void initColor(double* colors) {
    int i;
    double data;

    for (i = 0; i < 24; i++) {
        _org_color[i] = colors[i];
        _palette[i] = lv_color_hex((uint32_t)colors[i]);
    }

    // ignore _palette[0] ~ _palette[23]
    for (i = 24; i < PALETTESIZE; i++) {
        data = colors[i];
        if (data < 0) {
            _palette[i] = lv_color_hex((uint32_t)colors[(int)((data * -1) - 1)]);
        }
        else
            _palette[i] = lv_color_hex((uint32_t)data);
    }
}

static void Palette(void) {
    double light_palette[PALETTESIZE] = {
    0x123456, // 1
    0x123456,
    0x123456,
    0x123456,
    0x123456,
    0x123456,
    0x123456,
    0x123456,
    0x123456,
    0x123456, // 10
    0x123456,
    0x123456,
    0x123456,
    0x123456,
    0x123456,
    0x123456,
    0x123456,
    0x123456,
    0x123456,
    0x123456, // 20
    0x123456,
    0x123456,
    0x123456,
    0x123456, // 24

    0x000000, // 25
    0x111111,
    0x202020,
    0x272727,
    0x3F3F3F,
    0x555555, // 30
    0x5596D8,
    0x808080,
    0xAAAAAA,
    0xC0C0C0,
    0xF0F0F0, // 35
    0xFFFFFF,
    0x444B4C, // bg
    0x000000, // theme sv ratio
    0x000000, // theme grad sv ratio
    };

    lv_font_t* theme_fonts[6] = {
    &lv_font_montserrat_18, // normal
    &lv_font_montserrat_34, // large
    &lv_font_montserrat_14,
    &lv_font_montserrat_18,
    &lv_font_montserrat_18,
    &lv_font_montserrat_18 };
}

int v6_get_font_id_part(lv_obj_t* obj, uint8_t part);
int v6_get_font_id(lv_obj_t* obj);
void v6_set_font_id_part(lv_obj_t* obj, uint8_t part, int id);
int v6_set_font_id(lv_obj_t* obj, int id);
int v6_set_color_id(lv_obj_t* obj, int groupid);
int v6_get_color_id(lv_obj_t* obj);
double v6_get_theme_color(int group, int order);
lv_font_t* v6_get_theme_font(int index);

/**
 * Initialize the default
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param flags ORed flags starting with `LV_THEME_DEF_FLAG_...`
 * @param font_small pointer to a small font
 * @param font_normal pointer to a normal font
 * @param font_subtitle pointer to a large font
 * @param font_title pointer to a extra large font
 * @return a pointer to reference this theme later
 */
static void update_id_color();
lv_theme_t* lv_theme_lvgl6_init(double* colors, lv_font_t** fonts)
{
    int i;
    /* This trick is required only to avoid the garbage collection of
     * styles' data if LVGL is used in a binding (e.g. Micropython)
     * In a general case styles could be simple `static lv_style_t my style` variables*/
    if (!inited) {
        LV_GC_ROOT(_lv_theme_material_styles) = lv_mem_alloc(sizeof(theme_styles_t));
        styles = (theme_styles_t*)LV_GC_ROOT(_lv_theme_material_styles);
    }

    initColor(colors); // check whether "-1" or not in colors array, if exist, re-map to "primary" or "secondary" area.

    if (!inited) {
        theme_colors = _palette;
        theme_fonts = fonts;
        theme.flags = 1;

        for (i = 0; i < FONTSIZE; i++) {
            lv_style_init(&_font[i]);
            style_font[i] = &_font[i];
        }

        theme.apply_xcb = NULL;
        theme.apply_cb = theme_apply;

        theme.user_data = (void*)prepare_theme_style_ptrs();
    }

    if (getPreThemeID() != v6_theme_ID) {
        for (i = 0; i < CL_STYLE_SIZE; i++) {
            style_init_reset(&_color[i]);
            style_color[i].obj = NULL;
            style_color[i].style = &_color[i];
            style_color[i].groupid = -1;
            style_color[i].themeid = -1;
            scCount = 0;
        }
    }

    theme.color_primary = theme_colors[0];
    theme.color_secondary = theme_colors[0]; // no used
    theme.font_small = theme_fonts[2];
    theme.font_normal = theme_fonts[0];
    theme.font_subtitle = theme_fonts[3];
    theme.font_title = theme_fonts[1];

    for (i = 0; i < FONTSIZE; i++) {
        lv_style_set_text_font(&_font[i], LV_STATE_DEFAULT, theme_fonts[i]);
        lv_style_set_text_font(&_font[i], LV_STATE_PRESSED, theme_fonts[i]);
        lv_style_set_text_font(&_font[i], LV_STATE_CHECKED, theme_fonts[i]);
        lv_style_set_text_font(&_font[i], LV_STATE_CHECKED | LV_STATE_PRESSED, theme_fonts[i]);
        lv_style_set_value_font(&_font[i], LV_STATE_DEFAULT, theme_fonts[i]);
        lv_style_set_value_font(&_font[i], LV_STATE_PRESSED, theme_fonts[i]);
        lv_style_set_value_font(&_font[i], LV_STATE_CHECKED, theme_fonts[i]);
        lv_style_set_value_font(&_font[i], LV_STATE_CHECKED | LV_STATE_PRESSED, theme_fonts[i]);
    }

    basic_init();
    cont_init();
    btn_init();
    label_init();
    bar_init();
    img_init();
    line_init();
    led_init();
    slider_init();
    switch_init();
    linemeter_init();
    gauge_init();
    arc_init();
    spinner_init();
    chart_init();
    calendar_init();
    cpicker_init();
    checkbox_init();
    btnmatrix_init();
    keyboard_init();
    msgbox_init();
    page_init();
    textarea_init();
    spinbox_init();
    list_init();
    ddlist_init();
    roller_init();
    tabview_win_shared_init();
    tabview_init();
    tileview_init();
    table_init();
    win_init();
    qtable_init();

    lv_theme_change(&theme);

    registerThemeTools(v6_theme_ID, v6_get_font_id_part, v6_get_font_id, v6_set_font_id_part, v6_set_font_id, v6_set_color_id, v6_get_color_id, v6_get_theme_color, v6_get_theme_font);

    update_id_color(); // if user call set_color_id(), this will update all node from prevTheme to NowTheme

    inited = true;
    setThemeID(v6_theme_ID); // call it after update_id_color();

    return &theme;
}

static bool isMyFont(lv_style_list_t* list, int* list_id, int* font_id) {
    int i, j;
    for (i = 0; i < list->style_cnt; i++) {
        for (j = 0; j < FONTSIZE; j++) {
            if (list->style_list[i] == style_font[j]) {
                *list_id = i;
                *font_id = j;
                return true;
            }
        }
    }
    return false;
}

int v6_get_font_id_part(lv_obj_t* obj, uint8_t part) {
    int list_id, font_id;
    lv_style_list_t* list = lv_obj_get_style_list(obj, part);
    if (list != NULL && isMyFont(list, &list_id, &font_id))
        return font_id;
    return -1; // default
}

int v6_get_font_id(lv_obj_t* obj) { // always get from part 0
    return v6_get_font_id_part(obj, 0);
}

void v6_set_font_id_part(lv_obj_t* obj, uint8_t part, int id) {

}

int v6_set_font_id(lv_obj_t* obj, int id) { // set all part
    int part, lid, fid, ret = -1;
    for (part = 0; part != 100; part++) {
        lv_style_list_t* list = lv_obj_get_style_list(obj, part);
        if (list == NULL) continue;
        if (isMyFont(list, &lid, &fid)) {
            if (id < 0) {// remove this style (back to default)
                _lv_style_list_remove_style(list, list->style_list[lid]);
                ret = fid;
            } else
                list->style_list[lid] = style_font[id]; // change style pointer
        } else {
            if (id < 0) continue; // no MyFont, just default, do nothing
            _lv_style_list_add_style(list, style_font[id]);
        }
        lv_obj_refresh_style(obj, part, LV_STYLE_PROP_ALL);
    }

    return ret;
}



// Handle COLOR style
static struct _group* getStyleGroup(int* styleColorID) {

    for (int i = 0; i < scCount; i++) {
        if (!lv_debug_check_obj_valid(style_color[i].obj)) {
            style_color[i].obj = NULL;
            memset(style_color[i].style, 0, sizeof(lv_style_t));
            style_color[i].groupid = -1;
            style_color[i].themeid = -1;
        }
    }

    for (int i = 0; i < scCount; i++) {
        if (style_color[i].obj == NULL) {
            *styleColorID = i;
            return &style_color[i];
        }
    }

    if (scCount == (CL_STYLE_SIZE - 1)) {
        printf("No space in style_color.\n");
        while (1);
    }

    *styleColorID = scCount;
    return &style_color[scCount++];
}

static int get_theme_style_index(lv_style_t* s) {
    int i;
    for (i = 0; theme_styles[i] != NULL; i++)
        if (theme_styles[i] == s)
            return i;

    return -1;
}

static bool isMyColor(lv_style_list_t* list, int* list_id, int* font_id) {
    int i, j;
    for (i = 0; i < list->style_cnt; i++) {
        for (j = 0; j < CL_STYLE_SIZE; j++) {
            if (list->style_list[i] == style_color[j].style) {
                *list_id = i;
                *font_id = j;
                return true;
            }
        }
    }
    return false;
}

static bool set_color(int id, int groupid, bool addit, lv_style_list_t* list, struct _group* p, int* styleColorID) {
    bool changed = true;
    uint8_t sv_ratio = lv_color_to_hsv(theme_colors[37]).v;
    uint8_t grad_sv_ratio = lv_color_to_hsv(theme_colors[38]).v;


    switch (id) {
    case -1: // no find anything
        changed = false;
        break;
    case 0: // scr
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_text_sel_bg_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 1: // bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_value_font(p->style, LV_STATE_DEFAULT, theme_fonts[groupid]);
        lv_style_set_text_font(p->style, LV_STATE_DEFAULT, theme_fonts[groupid]);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 2: // bg_click
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED, theme_colors[groupid]);
        lv_style_set_bg_color(p->style, LV_STATE_PRESSED | LV_STATE_CHECKED, lv_color_darken(theme_colors[groupid], LV_OPA_20));
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_darken(theme_colors[groupid], LV_OPA_20));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 4: case 13: case 32: case 39: case 49: case 54: // btn, btnm_btn, kb_btn, mbox_btn, spinbox_btn, tabview_btns
        if (addit) p = getStyleGroup(styleColorID);

        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 40));	// by &btn_rel
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 20));		// by &btn_rel

        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], sv_ratio, 80, 80));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], grad_sv_ratio, 80, 30));
        lv_style_set_bg_color(p->style, LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], sv_ratio, 80, 50));
        lv_style_set_bg_grad_color(p->style, LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], grad_sv_ratio, 80, 30));
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED, recalculate_hsv(theme_colors[groupid], grad_sv_ratio, 80, 20));
        lv_style_set_bg_grad_color(p->style, LV_STATE_CHECKED, recalculate_hsv(theme_colors[groupid], sv_ratio, 80, 50));
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], grad_sv_ratio, 80, 30));
        lv_style_set_bg_grad_color(p->style, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], sv_ratio, 80, 50));
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED, recalculate_hsv(theme_colors[groupid], 0, 80, 20));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DISABLED, recalculate_hsv(theme_colors[groupid], 0, 80, 20));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 5: // pad_inner
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 40));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 40));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 7: // arc_indic
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], sv_ratio, 80, 70));
        lv_style_set_line_color(p->style, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[groupid], sv_ratio, 80, 70), COLOR_MY_GREY, LV_OPA_50));
        lv_style_set_line_color(p->style, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[groupid], sv_ratio, 80, 70), COLOR_MY_GREY, LV_OPA_50));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 8: // arc_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 9: // arc_knob
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 40));   // by &styles->arc_knob
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 20));   // by &styles->arc_knob
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));   // by &styles->arc_knob
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));   // by &styles->arc_knob
        lv_style_set_bg_grad_color(p->style, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));   // by &styles->arc_knob
        lv_style_set_bg_grad_color(p->style, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));   // by &styles->arc_knob
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 10: // bar_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 25));	// by &panel
        lv_style_set_outline_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 11: // bar_indic
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], sv_ratio, 80, 70));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], sv_ratio, 80, 70));
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 20, 15));
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 80, 70), COLOR_MY_GREY, LV_OPA_50));
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 80, 70), COLOR_MY_GREY, LV_OPA_50));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 80, 70), COLOR_MY_GREY, LV_OPA_50));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 80, 70), COLOR_MY_GREY, LV_OPA_50));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 12: // btnm_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 40));	// by &btn_rel
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 20));		// by &btn_rel
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 14: // calendar_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 40));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 40));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 15: // calendar_date_nums
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 40));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 40));
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED, recalculate_hsv(theme_colors[groupid], 0, 80, 70));
        lv_style_set_bg_color(p->style, LV_STATE_FOCUSED, recalculate_hsv(theme_colors[groupid], 0, 80, 70));
        lv_style_set_text_color(p->style, LV_STATE_CHECKED, recalculate_hsv(theme_colors[groupid], 0, 40, 80));
        lv_style_set_text_color(p->style, LV_STATE_DISABLED, recalculate_hsv(theme_colors[groupid], 0, 0, 60));
        lv_style_set_border_color(p->style, LV_STATE_CHECKED, recalculate_hsv(theme_colors[groupid], 0, 80, 70));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 16: // calendar_header
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 20));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 20));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;

    case 17: // calendar_daynames
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 18));	// by &panel
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 18));	// by &panel
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 25));	// by &panel
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 20, 40));	// by &panel
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 18: // cpicker_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 30));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 19: // cpicker_indic
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_text_font(p->style, LV_STATE_DEFAULT, theme_fonts[groupid]);
        lv_style_set_value_font(p->style, LV_STATE_DEFAULT, theme_fonts[groupid]);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 20: // chart_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 21: // chart_series_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 22: // chart_series
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 23: // cb_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_outline_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 24: // cb_bullet
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 95));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 95));
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 50));
        lv_style_set_bg_color(p->style, LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], 0, 10, 80));
        lv_style_set_bg_grad_color(p->style, LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], 0, 10, 80));
        lv_style_set_border_color(p->style, LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], 0, 10, 20));
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED, recalculate_hsv(theme_colors[groupid], sv_ratio, 80, 90));
        lv_style_set_bg_grad_color(p->style, LV_STATE_CHECKED, recalculate_hsv(theme_colors[groupid], sv_ratio, 80, 90));
        lv_style_set_border_color(p->style, LV_STATE_CHECKED, recalculate_hsv(theme_colors[groupid], 0, 80, 50));
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], sv_ratio, 80, 70));
        lv_style_set_bg_grad_color(p->style, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], sv_ratio, 80, 70));
        lv_style_set_border_color(p->style, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], 0, 80, 30));
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 10, 95), COLOR_MY_GREY, LV_OPA_50));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 10, 95), COLOR_MY_GREY, LV_OPA_50));
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 80, 90), COLOR_MY_GREY, LV_OPA_50));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 80, 90), COLOR_MY_GREY, LV_OPA_50));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 25: // ddlist_main
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 40));	// by &btn_rel
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 20));		// by &btn_rel
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 26: // ddlist_page
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 40));	// by &btn_rel
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 20));		// by &btn_rel
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 27: // ddlist_sel
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 20, 50));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 20, 50));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 28: // gauge_main
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 70));
        lv_style_set_scale_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 70));
        lv_style_set_scale_end_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 80, 75));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 29: // gauge_strong
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 70));
        lv_style_set_scale_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 70));
        lv_style_set_scale_end_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 80, 75));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 30: // gauge_needle
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_scale_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 70));   // by &styles->gauge_main
        lv_style_set_scale_end_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 80, 75));    // by &styles->gauge_main
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 80, 75));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 31: // kb_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 33: // led
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], sv_ratio, 100, 100));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], grad_sv_ratio, 100, 40));
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], sv_ratio, 60, 60));
        lv_style_set_shadow_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], sv_ratio, 100, 100));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 34: // lmeter
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 70));
        lv_style_set_scale_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 95, 90));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 35: // list_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 18));	// by &panel
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 18));	// by &panel
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 25));	// by &panel
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 20, 40));	// by &panel
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 36: // list_btn
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 5));
        lv_style_set_text_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 5, 80));
        lv_style_set_image_recolor(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 5, 80));
        lv_style_set_bg_color(p->style, LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], 0, 10, 5));
        lv_style_set_bg_grad_color(p->style, LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], 0, 10, 5));
        lv_style_set_border_color(p->style, LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], 0, 10, 5));
        lv_style_set_text_color(p->style, LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], 0, 5, 80));
        lv_style_set_image_recolor(p->style, LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], 0, 5, 80));
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED, recalculate_hsv(theme_colors[groupid], 0, 80, 70));
        lv_style_set_bg_grad_color(p->style, LV_STATE_CHECKED, recalculate_hsv(theme_colors[groupid], 0, 80, 70));
        lv_style_set_border_color(p->style, LV_STATE_CHECKED, recalculate_hsv(theme_colors[groupid], 0, 60, 40));
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], 0, 80, 60));
        lv_style_set_bg_grad_color(p->style, LV_STATE_CHECKED | LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], 0, 80, 60));
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED, recalculate_hsv(theme_colors[groupid], 0, 10, 20));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DISABLED, recalculate_hsv(theme_colors[groupid], 0, 10, 20));
        lv_style_set_outline_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 37: // mbox_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 30, 30));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 30, 30));
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 20));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;

    case 40: // page_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 18));	// by &panel
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 18));	// by &panel
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 25));	// by &panel
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 20, 40));	// by &panel
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 41: // sb
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 30, 60));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 30, 60));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 43: // roller_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(&styles->roller_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 20));
        lv_style_set_bg_grad_color(&styles->roller_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 40));
        lv_style_set_text_color(&styles->roller_bg, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 5, 70));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 44: // roller_sel
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 20, 50));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 20, 50));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 45: // slider_knob
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 40));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 20));
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 46: // slider_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 25));	// by &panel
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 47: // spinbox_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 48: // spinbox_cursor
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 18));	// by &panel
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 18));	// by &panel
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 25));	// by &panel
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 20, 40));	// by &panel
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 50: // sw_knob
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 40));   // by &styles->slider_knob
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 20));  // by &styles->slider_knob
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));  // by &styles->slider_knob
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));   // by &styles->slider_knob
        lv_style_set_bg_grad_color(p->style, LV_STATE_DISABLED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50)); // by &styles->slider_knob
        lv_style_set_bg_grad_color(p->style, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(recalculate_hsv(theme_colors[groupid], 0, 10, 30), COLOR_MY_GREY, LV_OPA_50));  // by &styles->slider_knob
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 51: // table_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 52: // table_cell
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 18));	// by &panel
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 25));	// by &panel
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 20, 40));	// by &panel
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 53: // tabview_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 56: // tabview_indic
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 59: // ta_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 18));	// by &panel
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 18));	// by &panel
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 25));	// by &panel
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 20, 40));	// by &panel
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 62: // win_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 63: // win_header
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 20));
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 20));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 64: // win_btn
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], 0, 10, 10));
        lv_style_set_bg_grad_color(p->style, LV_STATE_PRESSED, recalculate_hsv(theme_colors[groupid], 0, 10, 10));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 65: // cont_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 66: // table_cell_row
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 18));	// by &panel
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 25));	// by &panel
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 20, 40));	// by &panel
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 67: // table_cell_col
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 18));	// by &panel
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 25));	// by &panel
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 20, 40));	// by &panel
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 68: // qtable_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 69: // qtable_cell
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid]);	// by &styles->bg
        lv_style_set_bg_grad_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 11, 18));	// by &panel
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 10, 25));	// by &panel
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, recalculate_hsv(theme_colors[groupid], 0, 20, 40));	// by &panel
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    default:
        changed = false;
        break;
    }

    return changed;
}

static int setNewColor(lv_style_list_t* list, bool addit, int groupid, struct _group* p, int* styleColorID) {
    int i, id;
    for (i = 0; i < list->style_cnt; i++) {
        id = get_theme_style_index(list->style_list[i]);
        if (id == -1) continue;
        if (set_color(id, groupid, addit, list, p, styleColorID)) // if something is changed
            return 1; // break it, notice: just change first one found style.
        // Nothing is changed, next style...
    }
    return 0;
}

int v6_set_color_id(lv_obj_t* obj, int groupid) {
    int part, lid, fid, mod, _groupID = -1, SCID = -1;
    for (part = 0; part != 100; part++) {
        lv_style_list_t* list = lv_obj_get_style_list(obj, part);
        if (list == NULL) continue;
        mod = 0;
        if (isMyColor(list, &lid, &fid)) {
            if (groupid < 0) {// remove this style (back to default)
                _lv_style_list_remove_style(list, list->style_list[lid]);
                _groupID = style_color[fid].groupid;
                mod = 1;
            } else
                mod = setNewColor(list, false, groupid, &style_color[fid], &SCID); // change color
        } else { // not my color style
            if (groupid < 0) continue; // keep _groupID and do nothing
            // set new color style
            mod = setNewColor(list, true, groupid, NULL, &SCID); // notice: maybe do nothing
        }
        if (mod > 0) {
            if (SCID != -1) style_color[SCID].obj = obj;
            lv_obj_refresh_style(obj, part, LV_STYLE_PROP_ALL);
        }
        // next part
    }

    return _groupID; // be leared groupID
}

int v6_get_color_id(lv_obj_t* obj) {
    int part, list_id, font_id;
    for (part = 0; part < 100; part++) {
        lv_style_list_t* list = lv_obj_get_style_list(obj, part);
        if (list != NULL && isMyColor(list, &list_id, &font_id))
            return style_color[font_id].groupid;
    }
    return -1; // use default theme
}

double v6_get_theme_color(int group, int order) {
    if (!inited || group < 0 || order != 0) return -1;
    if (group >= 24) return -1;
    return _org_color[group];
}

lv_font_t* v6_get_theme_font(int index) {
    if (index >= FONTSIZE || index < 0) return NULL;
    return theme_fonts[index];
}

static void report_style_mod_core(lv_obj_t* obj)
{
    int prevGroupID, preFontID;

    if ((obj == lv_layer_top()) || (obj == lv_layer_sys())) return;


    if (getPreThemeID() != v6_theme_ID)
        prevGroupID = clearPreColorStyle(obj);
    else
        prevGroupID = v6_get_color_id(obj); // dont clear node and get it's ID

    if (prevGroupID != -1) v6_set_color_id(obj, prevGroupID);

    if (getPreThemeID() != v6_theme_ID)
        preFontID = clearPreFontStyle(obj);
    else
        preFontID = v6_get_font_id(obj); // dont clear node and get it's ID

    if (preFontID != -1) v6_set_font_id(obj, preFontID);


    lv_obj_t* child = lv_obj_get_child(obj, NULL);
    while (child) {
        report_style_mod_core(child);
        child = lv_obj_get_child(obj, child);
    }
}

static void update_id_color() {
    // start to clear prev theme style node and set new theme style node
    lv_disp_t* d = lv_disp_get_next(NULL);

    while (d) {
        lv_obj_t* i;
        _LV_LL_READ(d->scr_ll, i) {
            report_style_mod_core(i);
        }
        d = lv_disp_get_next(d);
    }
}

static void theme_apply(lv_theme_t* th, lv_obj_t* obj, lv_theme_style_t name)
{
    LV_UNUSED(th);

    lv_style_list_t* list;

    switch (name) {
    case LV_THEME_NONE:
        break;

    case LV_THEME_SCR:
        list = lv_obj_get_style_list(obj, LV_OBJ_PART_MAIN);
        _lv_style_list_add_style(list, &styles->scr);

        break;
    case LV_THEME_OBJ:
        list = lv_obj_get_style_list(obj, LV_OBJ_PART_MAIN);
        _lv_style_list_add_style(list, &styles->bg);

        break;
#if LV_USE_CONT
    case LV_THEME_CONT:
        list = lv_obj_get_style_list(obj, LV_CONT_PART_MAIN);
        _lv_style_list_add_style(list, &styles->cont_bg);

        break;
#endif

#if LV_USE_BTN
    case LV_THEME_BTN:
        list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
        _lv_style_list_add_style(list, &styles->btn);

        break;
#endif

#if LV_USE_BTNMATRIX
    case LV_THEME_BTNMATRIX:
        list = lv_obj_get_style_list(obj, LV_BTNMATRIX_PART_BG);
        _lv_style_list_add_style(list, &styles->btnm_bg);


        list = lv_obj_get_style_list(obj, LV_BTNMATRIX_PART_BTN);
        _lv_style_list_add_style(list, &styles->btnm_btn);

        break;
#endif

#if LV_USE_KEYBOARD
    case LV_THEME_KEYBOARD:
        list = lv_obj_get_style_list(obj, LV_KEYBOARD_PART_BG);
        _lv_style_list_add_style(list, &styles->kb_bg);


        list = lv_obj_get_style_list(obj, LV_KEYBOARD_PART_BTN);
        _lv_style_list_add_style(list, &styles->kb_btn);

        break;
#endif

#if LV_USE_BAR
    case LV_THEME_BAR:
        list = lv_obj_get_style_list(obj, LV_BAR_PART_BG);
        _lv_style_list_add_style(list, &styles->bar_bg);


        list = lv_obj_get_style_list(obj, LV_BAR_PART_INDIC);
        _lv_style_list_add_style(list, &styles->bar_indic);

        break;
#endif

#if LV_USE_SWITCH
    case LV_THEME_SWITCH:
        list = lv_obj_get_style_list(obj, LV_SWITCH_PART_BG);
        _lv_style_list_add_style(list, &styles->bar_bg);


        list = lv_obj_get_style_list(obj, LV_SWITCH_PART_INDIC);
        _lv_style_list_add_style(list, &styles->bar_indic);


        list = lv_obj_get_style_list(obj, LV_SWITCH_PART_KNOB);
        _lv_style_list_add_style(list, &styles->sw_knob);

        break;
#endif

#if LV_USE_CANVAS
    case LV_THEME_CANVAS:

        break;
#endif

#if LV_USE_IMG
    case LV_THEME_IMAGE:

        break;
#endif

#if LV_USE_IMGBTN
    case LV_THEME_IMGBTN:
        list = lv_obj_get_style_list(obj, LV_IMGBTN_PART_MAIN);
        _lv_style_list_add_style(list, &styles->bg_sec);
        break;
#endif

#if LV_USE_LABEL
    case LV_THEME_LABEL:

        break;
#endif

#if LV_USE_LINE
    case LV_THEME_LINE:

        break;
#endif

#if LV_USE_ARC
    case LV_THEME_ARC:
        list = lv_obj_get_style_list(obj, LV_ARC_PART_BG);
        _lv_style_list_add_style(list, &styles->arc_bg);


        list = lv_obj_get_style_list(obj, LV_ARC_PART_INDIC);
        _lv_style_list_add_style(list, &styles->arc_indic);


        list = lv_obj_get_style_list(obj, LV_ARC_PART_KNOB);
        _lv_style_list_add_style(list, &styles->arc_knob);

        break;
#endif

#if LV_USE_SPINNER
    case LV_THEME_SPINNER:
        list = lv_obj_get_style_list(obj, LV_SPINNER_PART_BG);
        _lv_style_list_add_style(list, &styles->arc_bg);


        list = lv_obj_get_style_list(obj, LV_SPINNER_PART_INDIC);
        _lv_style_list_add_style(list, &styles->arc_indic);

        break;
#endif

#if LV_USE_SLIDER
    case LV_THEME_SLIDER:
        list = lv_obj_get_style_list(obj, LV_SLIDER_PART_BG);
        _lv_style_list_add_style(list, &styles->slider_bg);


        list = lv_obj_get_style_list(obj, LV_SLIDER_PART_INDIC);
        _lv_style_list_add_style(list, &styles->bar_indic);


        list = lv_obj_get_style_list(obj, LV_SLIDER_PART_KNOB);
        _lv_style_list_add_style(list, &styles->slider_knob);

        break;
#endif

#if LV_USE_CHECKBOX
    case LV_THEME_CHECKBOX:
        list = lv_obj_get_style_list(obj, LV_CHECKBOX_PART_BG);
        _lv_style_list_add_style(list, &styles->cb_bg);


        list = lv_obj_get_style_list(obj, LV_CHECKBOX_PART_BULLET);
        _lv_style_list_add_style(list, &styles->cb_bullet);

        break;
#endif

#if LV_USE_MSGBOX
    case LV_THEME_MSGBOX:
        list = lv_obj_get_style_list(obj, LV_MSGBOX_PART_BG);
        _lv_style_list_add_style(list, &styles->mbox_bg);

        break;

    case LV_THEME_MSGBOX_BTNS:
        list = lv_obj_get_style_list(obj, LV_MSGBOX_PART_BTN_BG);
        _lv_style_list_add_style(list, &styles->mbox_btn_bg); // now it is empty


        list = lv_obj_get_style_list(obj, LV_MSGBOX_PART_BTN);
        _lv_style_list_add_style(list, &styles->mbox_btn);

        break;

#endif
#if LV_USE_LED
    case LV_THEME_LED:
        list = lv_obj_get_style_list(obj, LV_LED_PART_MAIN);
        _lv_style_list_add_style(list, &styles->led);

        break;
#endif
#if LV_USE_PAGE
    case LV_THEME_PAGE:
        list = lv_obj_get_style_list(obj, LV_PAGE_PART_BG);
        _lv_style_list_add_style(list, &styles->page_bg);


        list = lv_obj_get_style_list(obj, LV_PAGE_PART_SCROLLABLE);
        _lv_style_list_add_style(list, &styles->pad_inner);


        list = lv_obj_get_style_list(obj, LV_PAGE_PART_SCROLLBAR);
        _lv_style_list_add_style(list, &styles->sb);


#if LV_USE_ANIMATION
        list = lv_obj_get_style_list(obj, LV_PAGE_PART_EDGE_FLASH);
        _lv_style_list_add_style(list, &styles->edge_flash);

#endif
        break;
#endif
#if LV_USE_TABVIEW
    case LV_THEME_TABVIEW:
        list = lv_obj_get_style_list(obj, LV_TABVIEW_PART_BG);
        _lv_style_list_add_style(list, &styles->tabview_bg);


        list = lv_obj_get_style_list(obj, LV_TABVIEW_PART_TAB_BG);
        _lv_style_list_add_style(list, &styles->tabview_btns_bg);


        list = lv_obj_get_style_list(obj, LV_TABVIEW_PART_INDIC);
        _lv_style_list_add_style(list, &styles->tabview_indic);


        list = lv_obj_get_style_list(obj, LV_TABVIEW_PART_TAB_BTN);
        _lv_style_list_add_style(list, &styles->tabview_btns);

        break;

    case LV_THEME_TABVIEW_PAGE:
        list = lv_obj_get_style_list(obj, LV_PAGE_PART_SCROLLABLE);
        _lv_style_list_add_style(list, &styles->tabview_page_scrl);


        break;
#endif

#if LV_USE_TILEVIEW
    case LV_THEME_TILEVIEW:
        list = lv_obj_get_style_list(obj, LV_TILEVIEW_PART_BG);
        _lv_style_list_add_style(list, &styles->tileview_bg);


        list = lv_obj_get_style_list(obj, LV_TILEVIEW_PART_SCROLLBAR);
        _lv_style_list_add_style(list, &styles->sb);


#if LV_USE_ANIMATION
        list = lv_obj_get_style_list(obj, LV_TILEVIEW_PART_EDGE_FLASH);
        _lv_style_list_add_style(list, &styles->edge_flash);

#endif
        break;
#endif


#if LV_USE_ROLLER
    case LV_THEME_ROLLER:
        list = lv_obj_get_style_list(obj, LV_ROLLER_PART_BG);
        // _lv_style_list_add_style(list, &styles->bg);
        _lv_style_list_add_style(list, &styles->roller_bg);


        list = lv_obj_get_style_list(obj, LV_ROLLER_PART_SELECTED);
        _lv_style_list_add_style(list, &styles->roller_sel);

        break;
#endif

#if LV_USE_OBJMASK
    case LV_THEME_OBJMASK:
        list = lv_obj_get_style_list(obj, LV_OBJMASK_PART_MAIN);

        break;
#endif

#if LV_USE_LIST
    case LV_THEME_LIST:
        list = lv_obj_get_style_list(obj, LV_LIST_PART_BG);
        //_lv_style_list_add_style(list, &styles->bg);
        _lv_style_list_add_style(list, &styles->list_bg);



        list = lv_obj_get_style_list(obj, LV_LIST_PART_SCROLLBAR);
        _lv_style_list_add_style(list, &styles->sb);

        break;

    case LV_THEME_LIST_BTN:
        list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
        _lv_style_list_add_style(list, &styles->list_btn);

        break;
#endif

#if LV_USE_DROPDOWN
    case LV_THEME_DROPDOWN:
        list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_MAIN);
        _lv_style_list_add_style(list, &styles->ddlist_main);


        list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_LIST);
        _lv_style_list_add_style(list, &styles->ddlist_page);


        list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_SCROLLBAR);
        _lv_style_list_add_style(list, &styles->sb);


        list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_SELECTED);
        _lv_style_list_add_style(list, &styles->ddlist_sel);

        break;
#endif

#if LV_USE_CHART
    case LV_THEME_CHART:
        list = lv_obj_get_style_list(obj, LV_CHART_PART_BG);
        _lv_style_list_add_style(list, &styles->chart_bg);


        list = lv_obj_get_style_list(obj, LV_CHART_PART_SERIES_BG);
        _lv_style_list_add_style(list, &styles->chart_series_bg);


        list = lv_obj_get_style_list(obj, LV_CHART_PART_CURSOR);
        _lv_style_list_add_style(list, &styles->chart_series_bg);


        list = lv_obj_get_style_list(obj, LV_CHART_PART_SERIES);
        _lv_style_list_add_style(list, &styles->chart_series);

        break;
#endif
#if LV_USE_TABLE
    case LV_THEME_TABLE: {
        list = lv_obj_get_style_list(obj, LV_TABLE_PART_BG);
        _lv_style_list_add_style(list, &styles->table_bg);


        int idx = 1; /* start value should be 1, not zero, since cell styles
                    start at 1 due to presence of LV_TABLE_PART_BG=0
                    in the enum (lv_table.h) */
                    /* declaring idx outside loop to work with older compilers */
        for (; idx <= LV_TABLE_CELL_STYLE_CNT; idx++) {
            list = lv_obj_get_style_list(obj, idx);
            if (idx == 2) {
                _lv_style_list_add_style(list, &styles->table_cell_row);
                continue;
            }
            else if (idx == 3) {
                _lv_style_list_add_style(list, &styles->table_cell_col);
                continue;
            }
            _lv_style_list_add_style(list, &styles->table_cell);

        }
        break;
    }
#endif

#if LV_USE_WIN
    case LV_THEME_WIN:
        list = lv_obj_get_style_list(obj, LV_WIN_PART_BG);
        _lv_style_list_add_style(list, &styles->win_bg);


        list = lv_obj_get_style_list(obj, LV_WIN_PART_SCROLLBAR);
        _lv_style_list_add_style(list, &styles->sb);


        list = lv_obj_get_style_list(obj, LV_WIN_PART_CONTENT_SCROLLABLE);
        _lv_style_list_add_style(list, &styles->tabview_page_scrl);


        list = lv_obj_get_style_list(obj, LV_WIN_PART_HEADER);
        _lv_style_list_add_style(list, &styles->win_header);

        break;

    case LV_THEME_WIN_BTN:
        list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
        _lv_style_list_add_style(list, &styles->win_btn);

        break;
#endif

#if LV_USE_TEXTAREA
    case LV_THEME_TEXTAREA:
        list = lv_obj_get_style_list(obj, LV_TEXTAREA_PART_BG);
        _lv_style_list_add_style(list, &styles->ta_bg);


        list = lv_obj_get_style_list(obj, LV_TEXTAREA_PART_PLACEHOLDER);
        _lv_style_list_add_style(list, &styles->ta_placeholder);


        list = lv_obj_get_style_list(obj, LV_TEXTAREA_PART_CURSOR);
        _lv_style_list_add_style(list, &styles->ta_cursor);


        list = lv_obj_get_style_list(obj, LV_TEXTAREA_PART_SCROLLBAR);
        _lv_style_list_add_style(list, &styles->sb);

        break;

#endif


#if LV_USE_SPINBOX
    case LV_THEME_SPINBOX:
        list = lv_obj_get_style_list(obj, LV_SPINBOX_PART_BG);
        _lv_style_list_add_style(list, &styles->spinbox_bg);

        list = lv_obj_get_style_list(obj, LV_SPINBOX_PART_CURSOR);
        _lv_style_list_add_style(list, &styles->spinbox_cursor);

        break;

    case LV_THEME_SPINBOX_BTN:
        list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
        _lv_style_list_add_style(list, &styles->spinbox_btn); // add by myself
        break;
#endif

#if LV_USE_CALENDAR
    case LV_THEME_CALENDAR:
        list = lv_obj_get_style_list(obj, LV_CALENDAR_PART_BG);
        _lv_style_list_add_style(list, &styles->calendar_bg);


        list = lv_obj_get_style_list(obj, LV_CALENDAR_PART_DATE);
        _lv_style_list_add_style(list, &styles->calendar_date_nums);


        list = lv_obj_get_style_list(obj, LV_CALENDAR_PART_HEADER);
        _lv_style_list_add_style(list, &styles->calendar_header);


        list = lv_obj_get_style_list(obj, LV_CALENDAR_PART_DAY_NAMES);
        _lv_style_list_add_style(list, &styles->calendar_daynames);

        break;
#endif
#if LV_USE_CPICKER
    case LV_THEME_CPICKER:
        list = lv_obj_get_style_list(obj, LV_CPICKER_PART_MAIN);
        _lv_style_list_add_style(list, &styles->cpicker_bg);


        list = lv_obj_get_style_list(obj, LV_CPICKER_PART_KNOB);
        _lv_style_list_add_style(list, &styles->cpicker_indic);

        break;
#endif

#if LV_USE_LINEMETER
    case LV_THEME_LINEMETER:
        list = lv_obj_get_style_list(obj, LV_LINEMETER_PART_MAIN);
        _lv_style_list_add_style(list, &styles->lmeter);

        break;
#endif
#if LV_USE_GAUGE
    case LV_THEME_GAUGE:
        list = lv_obj_get_style_list(obj, LV_GAUGE_PART_MAIN);
        _lv_style_list_add_style(list, &styles->gauge_main);


        list = lv_obj_get_style_list(obj, LV_GAUGE_PART_MAJOR);
        _lv_style_list_add_style(list, &styles->gauge_strong);


        list = lv_obj_get_style_list(obj, LV_GAUGE_PART_NEEDLE);
        _lv_style_list_add_style(list, &styles->gauge_needle);

        break;
#endif
#if LV_USE_QTABLE
    case LV_THEME_QTABLE: {
        list = lv_obj_get_style_list(obj, LV_QTABLE_PART_BG);
        _lv_style_list_add_style(list, &styles->qtable_bg);

        int idx = 1; /* start value should be 1, not zero, since cell styles
                    start at 1 due to presence of LV_TABLE_PART_BG=0
                    in the enum (lv_table.h) */
                    /* declaring idx outside loop to work with older compilers */
        for (; idx <= LV_QTABLE_CELL_STYLE_CNT; idx++) {
            list = lv_obj_get_style_list(obj, idx);
            _lv_style_list_add_style(list, &styles->qtable_cell);
        }
        break;
    }
#endif
    default:
        break;
    }

    lv_obj_refresh_style(obj, LV_OBJ_PART_ALL, LV_STYLE_PROP_ALL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void style_init_reset(lv_style_t* style)
{
    if (inited) lv_style_reset(style);
    else lv_style_init(style);
}
