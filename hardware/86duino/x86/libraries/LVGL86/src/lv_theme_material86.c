/**
 * @file lv_theme_material.c
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

   /*SCREEN*/

#define COLOR_SCR        (theme_colors[24])
#define COLOR_SCR_TEXT   (theme_colors[27])

/*BUTTON*/
#define COLOR_BTN           (theme_colors[25])
#define COLOR_BTN_PR        (IS_LIGHT ? lv_color_mix(theme_colors[0], COLOR_BTN, LV_OPA_20) : lv_color_mix(theme_colors[0], COLOR_BTN, LV_OPA_30))

#define COLOR_BTN_CHK       (theme_colors[0])
#define COLOR_BTN_CHK_PR    (lv_color_darken(theme_colors[0], LV_OPA_30))
#define COLOR_BTN_DIS       (theme_colors[37])

#define COLOR_BTN_BORDER        theme_colors[0]
#define COLOR_BTN_BORDER_PR     theme_colors[0]
#define COLOR_BTN_BORDER_CHK    theme_colors[0]
#define COLOR_BTN_BORDER_CHK_PR theme_colors[0]
#define COLOR_BTN_BORDER_INA    (theme_colors[34])

/*BACKGROUND*/
#define COLOR_BG            (theme_colors[25])
#define COLOR_BG_PR         (theme_colors[26])
#define COLOR_BG_CHK        theme_colors[0]
#define COLOR_BG_PR_CHK     lv_color_darken(theme_colors[0], LV_OPA_20)
#define COLOR_BG_DIS        (theme_colors[35])

#define COLOR_BG_BORDER         (theme_colors[31])   /*dfe7ed*/
#define COLOR_BG_BORDER_PR      (theme_colors[32])
#define COLOR_BG_BORDER_CHK     (theme_colors[32])
#define COLOR_BG_BORDER_CHK_PR  (theme_colors[32])
#define COLOR_BG_BORDER_DIS     (theme_colors[34])

#define COLOR_BG_TEXT           (theme_colors[27])
#define COLOR_BG_TEXT_PR        (theme_colors[27])
#define COLOR_BG_TEXT_CHK       (theme_colors[29])
#define COLOR_BG_TEXT_CHK_PR    (theme_colors[29])
#define COLOR_BG_TEXT_DIS       (theme_colors[33])

/*SECONDARY BACKGROUND*/
#define COLOR_BG_SEC            (theme_colors[38])
#define COLOR_BG_SEC_BORDER     (IS_LIGHT ? lv_color_hex(0xdfe7ed) : lv_color_hex(0x404040))
#define COLOR_BG_SEC_TEXT       (theme_colors[27])
#define COLOR_BG_SEC_TEXT_DIS   (IS_LIGHT ? lv_color_hex(0xaaaaaa) : lv_color_hex(0xa5a8ad))

#define TRANSITION_TIME         0/*((theme.flags & LV_THEME_MATERIAL_FLAG_NO_TRANSITION) ? 0 : 150)*/
#define BORDER_WIDTH            LV_DPX(2)
#define OUTLINE_WIDTH           ((theme.flags & LV_THEME_MATERIAL_FLAG_NO_FOCUS) ? 0 : LV_DPX(2))
#define IS_LIGHT (theme.flags & LV_THEME_MATERIAL_FLAG_LIGHT)

#define PAD_DEF (lv_disp_get_size_category(NULL) <= LV_DISP_SIZE_MEDIUM ? LV_DPX(15) : (LV_DPX(30)))

// 86Duino
#define COLOR_MY_GREY           (IS_LIGHT ? LV_COLOR_MAKE(0xE0, 0xE0, 0xE0) : LV_COLOR_MAKE(0x61, 0x61, 0x61))
/**********************
 *      TYPEDEFS
 **********************/
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

#define PALETTESIZE    49
lv_color_t _palette[PALETTESIZE];
double     _org_color[PALETTESIZE];

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_theme_t theme;
static theme_styles_t* styles;

static bool inited = false;
static void myCombineStyle(lv_style_t* dec, lv_style_t* src1, lv_style_t* src2, lv_style_t* src3);

/**********************
 *      MACROS
 **********************/

 /**********************
  *   STATIC FUNCTIONS
  **********************/

static void basic_init(void)
{
    style_init_reset(&styles->scr);
    lv_style_set_bg_opa(&styles->scr, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->scr, LV_STATE_DEFAULT, COLOR_SCR);
    lv_style_set_text_color(&styles->scr, LV_STATE_DEFAULT, COLOR_SCR_TEXT);
    lv_style_set_value_color(&styles->scr, LV_STATE_DEFAULT, COLOR_SCR_TEXT);
    lv_style_set_text_sel_color(&styles->scr, LV_STATE_DEFAULT, theme_colors[28]);
    lv_style_set_text_sel_bg_color(&styles->scr, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_value_font(&styles->scr, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_text_font(&styles->scr, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_outline_opa(&styles->scr, LV_STATE_DISABLED, LV_OPA_0);

    style_init_reset(&styles->bg);
    lv_style_set_radius(&styles->bg, LV_STATE_DEFAULT, LV_DPX(8));
    lv_style_set_bg_opa(&styles->bg, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->bg, LV_STATE_DEFAULT, COLOR_BG);
    lv_style_set_border_color(&styles->bg, LV_STATE_DEFAULT, COLOR_BG_BORDER);
    if ((theme.flags & LV_THEME_MATERIAL_FLAG_NO_FOCUS) == 0)lv_style_set_border_color(&styles->bg, LV_STATE_FOCUSED,
        theme_colors[0]);
    lv_style_set_border_color(&styles->bg, LV_STATE_EDITED, theme_colors[1]);
    lv_style_set_border_width(&styles->bg, LV_STATE_DEFAULT, BORDER_WIDTH);
    lv_style_set_border_post(&styles->bg, LV_STATE_DEFAULT, true);
    lv_style_set_text_color(&styles->bg, LV_STATE_DEFAULT, COLOR_BG_TEXT);
    lv_style_set_value_font(&styles->bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_color(&styles->bg, LV_STATE_DEFAULT, COLOR_BG_TEXT);
    lv_style_set_image_recolor(&styles->bg, LV_STATE_DEFAULT, theme_colors[30]);
    lv_style_set_line_color(&styles->bg, LV_STATE_DEFAULT, theme_colors[30]);
    lv_style_set_line_width(&styles->bg, LV_STATE_DEFAULT, 1);
    lv_style_set_outline_opa(&styles->bg, LV_STATE_DISABLED, LV_OPA_0);
    lv_style_set_pad_left(&styles->bg, LV_STATE_DEFAULT, PAD_DEF + BORDER_WIDTH);
    lv_style_set_pad_right(&styles->bg, LV_STATE_DEFAULT, PAD_DEF + BORDER_WIDTH);
    lv_style_set_pad_top(&styles->bg, LV_STATE_DEFAULT, PAD_DEF + BORDER_WIDTH);
    lv_style_set_pad_bottom(&styles->bg, LV_STATE_DEFAULT, PAD_DEF + BORDER_WIDTH);
    lv_style_set_pad_inner(&styles->bg, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_transition_time(&styles->bg, LV_STATE_DEFAULT, TRANSITION_TIME);
    lv_style_set_transition_prop_6(&styles->bg, LV_STATE_DEFAULT, LV_STYLE_BORDER_COLOR);
    lv_style_set_text_font(&styles->bg, LV_STATE_DEFAULT, theme_fonts[0]);

    // Note: This style is uesd on imgbtn for us, because no one obj use it before.
    style_init_reset(&styles->bg_sec);
    // NEW
    lv_style_set_text_color(&styles->bg_sec, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    // lv_style_set_image_recolor_opa(&styles->bg_sec, LV_STATE_PRESSED, LV_OPA_30);
    // lv_style_set_image_recolor(&styles->bg_sec, LV_STATE_PRESSED, LV_COLOR_BLACK);
    lv_style_set_image_recolor(&styles->bg_sec, LV_STATE_DISABLED, COLOR_MY_GREY);
    lv_style_set_image_recolor(&styles->bg_sec, LV_STATE_DISABLED | LV_STATE_CHECKED, COLOR_MY_GREY);
    lv_style_set_image_recolor_opa(&styles->bg_sec, LV_STATE_DISABLED, LV_OPA_50);
    lv_style_set_image_recolor_opa(&styles->bg_sec, LV_STATE_DISABLED | LV_STATE_CHECKED, LV_OPA_50);
    /* OLD:
    lv_style_copy(&styles->bg_sec, &styles->bg);
    lv_style_set_bg_color(&styles->bg_sec, LV_STATE_DEFAULT, COLOR_BG_SEC);
    lv_style_set_border_color(&styles->bg_sec, LV_STATE_DEFAULT, COLOR_BG_SEC_BORDER);
    lv_style_set_text_color(&styles->bg_sec, LV_STATE_DEFAULT, COLOR_BG_SEC_TEXT);
    lv_style_set_value_color(&styles->bg_sec, LV_STATE_DEFAULT, COLOR_BG_SEC_TEXT);
    lv_style_set_image_recolor(&styles->bg_sec, LV_STATE_DEFAULT, COLOR_BG_SEC_TEXT);
    lv_style_set_line_color(&styles->bg_sec, LV_STATE_DEFAULT, COLOR_BG_SEC_TEXT);*/

    style_init_reset(&styles->bg_click);
    lv_style_set_bg_color(&styles->bg_click, LV_STATE_PRESSED, COLOR_BG_PR);
    lv_style_set_bg_color(&styles->bg_click, LV_STATE_CHECKED, COLOR_BG_CHK);
    lv_style_set_bg_color(&styles->bg_click, LV_STATE_PRESSED | LV_STATE_CHECKED, COLOR_BG_PR_CHK);
    lv_style_set_bg_color(&styles->bg_click, LV_STATE_DISABLED, COLOR_BG_DIS);
    lv_style_set_border_width(&styles->bg_click, LV_STATE_CHECKED, 0);
    lv_style_set_border_color(&styles->bg_click, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_darken(theme_colors[0],
        LV_OPA_20));
    lv_style_set_border_color(&styles->bg_click, LV_STATE_PRESSED, COLOR_BG_BORDER_PR);
    lv_style_set_border_color(&styles->bg_click, LV_STATE_CHECKED, COLOR_BG_BORDER_CHK);
    lv_style_set_border_color(&styles->bg_click, LV_STATE_PRESSED | LV_STATE_CHECKED, COLOR_BG_BORDER_CHK_PR);
    lv_style_set_border_color(&styles->bg_click, LV_STATE_DISABLED, COLOR_BG_BORDER_DIS);
    lv_style_set_text_color(&styles->bg_click, LV_STATE_PRESSED, COLOR_BG_TEXT_PR);
    lv_style_set_text_color(&styles->bg_click, LV_STATE_CHECKED, COLOR_BG_TEXT_CHK);
    lv_style_set_text_color(&styles->bg_click, LV_STATE_PRESSED | LV_STATE_CHECKED, COLOR_BG_TEXT_CHK_PR);
    lv_style_set_text_color(&styles->bg_click, LV_STATE_DISABLED, COLOR_BG_TEXT_DIS);
    lv_style_set_image_recolor(&styles->bg_click, LV_STATE_PRESSED, theme_colors[30]);
    lv_style_set_image_recolor(&styles->bg_click, LV_STATE_CHECKED, COLOR_BG_TEXT_CHK);
    lv_style_set_image_recolor(&styles->bg_click, LV_STATE_PRESSED | LV_STATE_CHECKED, COLOR_BG_TEXT_CHK_PR);
    lv_style_set_image_recolor(&styles->bg_click, LV_STATE_DISABLED, COLOR_BG_TEXT_DIS);
    lv_style_set_outline_opa(&styles->bg_click, LV_STATE_DISABLED, LV_OPA_0);
    lv_style_set_transition_prop_5(&styles->bg_click, LV_STATE_DEFAULT, LV_STYLE_BG_COLOR);
    lv_style_set_text_font(&styles->bg_click, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->bg_click, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->btn);
    lv_style_set_radius(&styles->btn, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_bg_opa(&styles->btn, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->btn, LV_STATE_DEFAULT, COLOR_BTN);
    lv_style_set_bg_color(&styles->btn, LV_STATE_PRESSED, COLOR_BTN_PR);
    lv_style_set_bg_color(&styles->btn, LV_STATE_CHECKED, COLOR_BTN_CHK);
    lv_style_set_bg_color(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, COLOR_BTN_CHK_PR);
    lv_style_set_bg_color(&styles->btn, LV_STATE_DISABLED, COLOR_BG_DIS);
    lv_style_set_bg_color(&styles->btn, LV_STATE_DISABLED | LV_STATE_CHECKED, COLOR_BTN_DIS);
    lv_style_set_border_color(&styles->btn, LV_STATE_DEFAULT, COLOR_BTN_BORDER);
    lv_style_set_border_color(&styles->btn, LV_STATE_PRESSED, COLOR_BTN_BORDER_PR);
    lv_style_set_border_color(&styles->btn, LV_STATE_DISABLED, COLOR_BTN_BORDER_INA);
    lv_style_set_border_width(&styles->btn, LV_STATE_DEFAULT, BORDER_WIDTH);
    lv_style_set_border_opa(&styles->btn, LV_STATE_CHECKED, LV_OPA_TRANSP);

    lv_style_set_text_color(&styles->btn, LV_STATE_DEFAULT, theme_colors[27]);
    lv_style_set_text_color(&styles->btn, LV_STATE_PRESSED, theme_colors[27]);
    lv_style_set_text_color(&styles->btn, LV_STATE_CHECKED, theme_colors[29]);
    lv_style_set_text_color(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, theme_colors[29]);
    lv_style_set_text_color(&styles->btn, LV_STATE_DISABLED, theme_colors[33]);

    lv_style_set_image_recolor(&styles->btn, LV_STATE_DEFAULT, theme_colors[30]);
    lv_style_set_image_recolor(&styles->btn, LV_STATE_PRESSED, theme_colors[30]);
    lv_style_set_image_recolor(&styles->btn, LV_STATE_CHECKED, theme_colors[29]);
    lv_style_set_image_recolor(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, theme_colors[29]);
    lv_style_set_image_recolor(&styles->btn, LV_STATE_DISABLED, theme_colors[33]);

    lv_style_set_value_color(&styles->btn, LV_STATE_DEFAULT, theme_colors[27]);
    lv_style_set_value_color(&styles->btn, LV_STATE_PRESSED, theme_colors[27]);
    lv_style_set_value_color(&styles->btn, LV_STATE_CHECKED, theme_colors[29]);
    lv_style_set_value_color(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, theme_colors[29]);
    lv_style_set_value_color(&styles->btn, LV_STATE_DISABLED, theme_colors[33]);

    lv_style_set_pad_left(&styles->btn, LV_STATE_DEFAULT, LV_DPX(40));
    lv_style_set_pad_right(&styles->btn, LV_STATE_DEFAULT, LV_DPX(40));
    lv_style_set_pad_top(&styles->btn, LV_STATE_DEFAULT, LV_DPX(15));
    lv_style_set_pad_bottom(&styles->btn, LV_STATE_DEFAULT, LV_DPX(15));
    lv_style_set_pad_inner(&styles->btn, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_outline_width(&styles->btn, LV_STATE_DEFAULT, OUTLINE_WIDTH);
    lv_style_set_outline_opa(&styles->btn, LV_STATE_DEFAULT, LV_OPA_0);
    lv_style_set_outline_opa(&styles->btn, LV_STATE_FOCUSED, LV_OPA_50);
    lv_style_set_outline_opa(&styles->btn, LV_STATE_DISABLED, LV_OPA_0);
    lv_style_set_outline_color(&styles->btn, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_outline_color(&styles->btn, LV_STATE_EDITED, theme_colors[1]);
    lv_style_set_transition_time(&styles->btn, LV_STATE_DEFAULT, TRANSITION_TIME);
    lv_style_set_transition_prop_4(&styles->btn, LV_STATE_DEFAULT, LV_STYLE_BORDER_OPA);
    lv_style_set_transition_prop_5(&styles->btn, LV_STATE_DEFAULT, LV_STYLE_BG_COLOR);
    lv_style_set_transition_prop_6(&styles->btn, LV_STATE_DEFAULT, LV_STYLE_OUTLINE_OPA);
    lv_style_set_transition_delay(&styles->btn, LV_STATE_DEFAULT, TRANSITION_TIME);
    lv_style_set_transition_delay(&styles->btn, LV_STATE_PRESSED, 0);
    lv_style_set_text_font(&styles->btn, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->btn, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->pad_inner);

    lv_style_set_pad_inner(&styles->pad_inner, LV_STATE_DEFAULT,
        lv_disp_get_size_category(NULL) <= LV_DISP_MEDIUM_LIMIT ? LV_DPX(20) : LV_DPX(40));

    style_init_reset(&styles->pad_small);
    lv_style_int_t pad_small_value = lv_disp_get_size_category(NULL) <= LV_DISP_MEDIUM_LIMIT ? LV_DPX(10) : LV_DPX(20);
    lv_style_set_pad_left(&styles->pad_small, LV_STATE_DEFAULT, pad_small_value);
    lv_style_set_pad_right(&styles->pad_small, LV_STATE_DEFAULT, pad_small_value);
    lv_style_set_pad_top(&styles->pad_small, LV_STATE_DEFAULT, pad_small_value);
    lv_style_set_pad_bottom(&styles->pad_small, LV_STATE_DEFAULT, pad_small_value);
    lv_style_set_pad_inner(&styles->pad_small, LV_STATE_DEFAULT, pad_small_value);
}

static void cont_init(void)
{
#if LV_USE_CONT != 0
    style_init_reset(&styles->cont_bg);
    lv_style_copy(&styles->cont_bg, &styles->bg);
#endif
}

static void btn_init(void)
{
#if LV_USE_BTN != 0

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
    style_init_reset(&styles->bar_bg);
    lv_style_set_radius(&styles->bar_bg, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_bg_opa(&styles->bar_bg, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->bar_bg, LV_STATE_DEFAULT, COLOR_BG_SEC);
    lv_style_set_value_color(&styles->bar_bg, LV_STATE_DEFAULT, theme_colors[27]);
    lv_style_set_outline_color(&styles->bar_bg, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_outline_color(&styles->bar_bg, LV_STATE_EDITED, theme_colors[1]);
    lv_style_set_outline_opa(&styles->bar_bg, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_outline_opa(&styles->bar_bg, LV_STATE_FOCUSED, LV_OPA_50);
    lv_style_set_outline_opa(&styles->bar_bg, LV_STATE_DISABLED, LV_OPA_0);
    lv_style_set_outline_width(&styles->bar_bg, LV_STATE_DEFAULT, OUTLINE_WIDTH);
    lv_style_set_transition_time(&styles->bar_bg, LV_STATE_DEFAULT, TRANSITION_TIME);
    lv_style_set_transition_prop_6(&styles->bar_bg, LV_STATE_DEFAULT, LV_STYLE_OUTLINE_OPA);
    lv_style_set_text_font(&styles->bar_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->bar_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    // 86Duino
    lv_style_set_bg_color(&styles->bar_bg, LV_STATE_DISABLED, lv_color_mix(COLOR_BG_SEC, COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_color(&styles->bar_bg, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(COLOR_BG_SEC, COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_border_color(&styles->bar_bg, LV_STATE_DISABLED, COLOR_BTN_BORDER_INA);
    lv_style_set_value_color(&styles->bar_bg, LV_STATE_DISABLED, lv_color_mix(LV_COLOR_BLACK, COLOR_MY_GREY, LV_OPA_30));

    style_init_reset(&styles->bar_indic);
    lv_style_set_bg_opa(&styles->bar_indic, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_radius(&styles->bar_indic, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_bg_color(&styles->bar_indic, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_bg_color(&styles->bar_indic, LV_STATE_DISABLED, lv_color_mix(theme_colors[0], COLOR_MY_GREY, LV_OPA_50)); // 86Duino, ORG: theme_colors[36]
    lv_style_set_value_color(&styles->bar_indic, LV_STATE_DEFAULT, theme_colors[27]);
    lv_style_set_text_font(&styles->bar_indic, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->bar_indic, LV_STATE_DEFAULT, theme_fonts[0]);
    // 86Duino
    lv_style_set_bg_color(&styles->bar_indic, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(theme_colors[0], COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_border_color(&styles->bar_indic, LV_STATE_DISABLED, COLOR_BTN_BORDER_INA);
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
    style_init_reset(&styles->led);
    lv_style_set_bg_opa(&styles->led, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->led, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_border_width(&styles->led, LV_STATE_DEFAULT, 2);
    lv_style_set_border_opa(&styles->led, LV_STATE_DEFAULT, LV_OPA_50);
    lv_style_set_border_color(&styles->led, LV_STATE_DEFAULT, lv_color_lighten(theme_colors[0], LV_OPA_30));
    lv_style_set_radius(&styles->led, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_shadow_width(&styles->led, LV_STATE_DEFAULT, LV_DPX(15));
    lv_style_set_shadow_color(&styles->led, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_shadow_spread(&styles->led, LV_STATE_DEFAULT, LV_DPX(5));
    lv_style_set_text_font(&styles->led, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->led, LV_STATE_DEFAULT, theme_fonts[0]);
#endif
}

static void slider_init(void)
{
#if LV_USE_SLIDER != 0
    style_init_reset(&styles->slider_knob);
    lv_style_set_bg_opa(&styles->slider_knob, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->slider_knob, LV_STATE_DEFAULT, theme_colors[46]); // be modified for 86Duino
    lv_style_set_value_color(&styles->slider_knob, LV_STATE_DEFAULT, theme_colors[27]);
    lv_style_set_radius(&styles->slider_knob, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_pad_left(&styles->slider_knob, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_pad_right(&styles->slider_knob, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_pad_top(&styles->slider_knob, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_pad_bottom(&styles->slider_knob, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_text_font(&styles->slider_knob, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->slider_knob, LV_STATE_DEFAULT, theme_fonts[0]);
    // 86Duino
    lv_style_set_bg_color(&styles->slider_knob, LV_STATE_DISABLED, lv_color_mix(theme_colors[46], COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_color(&styles->slider_knob, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(theme_colors[46], COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_border_color(&styles->slider_knob, LV_STATE_DISABLED, COLOR_BTN_BORDER_INA);

    style_init_reset(&styles->slider_bg);
    myCombineStyle(&styles->slider_bg, &styles->bar_bg, NULL, NULL);
    lv_style_set_margin_left(&styles->slider_bg, LV_STATE_DEFAULT, LV_DPX(10));
    lv_style_set_margin_right(&styles->slider_bg, LV_STATE_DEFAULT, LV_DPX(10));
    lv_style_set_margin_top(&styles->slider_bg, LV_STATE_DEFAULT, LV_DPX(10));
    lv_style_set_margin_bottom(&styles->slider_bg, LV_STATE_DEFAULT, LV_DPX(10));
    lv_style_set_text_font(&styles->slider_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->slider_bg, LV_STATE_DEFAULT, theme_fonts[0]);
#endif
}

static void switch_init(void)
{
#if LV_USE_SWITCH != 0
    style_init_reset(&styles->sw_knob);
    lv_style_set_bg_opa(&styles->sw_knob, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->sw_knob, LV_STATE_DEFAULT, theme_colors[47]);
    lv_style_set_radius(&styles->sw_knob, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_pad_top(&styles->sw_knob, LV_STATE_DEFAULT, -LV_DPX(4));
    lv_style_set_pad_bottom(&styles->sw_knob, LV_STATE_DEFAULT, -LV_DPX(4));
    lv_style_set_pad_left(&styles->sw_knob, LV_STATE_DEFAULT, -LV_DPX(4));
    lv_style_set_pad_right(&styles->sw_knob, LV_STATE_DEFAULT, -LV_DPX(4));
    // 86Duino
    lv_style_set_bg_color(&styles->sw_knob, LV_STATE_DISABLED, lv_color_mix(theme_colors[47], COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_color(&styles->sw_knob, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(theme_colors[47], COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_border_color(&styles->sw_knob, LV_STATE_DISABLED, COLOR_BTN_BORDER_INA);
#endif
}

static void linemeter_init(void)
{
#if LV_USE_LINEMETER != 0
    style_init_reset(&styles->lmeter);
    myCombineStyle(&styles->lmeter, &styles->bg, NULL, NULL);
    lv_style_set_radius(&styles->lmeter, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_pad_left(&styles->lmeter, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_pad_right(&styles->lmeter, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_pad_top(&styles->lmeter, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_pad_inner(&styles->lmeter, LV_STATE_DEFAULT, LV_DPX(30));
    lv_style_set_scale_width(&styles->lmeter, LV_STATE_DEFAULT, LV_DPX(25));

    lv_style_set_line_color(&styles->lmeter, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_scale_grad_color(&styles->lmeter, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_scale_end_color(&styles->lmeter, LV_STATE_DEFAULT, theme_colors[42]);
    lv_style_set_line_width(&styles->lmeter, LV_STATE_DEFAULT, LV_DPX(10));
    lv_style_set_scale_end_line_width(&styles->lmeter, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_text_font(&styles->lmeter, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->lmeter, LV_STATE_DEFAULT, theme_fonts[0]); // use default font size
#endif
}

static void gauge_init(void)
{
#if LV_USE_GAUGE != 0
    style_init_reset(&styles->gauge_main);
    myCombineStyle(&styles->gauge_main, &styles->bg, NULL, NULL);
    lv_style_set_line_color(&styles->gauge_main, LV_STATE_DEFAULT, theme_colors[42]);
    lv_style_set_scale_grad_color(&styles->gauge_main, LV_STATE_DEFAULT, theme_colors[42]);
    lv_style_set_scale_end_color(&styles->gauge_main, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_line_width(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(5));
    lv_style_set_scale_end_line_width(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(4));
    lv_style_set_scale_end_border_width(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(8));
    lv_style_set_pad_left(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_pad_right(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_pad_top(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_pad_inner(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_scale_width(&styles->gauge_main, LV_STATE_DEFAULT, LV_DPX(15));
    lv_style_set_radius(&styles->gauge_main, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_text_font(&styles->gauge_main, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->gauge_main, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->gauge_strong);
    lv_style_set_line_color(&styles->gauge_strong, LV_STATE_DEFAULT, theme_colors[42]);
    lv_style_set_scale_grad_color(&styles->gauge_strong, LV_STATE_DEFAULT, theme_colors[42]);
    lv_style_set_scale_end_color(&styles->gauge_strong, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_line_width(&styles->gauge_strong, LV_STATE_DEFAULT, LV_DPX(8));
    lv_style_set_scale_end_line_width(&styles->gauge_strong, LV_STATE_DEFAULT, LV_DPX(8));
    lv_style_set_scale_width(&styles->gauge_strong, LV_STATE_DEFAULT, LV_DPX(25));

    style_init_reset(&styles->gauge_needle);
    lv_style_set_line_color(&styles->gauge_needle, LV_STATE_DEFAULT, theme_colors[41]);
    lv_style_set_line_width(&styles->gauge_needle, LV_STATE_DEFAULT, LV_DPX(8));
    lv_style_set_bg_opa(&styles->gauge_needle, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->gauge_needle, LV_STATE_DEFAULT, theme_colors[41]);
    lv_style_set_radius(&styles->gauge_needle, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_size(&styles->gauge_needle, LV_STATE_DEFAULT, LV_DPX(30));
    lv_style_set_pad_inner(&styles->gauge_needle, LV_STATE_DEFAULT, LV_DPX(10));
#endif
}

static void arc_init(void)
{
#if LV_USE_ARC != 0
    style_init_reset(&styles->arc_indic);
    lv_style_set_line_color(&styles->arc_indic, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_line_width(&styles->arc_indic, LV_STATE_DEFAULT, LV_DPX(25));
    lv_style_set_line_rounded(&styles->arc_indic, LV_STATE_DEFAULT, true);
    // 86Duino
    lv_style_set_line_color(&styles->arc_indic, LV_STATE_DISABLED, lv_color_mix(theme_colors[0], COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_line_color(&styles->arc_indic, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(theme_colors[0], COLOR_MY_GREY, LV_OPA_50));

    style_init_reset(&styles->arc_bg);
    myCombineStyle(&styles->arc_bg, &styles->bg, NULL, NULL);
    lv_style_set_line_color(&styles->arc_bg, LV_STATE_DEFAULT, COLOR_BG_SEC);
    lv_style_set_line_width(&styles->arc_bg, LV_STATE_DEFAULT, LV_DPX(25));
    lv_style_set_line_rounded(&styles->arc_bg, LV_STATE_DEFAULT, true);
    lv_style_set_border_width(&styles->arc_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_outline_width(&styles->arc_bg, LV_STATE_DEFAULT, 0);
    // lv_style_set_bg_opa(&styles->arc_bg, LV_STATE_DEFAULT, LV_OPA_0); 
    lv_style_set_text_font(&styles->arc_bg, LV_STATE_DEFAULT, theme_fonts[1]);
    lv_style_set_value_font(&styles->arc_bg, LV_STATE_DEFAULT, theme_fonts[1]);
    // 86Duino
    lv_style_set_line_color(&styles->arc_bg, LV_STATE_DISABLED, lv_color_mix(COLOR_BG_SEC, COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_border_color(&styles->arc_bg, LV_STATE_DISABLED, lv_color_mix(COLOR_BG_SEC, COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_line_color(&styles->arc_bg, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(COLOR_BG_SEC, COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_value_color(&styles->arc_bg, LV_STATE_DISABLED, lv_color_mix(LV_COLOR_BLACK, COLOR_MY_GREY, LV_OPA_30));
    
    style_init_reset(&styles->arc_knob);
    myCombineStyle(&styles->arc_knob, &styles->bg, &styles->bg_click, NULL);
    lv_style_set_radius(&styles->arc_knob, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_pad_top(&styles->arc_knob, LV_STATE_DEFAULT, LV_DPX(0));
    lv_style_set_pad_bottom(&styles->arc_knob, LV_STATE_DEFAULT, LV_DPX(0));
    lv_style_set_pad_left(&styles->arc_knob, LV_STATE_DEFAULT, LV_DPX(0));
    lv_style_set_pad_right(&styles->arc_knob, LV_STATE_DEFAULT, LV_DPX(0));

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
    myCombineStyle(&styles->chart_bg, &styles->bg, NULL, NULL);
    lv_style_set_text_color(&styles->chart_bg, LV_STATE_DEFAULT, theme_colors[40]);
    lv_style_set_text_font(&styles->chart_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->chart_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    myCombineStyle(&styles->chart_bg, NULL, &styles->pad_small, NULL);

    style_init_reset(&styles->chart_series_bg);
    myCombineStyle(&styles->chart_series_bg, &styles->pad_small, NULL, NULL);
    lv_style_set_line_width(&styles->chart_series_bg, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_line_dash_width(&styles->chart_series_bg, LV_STATE_DEFAULT, LV_DPX(10));
    lv_style_set_line_dash_gap(&styles->chart_series_bg, LV_STATE_DEFAULT, LV_DPX(10));
    lv_style_set_line_color(&styles->chart_series_bg, LV_STATE_DEFAULT, COLOR_BG_BORDER);
    lv_style_set_border_color(&styles->chart_series_bg, LV_STATE_DEFAULT, theme_colors[30]); // 86Duino
    lv_style_set_text_font(&styles->chart_series_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->chart_series_bg, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->chart_series);
    lv_style_set_line_width(&styles->chart_series, LV_STATE_DEFAULT, LV_DPX(3));
    lv_style_set_size(&styles->chart_series, LV_STATE_DEFAULT, LV_DPX(4));
    lv_style_set_pad_inner(&styles->chart_series, LV_STATE_DEFAULT, LV_DPX(2));     /*Space between columns*/
    lv_style_set_radius(&styles->chart_series, LV_STATE_DEFAULT, LV_DPX(1));

#endif
}

static void calendar_init(void)
{
#if LV_USE_CALENDAR
    style_init_reset(&styles->calendar_bg);
    myCombineStyle(&styles->calendar_bg, &styles->bg, NULL, NULL);

    style_init_reset(&styles->calendar_header);
    lv_style_set_pad_top(&styles->calendar_header, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_left(&styles->calendar_header, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_pad_right(&styles->calendar_header, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_pad_bottom(&styles->calendar_header, LV_STATE_DEFAULT, 0);
    lv_style_set_margin_top(&styles->calendar_header, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_margin_bottom(&styles->calendar_header, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_text_color(&styles->calendar_header, LV_STATE_PRESSED, theme_colors[39]);
    lv_style_set_text_font(&styles->calendar_header, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->calendar_header, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->calendar_daynames);
    lv_style_set_text_color(&styles->calendar_daynames, LV_STATE_DEFAULT, theme_colors[27]);
    lv_style_set_pad_left(&styles->calendar_daynames, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_pad_right(&styles->calendar_daynames, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_pad_bottom(&styles->calendar_daynames, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_text_font(&styles->calendar_daynames, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->calendar_daynames, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->calendar_date_nums);
    lv_style_set_radius(&styles->calendar_date_nums, LV_STATE_DEFAULT, LV_DPX(4));
    lv_style_set_text_color(&styles->calendar_date_nums, LV_STATE_CHECKED, theme_colors[27]);
    lv_style_set_text_color(&styles->calendar_date_nums, LV_STATE_DISABLED, theme_colors[33]);
    lv_style_set_bg_opa(&styles->calendar_date_nums, LV_STATE_CHECKED, IS_LIGHT ? LV_OPA_20 : LV_OPA_40);
    lv_style_set_bg_opa(&styles->calendar_date_nums, LV_STATE_PRESSED, LV_OPA_20);
    lv_style_set_bg_opa(&styles->calendar_date_nums, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_style_set_text_color(&styles->calendar_date_nums, LV_STATE_FOCUSED, LV_COLOR_WHITE);
    lv_style_set_bg_color(&styles->calendar_date_nums, LV_STATE_FOCUSED, theme_colors[0]);
    lv_style_set_bg_color(&styles->calendar_date_nums, LV_STATE_DEFAULT, theme_colors[39]);
    lv_style_set_bg_color(&styles->calendar_date_nums, LV_STATE_CHECKED, theme_colors[0]);
    lv_style_set_border_width(&styles->calendar_date_nums, LV_STATE_CHECKED, 2);
    lv_style_set_border_side(&styles->calendar_date_nums, LV_STATE_CHECKED, LV_BORDER_SIDE_LEFT);
    lv_style_set_border_color(&styles->calendar_date_nums, LV_STATE_CHECKED, theme_colors[0]);
    lv_style_set_pad_inner(&styles->calendar_date_nums, LV_STATE_DEFAULT, LV_DPX(3));
    lv_style_set_pad_left(&styles->calendar_date_nums, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_pad_right(&styles->calendar_date_nums, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_pad_bottom(&styles->calendar_date_nums, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_text_font(&styles->calendar_date_nums, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->calendar_date_nums, LV_STATE_DEFAULT, theme_fonts[0]);
#endif
}

static void cpicker_init(void)
{
#if LV_USE_CPICKER
    style_init_reset(&styles->cpicker_bg);
    lv_style_set_scale_width(&styles->cpicker_bg, LV_STATE_DEFAULT, LV_DPX(30));
    lv_style_set_bg_opa(&styles->cpicker_bg, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->cpicker_bg, LV_STATE_DEFAULT, COLOR_SCR);
    lv_style_set_pad_inner(&styles->cpicker_bg, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_radius(&styles->cpicker_bg, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_text_font(&styles->cpicker_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->cpicker_bg, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->cpicker_indic);
    lv_style_set_radius(&styles->cpicker_indic, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_bg_color(&styles->cpicker_indic, LV_STATE_DEFAULT, theme_colors[25]);
    lv_style_set_bg_opa(&styles->cpicker_indic, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_border_width(&styles->cpicker_indic, LV_STATE_DEFAULT, 2);
    lv_style_set_border_color(&styles->cpicker_indic, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_border_color(&styles->cpicker_indic, LV_STATE_FOCUSED, theme_colors[0]);
    lv_style_set_border_color(&styles->cpicker_indic, LV_STATE_EDITED, theme_colors[1]);
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
    style_init_reset(&styles->cb_bg);
    lv_style_set_radius(&styles->cb_bg, LV_STATE_DEFAULT, LV_DPX(4));
    lv_style_set_pad_inner(&styles->cb_bg, LV_STATE_DEFAULT, LV_DPX(10));
    lv_style_set_outline_color(&styles->cb_bg, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_outline_opa(&styles->cb_bg, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_outline_opa(&styles->cb_bg, LV_STATE_FOCUSED, LV_OPA_50);
    lv_style_set_outline_opa(&styles->cb_bg, LV_STATE_DISABLED, LV_OPA_0);
    lv_style_set_outline_width(&styles->cb_bg, LV_STATE_DEFAULT, OUTLINE_WIDTH);
    lv_style_set_outline_pad(&styles->cb_bg, LV_STATE_DEFAULT, LV_DPX(10));
    lv_style_set_transition_time(&styles->cb_bg, LV_STATE_DEFAULT, TRANSITION_TIME);
    lv_style_set_transition_prop_6(&styles->cb_bg, LV_STATE_DEFAULT, LV_STYLE_OUTLINE_OPA);
    lv_style_set_text_font(&styles->cb_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_text_color(&styles->cb_bg, LV_STATE_DISABLED, lv_color_mix(LV_COLOR_BLACK, COLOR_MY_GREY, LV_OPA_30));
    lv_style_set_value_font(&styles->cb_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    // 86Duino
    // lv_style_set_outline_color(&styles->cb_bg, LV_STATE_DISABLED, COLOR_BG_DIS);
    // lv_style_set_border_color(&styles->cb_bg, LV_STATE_DISABLED, COLOR_BTN_BORDER_INA);
    // lv_style_set_outline_color(&styles->cb_bg, LV_STATE_DISABLED | LV_STATE_CHECKED, COLOR_BTN_DIS);

    style_init_reset(&styles->cb_bullet);
    myCombineStyle(&styles->cb_bullet, &styles->btn, NULL, NULL);
    lv_style_set_outline_opa(&styles->cb_bullet, LV_STATE_FOCUSED, LV_OPA_TRANSP);
    lv_style_set_outline_opa(&styles->cb_bullet, LV_STATE_DISABLED, LV_OPA_0);
    lv_style_set_radius(&styles->cb_bullet, LV_STATE_DEFAULT, LV_DPX(4));
    lv_style_set_pattern_image(&styles->cb_bullet, LV_STATE_CHECKED, LV_SYMBOL_OK);
    lv_style_set_pattern_recolor(&styles->cb_bullet, LV_STATE_CHECKED, LV_COLOR_WHITE);
    lv_style_set_pattern_opa(&styles->cb_bullet, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_pattern_opa(&styles->cb_bullet, LV_STATE_CHECKED, LV_OPA_COVER);
    lv_style_set_transition_prop_3(&styles->cb_bullet, LV_STATE_DEFAULT, LV_STYLE_PATTERN_OPA);
    lv_style_set_text_font(&styles->cb_bullet, LV_STATE_CHECKED, theme_fonts[0]);
    lv_style_set_value_font(&styles->cb_bullet, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->cb_bullet, LV_STATE_CHECKED, theme_fonts[0]);
    lv_style_set_pad_left(&styles->cb_bullet, LV_STATE_DEFAULT, LV_DPX(3));
    lv_style_set_pad_right(&styles->cb_bullet, LV_STATE_DEFAULT, LV_DPX(3));
    lv_style_set_pad_top(&styles->cb_bullet, LV_STATE_DEFAULT, LV_DPX(3));
    lv_style_set_pad_bottom(&styles->cb_bullet, LV_STATE_DEFAULT, LV_DPX(3));
    // 86Duino
    lv_style_set_border_color(&styles->cb_bullet, LV_STATE_DISABLED, lv_color_mix(COLOR_BTN_BORDER, COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_pattern_recolor(&styles->cb_bullet, LV_STATE_DISABLED, lv_color_mix(LV_COLOR_WHITE, COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_pattern_recolor(&styles->cb_bullet, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(LV_COLOR_WHITE, COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_color(&styles->cb_bullet, LV_STATE_DISABLED, lv_color_mix(LV_COLOR_WHITE, COLOR_MY_GREY, LV_OPA_50));
    lv_style_set_bg_color(&styles->cb_bullet, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(theme_colors[0], COLOR_MY_GREY, LV_OPA_50));
#endif
}

static void btnmatrix_init(void)
{
    style_init_reset(&styles->btnm_bg);
    myCombineStyle(&styles->btnm_bg, &styles->bg, &styles->pad_small, NULL);

    style_init_reset(&styles->btnm_btn);
    myCombineStyle(&styles->btnm_btn, &styles->bg, &styles->bg_click, NULL);
}

static void keyboard_init(void)
{
#if LV_USE_KEYBOARD
    style_init_reset(&styles->kb_bg);
    myCombineStyle(&styles->kb_bg, &styles->scr, NULL, NULL);
    lv_style_set_radius(&styles->kb_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_border_width(&styles->kb_bg, LV_STATE_DEFAULT, LV_DPX(4));
    lv_style_set_border_side(&styles->kb_bg, LV_STATE_DEFAULT, LV_BORDER_SIDE_TOP);
    lv_style_set_border_color(&styles->kb_bg, LV_STATE_DEFAULT, theme_colors[32]);
    lv_style_set_border_color(&styles->kb_bg, LV_STATE_EDITED, theme_colors[1]);
    lv_style_set_text_font(&styles->kb_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->kb_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_pad_left(&styles->kb_bg, LV_STATE_DEFAULT, LV_DPX(5));
    lv_style_set_pad_right(&styles->kb_bg, LV_STATE_DEFAULT, LV_DPX(5));
    lv_style_set_pad_top(&styles->kb_bg, LV_STATE_DEFAULT, LV_DPX(5));
    lv_style_set_pad_bottom(&styles->kb_bg, LV_STATE_DEFAULT, LV_DPX(5));
    lv_style_set_pad_inner(&styles->kb_bg, LV_STATE_DEFAULT, LV_DPX(3));

    style_init_reset(&styles->kb_btn);
    myCombineStyle(&styles->kb_btn, &styles->bg, &styles->bg_click, NULL);

#endif
}

static void msgbox_init(void)
{
#if LV_USE_MSGBOX
    style_init_reset(&styles->mbox_bg);
    myCombineStyle(&styles->mbox_bg, &styles->bg, NULL, NULL);
    lv_style_set_shadow_width(&styles->mbox_bg, LV_STATE_DEFAULT, LV_DPX(50));
    lv_style_set_shadow_color(&styles->mbox_bg, LV_STATE_DEFAULT, theme_colors[44]);
    lv_style_set_text_font(&styles->mbox_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->mbox_bg, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->mbox_btn_bg);
    myCombineStyle(&styles->mbox_btn_bg, &styles->pad_small, NULL, NULL);

    style_init_reset(&styles->mbox_btn);
    myCombineStyle(&styles->mbox_btn, &styles->btn, NULL, NULL);
#endif
}

static void page_init(void)
{
#if LV_USE_PAGE
    style_init_reset(&styles->page_bg);
    lv_style_copy(&styles->page_bg, &styles->bg);

    style_init_reset(&styles->sb);
    lv_style_set_bg_opa(&styles->sb, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->sb, LV_STATE_DEFAULT, theme_colors[45]);
    lv_style_set_radius(&styles->sb, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_size(&styles->sb, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_pad_right(&styles->sb, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_pad_bottom(&styles->sb, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_text_font(&styles->sb, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->sb, LV_STATE_DEFAULT, theme_fonts[0]);

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
    myCombineStyle(&styles->ta_bg, &styles->bg, &styles->pad_small, NULL);
    lv_style_set_bg_color(&styles->ta_bg, LV_STATE_DISABLED, COLOR_MY_GREY);
    lv_style_set_text_color(&styles->ta_bg, LV_STATE_DISABLED, lv_color_mix(LV_COLOR_BLACK, COLOR_MY_GREY, LV_OPA_30));

    style_init_reset(&styles->ta_cursor);
    lv_style_set_border_color(&styles->ta_cursor, LV_STATE_DEFAULT, COLOR_BG_SEC_TEXT);
    lv_style_set_border_width(&styles->ta_cursor, LV_STATE_DEFAULT, LV_DPX(2));
    lv_style_set_pad_left(&styles->ta_cursor, LV_STATE_DEFAULT, LV_DPX(1));
    lv_style_set_border_side(&styles->ta_cursor, LV_STATE_DEFAULT, LV_BORDER_SIDE_LEFT);
    lv_style_set_text_font(&styles->ta_cursor, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->ta_cursor, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->ta_placeholder);
    lv_style_set_text_color(&styles->ta_placeholder, LV_STATE_DEFAULT, theme_colors[33]);
    lv_style_set_text_font(&styles->ta_placeholder, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->ta_placeholder, LV_STATE_DEFAULT, theme_fonts[0]);
#endif
}

static void spinbox_init(void)
{
#if LV_USE_SPINBOX
    style_init_reset(&styles->spinbox_bg);
    myCombineStyle(&styles->spinbox_bg, &styles->bg, &styles->pad_small, NULL);
    lv_style_set_text_color(&styles->spinbox_bg, LV_STATE_DISABLED, lv_color_mix(LV_COLOR_BLACK, COLOR_MY_GREY, LV_OPA_30));

    style_init_reset(&styles->spinbox_cursor);
    lv_style_set_bg_opa(&styles->spinbox_cursor, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->spinbox_cursor, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_text_color(&styles->spinbox_cursor, LV_STATE_DEFAULT, theme_colors[29]);
    lv_style_set_text_font(&styles->spinbox_cursor, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->spinbox_cursor, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_pad_top(&styles->spinbox_cursor, LV_STATE_DEFAULT, LV_DPX(100));
    lv_style_set_pad_bottom(&styles->spinbox_cursor, LV_STATE_DEFAULT, LV_DPX(100));

    style_init_reset(&styles->spinbox_btn);
    myCombineStyle(&styles->spinbox_btn, &styles->bg, &styles->bg_click, NULL);

#endif
}

static void list_init(void)
{
#if LV_USE_LIST != 0
    style_init_reset(&styles->list_bg);
    myCombineStyle(&styles->list_bg, &styles->bg, NULL, NULL);
    lv_style_set_clip_corner(&styles->list_bg, LV_STATE_DEFAULT, true);
    lv_style_set_pad_left(&styles->list_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&styles->list_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&styles->list_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&styles->list_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_inner(&styles->list_bg, LV_STATE_DEFAULT, 0);
    lv_style_set_text_font(&styles->list_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->list_bg, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->list_btn);
    lv_style_set_bg_opa(&styles->list_btn, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->list_btn, LV_STATE_DEFAULT, COLOR_BG);
    lv_style_set_bg_color(&styles->list_btn, LV_STATE_PRESSED, COLOR_BG_PR);
    lv_style_set_bg_color(&styles->list_btn, LV_STATE_DISABLED, COLOR_BG_DIS);
    lv_style_set_bg_color(&styles->list_btn, LV_STATE_CHECKED, COLOR_BG_CHK);
    lv_style_set_bg_color(&styles->list_btn, LV_STATE_CHECKED | LV_STATE_PRESSED, COLOR_BG_PR_CHK);

    lv_style_set_text_color(&styles->list_btn, LV_STATE_DEFAULT, COLOR_BG_TEXT);
    lv_style_set_text_color(&styles->list_btn, LV_STATE_CHECKED, COLOR_BG_TEXT_CHK);
    lv_style_set_text_color(&styles->list_btn, LV_STATE_DISABLED, COLOR_BG_TEXT_DIS);

    lv_style_set_image_recolor(&styles->list_btn, LV_STATE_DEFAULT, theme_colors[30]);
    lv_style_set_image_recolor(&styles->list_btn, LV_STATE_CHECKED, COLOR_BG_TEXT_CHK);
    lv_style_set_image_recolor(&styles->list_btn, LV_STATE_DISABLED, COLOR_BG_TEXT_DIS);

    lv_style_set_border_side(&styles->list_btn, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_color(&styles->list_btn, LV_STATE_DEFAULT, COLOR_BG_BORDER);
    lv_style_set_border_color(&styles->list_btn, LV_STATE_FOCUSED, theme_colors[0]);
    lv_style_set_border_width(&styles->list_btn, LV_STATE_DEFAULT, 1);

    lv_style_set_outline_color(&styles->list_btn, LV_STATE_CHECKED, theme_colors[43]);
    lv_style_set_outline_width(&styles->list_btn, LV_STATE_CHECKED, OUTLINE_WIDTH);
    lv_style_set_outline_pad(&styles->list_btn, LV_STATE_CHECKED, -BORDER_WIDTH);
    lv_style_set_outline_color(&styles->list_btn, LV_STATE_FOCUSED, theme_colors[43]);
    lv_style_set_outline_width(&styles->list_btn, LV_STATE_FOCUSED, OUTLINE_WIDTH);
    lv_style_set_outline_pad(&styles->list_btn, LV_STATE_FOCUSED, -BORDER_WIDTH);

    lv_style_set_pad_left(&styles->list_btn, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_pad_right(&styles->list_btn, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_pad_top(&styles->list_btn, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_pad_bottom(&styles->list_btn, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_pad_inner(&styles->list_btn, LV_STATE_DEFAULT, PAD_DEF);

    lv_style_set_transform_width(&styles->list_btn, LV_STATE_DEFAULT, -PAD_DEF);
    lv_style_set_transform_width(&styles->list_btn, LV_STATE_PRESSED, -BORDER_WIDTH);
    lv_style_set_transform_width(&styles->list_btn, LV_STATE_CHECKED, -BORDER_WIDTH);
    lv_style_set_transform_width(&styles->list_btn, LV_STATE_DISABLED, -BORDER_WIDTH);
    lv_style_set_transform_width(&styles->list_btn, LV_STATE_FOCUSED, -BORDER_WIDTH);

    lv_style_set_transition_time(&styles->list_btn, LV_STATE_DEFAULT, TRANSITION_TIME);
    lv_style_set_transition_prop_6(&styles->list_btn, LV_STATE_DEFAULT, LV_STYLE_BG_COLOR);
    lv_style_set_transition_prop_5(&styles->list_btn, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_WIDTH);

    lv_style_set_text_font(&styles->list_btn, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->list_btn, LV_STATE_DEFAULT, theme_fonts[0]);
#endif
}

static void ddlist_init(void)
{
#if LV_USE_DROPDOWN != 0
    style_init_reset(&styles->ddlist_main);
    myCombineStyle(&styles->ddlist_main, &styles->bg, &styles->bg_click, &styles->pad_small);

    style_init_reset(&styles->ddlist_page);
    myCombineStyle(&styles->ddlist_page, &styles->bg, NULL, NULL);
    lv_style_set_text_line_space(&styles->ddlist_page, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_clip_corner(&styles->ddlist_page, LV_STATE_DEFAULT, true);
    lv_style_set_text_font(&styles->ddlist_page, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->ddlist_page, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->ddlist_sel);
    lv_style_set_bg_opa(&styles->ddlist_sel, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->ddlist_sel, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_text_color(&styles->ddlist_sel, LV_STATE_DEFAULT, theme_colors[29]);
    lv_style_set_bg_color(&styles->ddlist_sel, LV_STATE_PRESSED, COLOR_BG_PR);
    lv_style_set_text_color(&styles->ddlist_sel, LV_STATE_PRESSED, COLOR_BG_TEXT_PR);
    lv_style_set_text_font(&styles->ddlist_sel, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->ddlist_sel, LV_STATE_DEFAULT, theme_fonts[0]);
#endif
}

static void roller_init(void)
{
#if LV_USE_ROLLER != 0
    style_init_reset(&styles->roller_bg);
    myCombineStyle(&styles->roller_bg, &styles->bg, NULL, NULL);
    lv_style_set_text_line_space(&styles->roller_bg, LV_STATE_DEFAULT, LV_DPX(25));
    lv_style_set_text_font(&styles->roller_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->roller_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_text_color(&styles->roller_bg, LV_STATE_DISABLED, lv_color_mix(LV_COLOR_BLACK, COLOR_MY_GREY, LV_OPA_30));

    style_init_reset(&styles->roller_sel);
    lv_style_set_bg_opa(&styles->roller_sel, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->roller_sel, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_text_color(&styles->roller_sel, LV_STATE_DEFAULT, theme_colors[29]);
    lv_style_set_text_font(&styles->roller_sel, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->roller_sel, LV_STATE_DEFAULT, theme_fonts[0]);
#endif
}

static void tabview_init(void)
{
#if LV_USE_TABVIEW != 0
    style_init_reset(&styles->tabview_bg);
    myCombineStyle(&styles->tabview_bg, &styles->scr, NULL, NULL);
#endif
}

static void tileview_init(void)
{
#if LV_USE_TILEVIEW != 0
    style_init_reset(&styles->tileview_bg);
    myCombineStyle(&styles->tileview_bg, &styles->scr, NULL, NULL);
#endif
}

static void table_init(void)
{
#if LV_USE_TABLE != 0
    style_init_reset(&styles->table_bg);
    myCombineStyle(&styles->table_bg, &styles->bg, NULL, NULL);

    style_init_reset(&styles->table_cell);
    lv_style_set_bg_color(&styles->table_cell, LV_STATE_DEFAULT, COLOR_BG);
    lv_style_set_bg_opa(&styles->table_cell, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_border_color(&styles->table_cell, LV_STATE_DEFAULT, COLOR_BG_BORDER);
    lv_style_set_border_width(&styles->table_cell, LV_STATE_DEFAULT, 1);
    lv_style_set_border_side(&styles->table_cell, LV_STATE_DEFAULT, LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_BOTTOM);
    lv_style_set_pad_left(&styles->table_cell, LV_STATE_DEFAULT, 5);
    lv_style_set_pad_right(&styles->table_cell, LV_STATE_DEFAULT, 5);
    lv_style_set_pad_top(&styles->table_cell, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&styles->table_cell, LV_STATE_DEFAULT, 0);
    lv_style_set_text_font(&styles->table_cell, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->table_cell, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->table_cell_row);
    lv_style_copy(&styles->table_cell_row, &styles->table_cell);
    lv_style_set_bg_color(&styles->table_cell_row, LV_STATE_DEFAULT, IS_LIGHT ? lv_color_hex(0x6BC7D1) : lv_color_hex(0x20AFBD));

    style_init_reset(&styles->table_cell_col);
    lv_style_copy(&styles->table_cell_col, &styles->table_cell);
    lv_style_set_bg_color(&styles->table_cell_col, LV_STATE_DEFAULT, IS_LIGHT ? lv_color_hex(0xB6E3E7) : lv_color_hex(0x6BC7D1));
#endif
}

static void win_init(void)
{
#if LV_USE_WIN != 0
    style_init_reset(&styles->win_bg);
    myCombineStyle(&styles->win_bg, &styles->scr, NULL, NULL);

    style_init_reset(&styles->win_header);
    myCombineStyle(&styles->win_header, &styles->tabview_btns_bg, NULL, NULL);

    style_init_reset(&styles->win_btn);
    myCombineStyle(&styles->win_btn, &styles->tabview_btns, NULL, NULL);
#endif
}

static void qtable_init(void)
{
#if LV_USE_QTABLE != 0
    style_init_reset(&styles->qtable_bg);
    myCombineStyle(&styles->qtable_bg, &styles->bg, NULL, NULL);

    style_init_reset(&styles->qtable_cell);
    lv_style_set_bg_color(&styles->qtable_cell, LV_STATE_DEFAULT, COLOR_BG);
    lv_style_set_bg_opa(&styles->qtable_cell, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_border_color(&styles->qtable_cell, LV_STATE_DEFAULT, COLOR_BG_BORDER);
    lv_style_set_border_width(&styles->qtable_cell, LV_STATE_DEFAULT, 1);
    lv_style_set_border_side(&styles->qtable_cell, LV_STATE_DEFAULT, LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_BOTTOM);
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
    lv_style_set_bg_opa(&styles->tabview_btns_bg, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->tabview_btns_bg, LV_STATE_DEFAULT, COLOR_BG);
    lv_style_set_text_color(&styles->tabview_btns_bg, LV_STATE_DEFAULT, COLOR_SCR_TEXT);
    lv_style_set_image_recolor(&styles->tabview_btns_bg, LV_STATE_DEFAULT, theme_colors[48]);
    lv_style_set_pad_top(&styles->tabview_btns_bg, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_pad_left(&styles->tabview_btns_bg, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_pad_right(&styles->tabview_btns_bg, LV_STATE_DEFAULT, LV_DPX(7));
    lv_style_set_text_font(&styles->tabview_btns_bg, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->tabview_btns_bg, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->tabview_btns);
    lv_style_set_bg_opa(&styles->tabview_btns, LV_STATE_PRESSED, LV_OPA_50);
    lv_style_set_bg_color(&styles->tabview_btns, LV_STATE_PRESSED, theme_colors[48]);
    lv_style_set_text_color(&styles->tabview_btns, LV_STATE_CHECKED, COLOR_SCR_TEXT);
    lv_style_set_pad_top(&styles->tabview_btns, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_pad_bottom(&styles->tabview_btns, LV_STATE_DEFAULT, LV_DPX(20));
    lv_style_set_text_color(&styles->tabview_btns, LV_STATE_FOCUSED, theme_colors[0]);
    lv_style_set_text_color(&styles->tabview_btns, LV_STATE_EDITED, theme_colors[1]);
    lv_style_set_text_font(&styles->tabview_btns, LV_STATE_DEFAULT, theme_fonts[0]);
    lv_style_set_value_font(&styles->tabview_btns, LV_STATE_DEFAULT, theme_fonts[0]);

    style_init_reset(&styles->tabview_indic);
    lv_style_set_bg_opa(&styles->tabview_indic, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->tabview_indic, LV_STATE_DEFAULT, theme_colors[0]);
    lv_style_set_bg_color(&styles->tabview_indic, LV_STATE_EDITED, theme_colors[1]);
    lv_style_set_size(&styles->tabview_indic, LV_STATE_DEFAULT, LV_DPX(5));
    lv_style_set_radius(&styles->tabview_indic, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);

    style_init_reset(&styles->tabview_page_scrl);
    lv_style_set_pad_top(&styles->tabview_page_scrl, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_pad_bottom(&styles->tabview_page_scrl, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_pad_left(&styles->tabview_page_scrl, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_pad_right(&styles->tabview_page_scrl, LV_STATE_DEFAULT, PAD_DEF);
    lv_style_set_pad_inner(&styles->tabview_page_scrl, LV_STATE_DEFAULT, PAD_DEF);
#endif
}

static void myCombineStyle2(lv_style_t* dec, lv_style_t* src) {
    if (src != NULL) {
        if (src == &styles->scr) {
            lv_style_set_bg_opa(dec, LV_STATE_DEFAULT, LV_OPA_COVER);
            lv_style_set_bg_color(dec, LV_STATE_DEFAULT, COLOR_SCR);
            lv_style_set_text_color(dec, LV_STATE_DEFAULT, COLOR_SCR_TEXT);
            lv_style_set_value_color(dec, LV_STATE_DEFAULT, COLOR_SCR_TEXT);
            lv_style_set_text_sel_color(dec, LV_STATE_DEFAULT, theme_colors[28]);
            lv_style_set_text_sel_bg_color(dec, LV_STATE_DEFAULT, theme_colors[0]);
            lv_style_set_value_font(dec, LV_STATE_DEFAULT, theme_fonts[0]);
            lv_style_set_text_font(dec, LV_STATE_DEFAULT, theme_fonts[0]);
        }
        else if (src == &styles->bg) {
            lv_style_set_radius(dec, LV_STATE_DEFAULT, LV_DPX(8));
            lv_style_set_bg_opa(dec, LV_STATE_DEFAULT, LV_OPA_COVER);
            lv_style_set_bg_color(dec, LV_STATE_DEFAULT, COLOR_BG);
            lv_style_set_border_color(dec, LV_STATE_DEFAULT, COLOR_BG_BORDER);
            if ((theme.flags & LV_THEME_MATERIAL_FLAG_NO_FOCUS) == 0)
                lv_style_set_border_color(dec, LV_STATE_FOCUSED, theme_colors[0]);
            lv_style_set_border_color(dec, LV_STATE_EDITED, theme_colors[1]);
            lv_style_set_border_width(dec, LV_STATE_DEFAULT, BORDER_WIDTH);
            lv_style_set_border_post(dec, LV_STATE_DEFAULT, true);
            lv_style_set_text_color(dec, LV_STATE_DEFAULT, COLOR_BG_TEXT);
            lv_style_set_value_font(dec, LV_STATE_DEFAULT, theme_fonts[0]);
            lv_style_set_value_color(dec, LV_STATE_DEFAULT, COLOR_BG_TEXT);
            lv_style_set_image_recolor(dec, LV_STATE_DEFAULT, theme_colors[30]);
            lv_style_set_line_color(dec, LV_STATE_DEFAULT, theme_colors[30]);
            lv_style_set_line_width(dec, LV_STATE_DEFAULT, 1);
            lv_style_set_pad_left(dec, LV_STATE_DEFAULT, PAD_DEF + BORDER_WIDTH);
            lv_style_set_pad_right(dec, LV_STATE_DEFAULT, PAD_DEF + BORDER_WIDTH);
            lv_style_set_pad_top(dec, LV_STATE_DEFAULT, PAD_DEF + BORDER_WIDTH);
            lv_style_set_pad_bottom(dec, LV_STATE_DEFAULT, PAD_DEF + BORDER_WIDTH);
            lv_style_set_pad_inner(dec, LV_STATE_DEFAULT, PAD_DEF);
            lv_style_set_transition_time(dec, LV_STATE_DEFAULT, TRANSITION_TIME);
            lv_style_set_transition_prop_6(dec, LV_STATE_DEFAULT, LV_STYLE_BORDER_COLOR);
            lv_style_set_text_font(dec, LV_STATE_DEFAULT, theme_fonts[0]);
        }
        else if (src == &styles->bg_click) {
            lv_style_set_bg_color(dec, LV_STATE_PRESSED, COLOR_BG_PR);
            lv_style_set_bg_color(dec, LV_STATE_CHECKED, COLOR_BG_CHK);
            lv_style_set_bg_color(dec, LV_STATE_PRESSED | LV_STATE_CHECKED, COLOR_BG_PR_CHK);
            lv_style_set_bg_color(dec, LV_STATE_DISABLED, COLOR_BG_DIS);
            lv_style_set_border_width(dec, LV_STATE_CHECKED, 0);
            lv_style_set_border_color(dec, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_darken(theme_colors[0], LV_OPA_20));
            lv_style_set_border_color(dec, LV_STATE_PRESSED, COLOR_BG_BORDER_PR);
            lv_style_set_border_color(dec, LV_STATE_CHECKED, COLOR_BG_BORDER_CHK);
            lv_style_set_border_color(dec, LV_STATE_PRESSED | LV_STATE_CHECKED, COLOR_BG_BORDER_CHK_PR);
            lv_style_set_border_color(dec, LV_STATE_DISABLED, COLOR_BG_BORDER_DIS);
            lv_style_set_text_color(dec, LV_STATE_PRESSED, COLOR_BG_TEXT_PR);
            lv_style_set_text_color(dec, LV_STATE_CHECKED, COLOR_BG_TEXT_CHK);
            lv_style_set_text_color(dec, LV_STATE_PRESSED | LV_STATE_CHECKED, COLOR_BG_TEXT_CHK_PR);
            lv_style_set_text_color(dec, LV_STATE_DISABLED, COLOR_BG_TEXT_DIS);
            lv_style_set_image_recolor(dec, LV_STATE_PRESSED, theme_colors[30]);
            lv_style_set_image_recolor(dec, LV_STATE_CHECKED, COLOR_BG_TEXT_CHK);
            lv_style_set_image_recolor(dec, LV_STATE_PRESSED | LV_STATE_CHECKED, COLOR_BG_TEXT_CHK_PR);
            lv_style_set_image_recolor(dec, LV_STATE_DISABLED, COLOR_BG_TEXT_DIS);
            lv_style_set_transition_prop_5(dec, LV_STATE_DEFAULT, LV_STYLE_BG_COLOR);
            lv_style_set_text_font(dec, LV_STATE_DEFAULT, theme_fonts[0]);
            lv_style_set_value_font(dec, LV_STATE_DEFAULT, theme_fonts[0]);
        }
        else if (src == &styles->pad_small) {
            lv_style_int_t pad_small_value = lv_disp_get_size_category(NULL) <= LV_DISP_MEDIUM_LIMIT ? LV_DPX(10) : LV_DPX(20);
            lv_style_set_pad_left(dec, LV_STATE_DEFAULT, pad_small_value);
            lv_style_set_pad_right(dec, LV_STATE_DEFAULT, pad_small_value);
            lv_style_set_pad_top(dec, LV_STATE_DEFAULT, pad_small_value);
            lv_style_set_pad_bottom(dec, LV_STATE_DEFAULT, pad_small_value);
            lv_style_set_pad_inner(dec, LV_STATE_DEFAULT, pad_small_value);
        }
    }
}

static void myCombineStyle(lv_style_t* dec, lv_style_t* src1, lv_style_t* src2, lv_style_t* src3) {
    // Now, dec is empty, src1 can be any style, but src2 and src3 only is one of 4 (src, bg, bg_click, pad_small)
    if (src1 != NULL) lv_style_copy(dec, src1);
    if (src2 != NULL) myCombineStyle2(dec, src2);
    if (src3 != NULL) myCombineStyle2(dec, src3);
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

void initColor(double* colors) {
    int i;
    double data;
    for (i = 0; i < PALETTESIZE; i++) {
        data = _org_color[i] = colors[i];
        if (data < 0)
            _palette[i] = lv_color_hex((uint32_t)colors[(int)((data * -1) - 1)]);
        else
            _palette[i] = lv_color_hex((uint32_t)data);
    }
}

void Palette(void) {
    double light_palette[PALETTESIZE] = {
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,

    0xeaeff3,
    0xffffff,
    0xeeeeee,
    0x3b3e42,
    0x3b3e42,
    0xffffff,
    0x3b3e42,
    0xd6dde3,
    0x3b3e42,
    0xaaaaaa,
    0xd6dde3,
    0xffffff,
    0x888888,
    0xcccccc,
    0xd4d7d9,
    0x666666,
    0xaaaaaa,
    0x464b5b,
    0x888888,
    0x44d1b6,
    0xc0c0c0,
    0xcccfd1,
    0xff0000,//sw_knob
    0xffffff,
    0x888888 };

    double dark_palette[PALETTESIZE] = {
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,
    0x01a2b1,
    0xff0000,

    0x444b5a,
    0x586273,
    0x494f57,
    0xffffff,
    0xe7e9ec,
    0xffffff,
    0xffffff,
    0x494f57,
    0x5f656e,
    0x999999,
    0x5f656e,
    0x586273,
    0x888888,
    0x888888,
    0x41404f,
    0xffffff,
    0xa1adbd,
    0xffffff,
    0x888888,
    0x44d1b6,
    0x999999,
    0x777f85,
    0xffffff,//sw_knob
    0xffffff,
    0x888888 };

    lv_font_t* theme_fonts[6] = {
    &lv_font_montserrat_18, // normal
    &lv_font_montserrat_34, // large
    &lv_font_montserrat_14,
    &lv_font_montserrat_18,
    &lv_font_montserrat_18,
    &lv_font_montserrat_18 };

    // lv_color_t * c = (flag == LV_THEME_MATERIAL_FLAG_DARK) ? &dark_palette[0] : &light_palette[0];
    // lv_theme_t * t = lv_theme_material86_init(c, fonts);
    // lv_theme_change(t);
}

int v7_get_font_id_part(lv_obj_t* obj, uint8_t part);
int v7_get_font_id(lv_obj_t* obj);
void v7_set_font_id_part(lv_obj_t* obj, uint8_t part, int id);
int v7_set_font_id(lv_obj_t* obj, int id);
int v7_set_color_id(lv_obj_t* obj, int groupid);
int v7_get_color_id(lv_obj_t* obj);
double v7_get_theme_color(int group, int order);
lv_font_t* v7_get_theme_font(int index);

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
lv_theme_t* lv_theme_material86_init(double* colors, lv_font_t** fonts)
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

        for (i = 0; i < FONTSIZE; i++) {
            lv_style_init(&_font[i]);
            style_font[i] = &_font[i];
        }

        theme.apply_xcb = NULL;
        theme.apply_cb = theme_apply;

        theme.user_data = (void*)prepare_theme_style_ptrs();
    }

    theme.flags = (colors[46] != -1 ? LV_THEME_MATERIAL_FLAG_DARK : LV_THEME_MATERIAL_FLAG_LIGHT);

    if (getPreThemeID() != v7_theme_ID) {
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
    theme.color_secondary = theme_colors[1];
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

    registerThemeTools(v7_theme_ID, v7_get_font_id_part, v7_get_font_id, v7_set_font_id_part, v7_set_font_id, v7_set_color_id, v7_get_color_id, v7_get_theme_color, v7_get_theme_font);

    update_id_color(); // if user call set_color_id(), this will update all node from prevTheme to NowTheme

    inited = true;
    setThemeID(v7_theme_ID); // call it after update_id_color();

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

int v7_get_font_id_part(lv_obj_t* obj, uint8_t part) {
    int list_id, font_id;
    lv_style_list_t* list = lv_obj_get_style_list(obj, part);
    if (list != NULL && isMyFont(list, &list_id, &font_id))
        return font_id;
    return -1; // default
}

int v7_get_font_id(lv_obj_t* obj) { // always get from part 0
    return v7_get_font_id_part(obj, 0);
}

void v7_set_font_id_part(lv_obj_t* obj, uint8_t part, int id) {

}

int v7_set_font_id(lv_obj_t* obj, int id) { // set all part
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

    switch (id) {
    case -1: // no find anything
        changed = false;
        break;
    case 0: // scr
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_text_sel_bg_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 1: // bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid * 2]);
        lv_style_set_border_color(p->style, LV_STATE_EDITED, theme_colors[groupid * 2 + 1]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 2: // bg_click
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED, theme_colors[groupid * 2]);
        lv_style_set_bg_color(p->style, LV_STATE_PRESSED | LV_STATE_CHECKED, lv_color_darken(theme_colors[groupid * 2], LV_OPA_20));
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_darken(theme_colors[groupid * 2], LV_OPA_20));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 4: case 13: // btn, btnm_btn
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_PRESSED, lv_color_mix(theme_colors[groupid * 2], COLOR_BTN, LV_OPA_20));
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED, theme_colors[groupid * 2]);
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED | LV_STATE_PRESSED, lv_color_darken(theme_colors[groupid * 2], LV_OPA_30));
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        lv_style_set_border_color(p->style, LV_STATE_PRESSED, theme_colors[groupid * 2]);
        lv_style_set_outline_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        lv_style_set_outline_color(p->style, LV_STATE_EDITED, theme_colors[groupid * 2 + 1]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 10: // bar_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_outline_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        lv_style_set_outline_color(p->style, LV_STATE_EDITED, theme_colors[groupid * 2 + 1]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 11: // bar_indic
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED, lv_color_mix(theme_colors[groupid * 2], COLOR_MY_GREY, LV_OPA_50));
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(theme_colors[groupid * 2], COLOR_MY_GREY, LV_OPA_50));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 33: // led
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, lv_color_lighten(theme_colors[groupid * 2], LV_OPA_30));
        lv_style_set_shadow_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 34: // lmeter
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        lv_style_set_scale_grad_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 28: // gauge_main
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_scale_end_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 29: // gauge_strong
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_scale_end_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 7: // arc_indic
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_line_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        lv_style_set_line_color(p->style, LV_STATE_DISABLED, lv_color_mix(theme_colors[groupid * 2], COLOR_MY_GREY, LV_OPA_50));
        lv_style_set_line_color(p->style, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(theme_colors[groupid * 2], COLOR_MY_GREY, LV_OPA_50));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 15: // calendar_date_nums
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid * 2]);
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED, theme_colors[groupid * 2]);
        lv_style_set_border_color(p->style, LV_STATE_CHECKED, theme_colors[groupid * 2]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 19: // cpicker_indic
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_border_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid * 2]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 23: // cb_bg
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_outline_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 24: // cb_bullet
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED, theme_colors[groupid * 2]);
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED | LV_STATE_PRESSED, lv_color_darken(theme_colors[groupid * 2], LV_OPA_30));
        lv_style_set_bg_color(p->style, LV_STATE_PRESSED, IS_LIGHT ? lv_color_mix(theme_colors[groupid * 2], COLOR_BTN, LV_OPA_20) : lv_color_mix(theme_colors[groupid * 2], COLOR_BTN, LV_OPA_30));
        lv_style_set_border_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        lv_style_set_border_color(p->style, LV_STATE_PRESSED, theme_colors[groupid * 2]);
        lv_style_set_border_color(p->style, LV_STATE_DISABLED, lv_color_mix(theme_colors[groupid * 2], COLOR_MY_GREY, LV_OPA_50));
        lv_style_set_bg_color(p->style, LV_STATE_DISABLED | LV_STATE_CHECKED, lv_color_mix(theme_colors[groupid * 2], COLOR_MY_GREY, LV_OPA_50));
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 48: // spinbox_cursor
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 35: // list_bg : 只是存放 color id，沒其他作用
        if (addit) p = getStyleGroup(styleColorID);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 36: // list_btn
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED, theme_colors[groupid * 2]);
        lv_style_set_bg_color(p->style, LV_STATE_CHECKED | LV_STATE_PRESSED, lv_color_darken(theme_colors[groupid * 2], LV_OPA_20));

        if (_org_color[43] < -1.0) { // get -2
            lv_style_set_outline_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid * 2 + 1]);
            lv_style_set_outline_color(p->style, LV_STATE_CHECKED, theme_colors[groupid * 2 + 1]);
        } else if (_org_color[43] < 0.0) { // get -1
            lv_style_set_outline_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid * 2]);
            lv_style_set_outline_color(p->style, LV_STATE_CHECKED, theme_colors[groupid * 2]);
        } else {
            lv_style_set_outline_color(p->style, LV_STATE_FOCUSED, theme_colors[43]);
            lv_style_set_outline_color(p->style, LV_STATE_CHECKED, theme_colors[43]);
        }

        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 27: // ddlist_sel
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 44: // roller_sel
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 54: // tabview_btns
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_text_color(p->style, LV_STATE_FOCUSED, theme_colors[groupid * 2]);
        lv_style_set_text_color(p->style, LV_STATE_EDITED, theme_colors[groupid * 2 + 1]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 56: // tabview_indic
        if (addit) p = getStyleGroup(styleColorID);
        lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        lv_style_set_bg_color(p->style, LV_STATE_EDITED, theme_colors[groupid * 2 + 1]);
        p->groupid = groupid;
        p->themeid = id;
        if (addit) _lv_style_list_add_style(list, p->style);
        break;
    case 45: // slider_knob
        // special case: light -> change with primary color, but Dark -> fix 0xFFFFFF color
        if (addit) p = getStyleGroup(styleColorID);
        if (_org_color[46] < 0) lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, theme_colors[groupid * 2]);
        else                    lv_style_set_bg_color(p->style, LV_STATE_DEFAULT, theme_colors[46]);
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

int v7_set_color_id(lv_obj_t* obj, int groupid) {
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
                mod = setNewColor(list, false, groupid, &style_color[fid], &SCID); // update color, no add node
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

int v7_get_color_id(lv_obj_t* obj) {
    int part, list_id, font_id;
    for (part = 0; part < 100; part++) {
        lv_style_list_t* list = lv_obj_get_style_list(obj, part);
        if (list != NULL && isMyColor(list, &list_id, &font_id))
            return style_color[font_id].groupid;
    }
    return -1; // use default theme
}

double v7_get_theme_color(int group, int order) {
    if (!inited || group < 0 || order < 0 || order > 1) return -1;
    if ((group * 2 + order) >= 24) return -1;
    return _org_color[group * 2 + order];
}

lv_font_t* v7_get_theme_font(int index) {
    if (index >= FONTSIZE || index < 0) return NULL;
    return theme_fonts[index];
}

static void report_style_mod_core(lv_obj_t* obj)
{
    int prevGroupID, preFontID;

    if ((obj == lv_layer_top()) || (obj == lv_layer_sys())) return;


    if (getPreThemeID() != v7_theme_ID)
        prevGroupID = clearPreColorStyle(obj);
    else
        prevGroupID = v7_get_color_id(obj); // dont clear node and get it's ID

    if (prevGroupID != -1) v7_set_color_id(obj, prevGroupID);

    if (getPreThemeID() != v7_theme_ID)
        preFontID = clearPreFontStyle(obj);
    else
        preFontID = v7_get_font_id(obj); // dont clear node and get it's ID

    if (preFontID != -1) v7_set_font_id(obj, preFontID);


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
        _lv_style_list_add_style(list, &styles->mbox_btn_bg);

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
            } else if (idx == 3) {
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
        _lv_style_list_add_style(list, &styles->spinbox_btn);

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

