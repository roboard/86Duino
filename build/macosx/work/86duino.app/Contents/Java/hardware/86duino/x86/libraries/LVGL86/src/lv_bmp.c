/**
 * @file lv_bmp.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

#include <stdio.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    FILE * f;
    unsigned int px_offset;
    int px_width;
    int px_height;
    unsigned int bpp;
    int row_size_bytes;
} bmp_dsc_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_res_t decoder_info(struct _lv_img_decoder * decoder, const void * src, lv_img_header_t * header);
static lv_res_t decoder_open(lv_img_decoder_t * dec, lv_img_decoder_dsc_t * dsc);


static lv_res_t decoder_read_line(struct _lv_img_decoder * decoder, struct _lv_img_decoder_dsc * dsc,
                                                 lv_coord_t x, lv_coord_t y, lv_coord_t len, uint8_t * buf);

static void decoder_close(lv_img_decoder_t * dec, lv_img_decoder_dsc_t * dsc);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_bmp_init(void)
{

    lv_img_decoder_t * dec = lv_img_decoder_create();
    lv_img_decoder_set_info_cb(dec, decoder_info);
    lv_img_decoder_set_open_cb(dec, decoder_open);
    lv_img_decoder_set_read_line_cb(dec, decoder_read_line);
    lv_img_decoder_set_close_cb(dec, decoder_close);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Get info about a PNG image
 * @param src can be file name or pointer to a C array
 * @param header store the info here
 * @return LV_RES_OK: no error; LV_RES_INV: can't get the info
 */
static lv_res_t decoder_info(struct _lv_img_decoder * decoder, const void * src, lv_img_header_t * header)
{
    (void) decoder; /*Unused*/
     lv_img_src_t src_type = lv_img_src_get_type(src);          /*Get the source type*/

     /*If it's a BMP file...*/
     if(src_type == LV_IMG_SRC_FILE) {
         const char * fn = src;
         if(!strcmp(&fn[strlen(fn) - 3], "bmp")) {              /*Check the extension*/
             /*Save the data in the header*/
             FILE* file;
             file = fopen(fn, "rb" );
             if(file == NULL) return LV_RES_INV;
             uint8_t headers[54];
             fread(headers, 1 , 54, file);
             header->w = *(int *)(headers + 18);
             header->h = *(int *)(headers + 22);
             fclose(file);
             header->always_zero = 0;
#if LV_COLOR_DEPTH == 32
             uint16_t bpp;
             memcpy(&bpp, header + 28, 2);
             header->cf = bpp == 32 ? LV_IMG_CF_TRUE_COLOR_ALPHA : LV_IMG_CF_TRUE_COLOR;
#else
             header->cf = LV_IMG_CF_TRUE_COLOR;
#endif
             return LV_RES_OK;
         }
     }
     /* BMP file as data not supported for simplicity.
      * Convert them to LVGL compatible C arrays directly. */
     else if(src_type == LV_IMG_SRC_VARIABLE) {
         return LV_RES_INV;
     }

     return LV_RES_INV;         /*If didn't succeeded earlier then it's an error*/
}


/**
 * Open a PNG image and return the decided image
 * @param src can be file name or pointer to a C array
 * @param style style of the image object (unused now but certain formats might use it)
 * @return pointer to the decoded image or  `LV_IMG_DECODER_OPEN_FAIL` if failed
 */
static lv_res_t decoder_open(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc)
{

    (void) decoder; /*Unused*/

    /*If it's a PNG file...*/
    if(dsc->src_type == LV_IMG_SRC_FILE) {
        const char * fn = dsc->src;

        if(strcmp(&fn[strlen(fn) - 3], "bmp")) return LV_RES_INV;       /*Check the extension*/

        bmp_dsc_t b;
        memset(&b, 0x00, sizeof(b));

        b.f = fopen(fn, "rb" );
        if(b.f == NULL) return LV_RES_INV;

        uint8_t header[54];
        fread(header, 1 , 54, b.f);

        if (0x42 != header[0] || 0x4d != header[1]) {
            return LV_RES_INV;
        }

        uint32_t tmp;
        memcpy(&b.px_offset, header + 10, 4);
        memcpy(&b.px_width, header + 18, 4);
        memcpy(&b.px_height, header + 22, 4);
        memcpy(&b.bpp, header + 28, 2);
        b.row_size_bytes = (b.bpp * b.px_width) / 8;

        dsc->user_data = lv_mem_alloc(sizeof(bmp_dsc_t));
        LV_ASSERT_MEM(dsc->user_data);
        if(dsc->user_data == NULL) return LV_RES_INV;
        memcpy(dsc->user_data, &b, sizeof(b));

        dsc->img_data = NULL;
        return LV_RES_OK;
    }
    /* BMP file as data not supported for simplicity.
     * Convert them to LVGL compatible C arrays directly. */
    else if(dsc->src_type == LV_IMG_SRC_VARIABLE) {
        return LV_RES_INV;
    }

    return LV_RES_INV;    /*If not returned earlier then it failed*/
}


static lv_res_t decoder_read_line(struct _lv_img_decoder * decoder, struct _lv_img_decoder_dsc * dsc,
                                                 lv_coord_t x, lv_coord_t y, lv_coord_t len, uint8_t * buf)
{

    bmp_dsc_t * b = dsc->user_data;
    y = (b->px_height - 1) - y; /*BMP images are stored upside down*/
    uint32_t p = b->px_offset + b->row_size_bytes * y;
    p += x * (b->bpp / 8);
    fseek(b->f, p, SEEK_SET);
    fread(buf, 1, len * (b->bpp / 8), b->f);

#if LV_COLOR_DEPTH == 32
    if(b->bpp == 32) {
        lv_color32_t * bufc = (lv_color32_t*)buf;
        uint32_t i;
        for(i = 0; i < len; i++) {
            lv_color_t t = bufc[i];
            bufc[i].ch.red = t.ch.alpha;
            bufc[i].ch.green = t.ch.red;
            bufc[i].ch.blue = t.ch.green;
            bufc[i].ch.alpha = t.ch.blue;
        }
    }
#endif

    return LV_RES_OK;
}


/**
 * Free the allocated resources
 */
static void decoder_close(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc)
{
    (void) decoder; /*Unused*/
    bmp_dsc_t * b = dsc->user_data;
    fclose(b->f);
    lv_mem_free(dsc->user_data);

}
//
///**
// * If the display is not in 32 bit format (ARGB888) then covert the image to the current color depth
// * @param img the ARGB888 image
// * @param px_cnt number of pixels in `img`
// */
//static void convert_color_depth(uint8_t * img, uint32_t px_cnt)
//{

//
