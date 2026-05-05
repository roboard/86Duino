/**
 * @file lv_qrcode
 *
 */

#ifndef LV_QRCODE_H
#define LV_QRCODE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an empty QR code (an `lv_canvas`) object.
 * @param parent point to an object where to create the QR code
 * @param size width and height of the QR code
 * @param dark_color dark color of the QR code
 * @param light_color light color of the QR code
 * @return pointer to the created QR code object
 */
lv_obj_t * lv_qrcode_create(lv_obj_t * parent, lv_coord_t size, lv_color_t dark_color, lv_color_t light_color);

/**
 * Set the data of a QR code object
 * @param qrcode pointer to aQ code object
 * @param data data to display
 * @param data_len length of data in bytes
 * @return LV_RES_OK: if no error; LV_RES_INV: on error
 */
lv_res_t lv_qrcode_update(lv_obj_t * qrcode, const void * data, uint32_t data_len);

/**
 * Delete a QR code object
 * @param qrcode pointer to a QR code object
 */
void lv_qrcode_delete(lv_obj_t * qrcode);

// New API for 86Duino
lv_obj_t * lv_qrcode_update_size(lv_obj_t * obj, lv_coord_t size);
void* lv_qrcode_get_private_data(lv_obj_t * obj);
void lv_qrcode_set_private_data(lv_obj_t* obj, void* privateData);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_QRCODE_H*/
