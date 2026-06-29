/**
 * @file lvgl.h
 * Include all LittleV GL related headers
 */
 
#ifndef LVGL_H
#define LVGL_H

#ifdef __cplusplus
extern "C" {
#endif


/***************************
 * CURRENT VERSION OF LVGL
 ***************************/
#define LVGL_VERSION_MAJOR 7
#define LVGL_VERSION_MINOR 11
#define LVGL_VERSION_PATCH 0
#define LVGL_VERSION_INFO ""

/*********************
 *      INCLUDES
 *********************/

#include "src/lv_log.h"
#include "src/lv_task.h"
#include "src/lv_math.h"
#include "src/lv_async.h"

#include "src/lv_hal.h"

#include "src/lv_obj.h"
#include "src/lv_group.h"
#include "src/lv_indev.h"

#include "src/lv_refr.h"
#include "src/lv_disp.h"

#include "src/lv_theme.h"

#include "src/lv_font.h"
#include "src/lv_font_loader.h"
#include "src/lv_font_fmt_txt.h"
#include "src/lv_printf.h"

#include "src/lv_btn.h"
#include "src/lv_imgbtn.h"
#include "src/lv_img.h"
#include "src/lv_label.h"
#include "src/lv_line.h"
#include "src/lv_page.h"
#include "src/lv_cont.h"
#include "src/lv_list.h"
#include "src/lv_chart.h"
#include "src/lv_table.h"
#include "src/lv_checkbox.h"
#include "src/lv_cpicker.h"
#include "src/lv_bar.h"
#include "src/lv_slider.h"
#include "src/lv_led.h"
#include "src/lv_btnmatrix.h"
#include "src/lv_keyboard.h"
#include "src/lv_dropdown.h"
#include "src/lv_roller.h"
#include "src/lv_textarea.h"
#include "src/lv_canvas.h"
#include "src/lv_win.h"
#include "src/lv_tabview.h"
#include "src/lv_tileview.h"
#include "src/lv_msgbox.h"
#include "src/lv_objmask.h"
#include "src/lv_gauge.h"
#include "src/lv_linemeter.h"
#include "src/lv_switch.h"
#include "src/lv_arc.h"
#include "src/lv_spinner.h"
#include "src/lv_calendar.h"
#include "src/lv_spinbox.h"

#include "src/lv_img_cache.h"

#include "lv_api_map.h"
#include "lv_theme_tools.h"
#include "lv_qtable.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

/** Gives 1 if the x.y.z version is supported in the current version
 * Usage:
 *
 * - Require v6
 * #if LV_VERSION_CHECK(6,0,0)
 *   new_func_in_v6();
 * #endif
 *
 *
 * - Require at least v5.3
 * #if LV_VERSION_CHECK(5,3,0)
 *   new_feature_from_v5_3();
 * #endif
 *
 *
 * - Require v5.3.2 bugfixes
 * #if LV_VERSION_CHECK(5,3,2)
 *   bugfix_in_v5_3_2();
 * #endif
 *
 * */
#define LV_VERSION_CHECK(x,y,z) (x == LVGL_VERSION_MAJOR && (y < LVGL_VERSION_MINOR || (y == LVGL_VERSION_MINOR && z <= LVGL_VERSION_PATCH)))


#ifdef __cplusplus
}
#endif

#endif /*LVGL_H*/
