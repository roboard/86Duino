/**
 * @file lv_gifenc.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "gifdec.h"

/*********************
 *      DEFINES
 *********************/
#define LV_OBJX_NAME "lv_gif"

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_img_ext_t img_ext;
    gd_GIF *gif;
    lv_task_t * task;
    lv_img_dsc_t imgdsc;
}lv_gif_ext_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void next_frame_task_cb(lv_task_t * t);
static lv_res_t lv_gif_signal(lv_obj_t * btn, lv_signal_t sign, void * param);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_signal_cb_t ancestor_signal;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_gif_create_from_file(lv_obj_t * parent, const char * path)
{

    lv_obj_t * img = lv_img_create(parent, NULL);
    lv_gif_ext_t * ext = lv_obj_allocate_ext_attr(img, sizeof(lv_gif_ext_t));
    LV_ASSERT_MEM(ext);


    if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(img);
    lv_obj_set_signal_cb(img, lv_gif_signal);

    ext->gif = gd_open_gif_file(path);
    if(ext->gif == NULL) return img;
    
    ext->imgdsc.data = ext->gif->palette;
    ext->imgdsc.header.always_zero = 0;
    ext->imgdsc.header.cf = LV_IMG_CF_INDEXED_8BIT;
    ext->imgdsc.header.h = ext->gif->height;
    ext->imgdsc.header.w = ext->gif->width;

    lv_img_set_src(img, &ext->imgdsc);

    ext->task = lv_task_create(next_frame_task_cb, 10, LV_TASK_PRIO_HIGH, img);

    return img;
}

lv_obj_t * lv_gif_create_from_data(lv_obj_t * parent, const void * data)
{

    lv_obj_t * img = lv_img_create(parent, NULL);
    lv_gif_ext_t * ext = lv_obj_allocate_ext_attr(img, sizeof(lv_gif_ext_t));
    LV_ASSERT_MEM(ext);


    if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(img);
    lv_obj_set_signal_cb(img, lv_gif_signal);

    ext->gif = gd_open_gif_data(data);
    if(ext->gif == NULL) return img;

    ext->imgdsc.data = ext->gif->palette;
    ext->imgdsc.header.always_zero = 0;
    ext->imgdsc.header.cf = LV_IMG_CF_INDEXED_8BIT;
    ext->imgdsc.header.h = ext->gif->height;
    ext->imgdsc.header.w = ext->gif->width;

    lv_img_set_src(img, &ext->imgdsc);

    ext->task = lv_task_create(next_frame_task_cb, 10, LV_TASK_PRIO_HIGH, img);

    return img;
}

void lv_gif_restart(lv_obj_t * gif)
{
    lv_gif_ext_t * ext = lv_obj_get_ext_attr(gif);
    gd_rewind(ext->gif);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void next_frame_task_cb(lv_task_t * t)
{
    static uint32_t counter = 0;
    if(counter > 0) {
        counter --;
        return;
    }

    lv_obj_t * img = t->user_data;
    lv_gif_ext_t * ext = lv_obj_get_ext_attr(img);

    int has_next = gd_get_frame(ext->gif);
    if(has_next == 0) {
        lv_event_send(img, LV_EVENT_LEAVE, NULL);
    }

    lv_obj_invalidate(img);

    counter = ext->gif->gce.delay;
}

/**
 * Signal function of the image
 * @param btn pointer to a button object
 * @param sign a signal type from lv_signal_t enum
 * @param param pointer to a signal specific variable
 * @return LV_RES_OK: the object is not deleted in the function; LV_RES_INV: the object is deleted
 */
static lv_res_t lv_gif_signal(lv_obj_t * img, lv_signal_t sign, void * param)
{
    lv_res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(img, sign, param);
    if(res != LV_RES_OK) return res;
    if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);

    if(sign == LV_SIGNAL_CLEANUP) {
        lv_gif_ext_t * ext = lv_obj_get_ext_attr(img);
        gd_close_gif(ext->gif);
        lv_task_del(ext->task);
    }

    return LV_RES_OK;
}
