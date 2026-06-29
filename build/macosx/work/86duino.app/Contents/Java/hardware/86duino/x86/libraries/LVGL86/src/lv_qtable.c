/**
 * @file lv_qtable.c
 *
 * lv_qtable is a widget that fixes the time complexity issue of lv_table and is faster than lv_table.
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "stdio.h"
#include "lv_qtable.h"
#if LV_USE_QTABLE != 0

#include "src/lv_indev.h"
#include "src/lv_debug.h"
#include "src/lv_txt.h"
#include "src/lv_txt_ap.h"
#include "src/lv_math.h"
#include "src/lv_printf.h"
#include "src/lv_draw_label.h"
#include "src/lv_theme.h"

/*********************
 *      DEFINES
 *********************/
#define LV_OBJX_NAME "lv_qtable"
#define USE_QUICK_REFR	1
#define USE_COL_BUFFER	1
#define CROP_MODE		1
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_design_res_t lv_qtable_design(lv_obj_t * qtable, const lv_area_t * clip_area, lv_design_mode_t mode);
static lv_res_t lv_qtable_signal(lv_obj_t * qtable, lv_signal_t sign, void * param);
static lv_style_list_t * lv_qtable_get_style(lv_obj_t * qtable, uint8_t part);
static lv_coord_t get_row_height(lv_obj_t * qtable, uint16_t row_id, const lv_font_t ** font,
                                 lv_style_int_t * letter_space, lv_style_int_t * line_space,
                                 lv_style_int_t * cell_left, lv_style_int_t * cell_right, lv_style_int_t * cell_top, lv_style_int_t * cell_bottom);
static void refr_size(lv_obj_t * qtable);

static void __quick_refr_height(lv_obj_t * qtable);
static void __quick_refr_width(lv_obj_t * qtable);
static void __quick_refr_row_height(lv_obj_t * qtable, uint16_t row);
static void __quick_refr_cell_height(lv_obj_t * qtable, uint16_t row, uint16_t col);
static lv_coord_t __qucik_get_row_height(lv_obj_t* qtable, uint16_t row, uint16_t col, const lv_font_t** font,
                                         lv_style_int_t* letter_space, lv_style_int_t* line_space,
                                         lv_style_int_t* cell_left, lv_style_int_t* cell_right, lv_style_int_t* cell_top, lv_style_int_t* cell_bottom);
static lv_coord_t __quick_lv_txt_get_height(const char * text, const lv_font_t * font, lv_coord_t letter_space, lv_coord_t line_space, lv_coord_t max_width, lv_txt_flag_t flag);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_signal_cb_t ancestor_signal;
static lv_design_cb_t ancestor_design;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a qtable object
 * @param par pointer to an object, it will be the parent of the new qtable
 * @param copy pointer to a qtable object, if not NULL then the new object will be copied from it
 * @return pointer to the created qtable
 */
lv_obj_t * lv_qtable_create(lv_obj_t * par, const lv_obj_t * copy)
{
    LV_LOG_TRACE("qtable create started");

    /*Create the ancestor of qtable*/
    lv_obj_t * qtable = lv_obj_create(par, copy);
    LV_ASSERT_MEM(qtable);
    if(qtable == NULL) return NULL;

    /*Allocate the qtable type specific extended data*/
    lv_qtable_ext_t * ext = lv_obj_allocate_ext_attr(qtable, sizeof(lv_qtable_ext_t));
    LV_ASSERT_MEM(ext);
    if(ext == NULL) {
        lv_obj_del(qtable);
        return NULL;
    }

    if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(qtable);
    if(ancestor_design == NULL) ancestor_design = lv_obj_get_design_cb(qtable);

    /*Initialize the allocated 'ext' */
    ext->cell_data     = NULL;
    ext->col_cnt       = 0;
    ext->row_cnt       = 0;
    ext->row_h         = NULL;
    ext->cell_types    = 1;

    uint16_t i;
    for(i = 0; i < LV_QTABLE_CELL_STYLE_CNT; i++) {
        lv_style_list_init(&ext->cell_style[i]);
    }

    for(i = 0; i < LV_QTABLE_COL_MAX; i++) {
        ext->col_w[i] = LV_DPI;
#if USE_COL_BUFFER
        ext->col_buf[i] = NULL;
		ext->col_buf_size[i] = 0;
#endif
    }

    /*The signal and design functions are not copied so set them here*/
    lv_obj_set_signal_cb(qtable, lv_qtable_signal);
    lv_obj_set_design_cb(qtable, lv_qtable_design);

    /*Init the new qtable qtable*/
    if(copy == NULL) {
        lv_theme_apply(qtable, LV_THEME_QTABLE);
    }
    /*Copy an existing qtable*/
    else {
        lv_qtable_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
        for(i = 0; i < LV_QTABLE_CELL_STYLE_CNT; i++) {
            lv_style_list_copy(&ext->cell_style[i], &copy_ext->cell_style[i]);
        }
        lv_qtable_set_row_cnt(qtable, copy_ext->row_cnt);
        lv_qtable_set_col_cnt(qtable, copy_ext->col_cnt);

        /*Refresh the style with new signal function*/
        lv_obj_refresh_style(qtable, LV_OBJ_PART_ALL, LV_STYLE_PROP_ALL);
    }

#if USE_QUICK_REFR
    __quick_refr_width(qtable);
    __quick_refr_height(qtable);
#endif
    LV_LOG_INFO("qtable created");
    return qtable;
}

/*=====================
 * Setter functions
 *====================*/
/**
 * Set the pointer of column buffer.
 * @param qtable pointer to a qTable object
 * @param col id of the column [0 .. col_cnt -1]
 * @param size size of the buffer
 * required after this function call
 */
bool lv_qtable_set_col_buf(lv_obj_t * qtable, uint16_t col, size_t size)
{
#if USE_COL_BUFFER
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    if (ext->col_buf_size[col] >= size * sizeof(char)) return false;
	ext->col_buf[col] = lv_mem_realloc(ext->col_buf[col], size * sizeof(char));
	LV_ASSERT_MEM(ext->col_buf[col]);
	if(ext->col_buf[col] == NULL) return false;
	
	ext->col_buf_size[col] = size * sizeof(char);

    return true;
#else
    return false;
#endif
}

/**
 * Set the value of a cell.
 * @param qtable pointer to a qTable object
 * @param row id of the row [0 .. row_cnt -1]
 * @param col id of the column [0 .. col_cnt -1]
 * @param txt text to display in the cell. It will be copied and saved so this variable is not
 * required after this function call.
 */
void lv_qtable_set_cell_value(lv_obj_t * qtable, uint16_t row, uint16_t col, const char * txt)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);
    LV_ASSERT_NULL(txt);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    if(col >= ext->col_cnt) {
        LV_LOG_WARN("lv_qtable_set_cell_value: invalid column");
        return;
    }

    /*Auto expand*/
    if(row >= ext->row_cnt) {
        lv_qtable_set_row_cnt(qtable, row + 1);
    }

    uint32_t cell = row * ext->col_cnt + col;
    lv_qtable_cell_format_t format;

    /*Save the format byte*/
    if(ext->cell_data[cell]) {
        format.format_byte = ext->cell_data[cell][0];
    }
    /*Initialize the format byte*/
    else {
        lv_bidi_dir_t base_dir = lv_obj_get_base_dir(qtable);
        if(base_dir == LV_BIDI_DIR_LTR) format.s.align = LV_LABEL_ALIGN_LEFT;
        else if(base_dir == LV_BIDI_DIR_RTL) format.s.align = LV_LABEL_ALIGN_RIGHT;
        else if(base_dir == LV_BIDI_DIR_AUTO)
#if LV_USE_BIDI
            format.s.align = _lv_bidi_detect_base_dir(txt);
#else
            format.s.align = LV_LABEL_ALIGN_LEFT;
#endif
        format.s.right_merge = 0;
        format.s.type        = 0;
        format.s.crop        = CROP_MODE;
    }

#if USE_COL_BUFFER
	if (ext->col_buf[col] && ext->col_buf_size[col])
	{
		uint16_t gap = ext->col_buf_size[col] / ext->row_cnt;
		if (ext->cell_data[cell] && (ext->cell_data[cell] < ext->col_buf[col] || ext->cell_data[cell] > ext->col_buf[col] + ext->col_buf_size[col]))
			lv_mem_free(ext->cell_data[cell]);
		
		ext->cell_data[cell] = ext->col_buf[col] + gap * row;
		
		strncpy(ext->cell_data[cell] + 1, txt, gap - 2);
		ext->cell_data[cell][gap - 1] = '\0';
	}
	else
	{
		ext->cell_data[cell] = lv_mem_realloc(ext->cell_data[cell], strlen(txt) + 2); /*+1: trailing '\0; +1: format byte*/
		LV_ASSERT_MEM(ext->cell_data[cell]);
		if(ext->cell_data[cell] == NULL) return;
		strcpy(ext->cell_data[cell] + 1, txt);  /*+1 to skip the format byte*/
	}
#else
	#if LV_USE_ARABIC_PERSIAN_CHARS
		/*Get the size of the Arabic text and process it*/
		size_t len_ap = _lv_txt_ap_calc_bytes_cnt(txt);
		ext->cell_data[cell] = lv_mem_realloc(ext->cell_data[cell], len_ap + 1);
		LV_ASSERT_MEM(ext->cell_data[cell]);
		if(ext->cell_data[cell] == NULL) return;

		_lv_txt_ap_proc(txt, &ext->cell_data[cell][1]);
	#else
		ext->cell_data[cell] = lv_mem_realloc(ext->cell_data[cell], strlen(txt) + 2); /*+1: trailing '\0; +1: format byte*/
		LV_ASSERT_MEM(ext->cell_data[cell]);
		if(ext->cell_data[cell] == NULL) return;
		strcpy(ext->cell_data[cell] + 1, txt);  /*+1 to skip the format byte*/
	#endif
#endif

    ext->cell_data[cell][0] = format.format_byte;
#if USE_QUICK_REFR
	__quick_refr_cell_height(qtable, row, col);
#else
    refr_size(qtable);
#endif
}

/**
 * Set the value of a cell.  Memory will be allocated to store the text by the qtable.
 * @param qtable pointer to a qTable object
 * @param row id of the row [0 .. row_cnt -1]
 * @param col id of the column [0 .. col_cnt -1]
 * @param fmt `printf`-like format
 */
void lv_qtable_set_cell_value_fmt(lv_obj_t * qtable, uint16_t row, uint16_t col, const char * fmt, ...)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);
    LV_ASSERT_STR(fmt);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    if(col >= ext->col_cnt) {
        LV_LOG_WARN("lv_qtable_set_cell_value: invalid column");
        return;
    }

    /*Auto expand*/
    if(row >= ext->row_cnt) {
        lv_qtable_set_row_cnt(qtable, row + 1);
    }

    uint32_t cell = row * ext->col_cnt + col;
    lv_qtable_cell_format_t format;

    /*Save the format byte*/
    if(ext->cell_data[cell]) {
        format.format_byte = ext->cell_data[cell][0];
    }
    /*Initialize the format byte*/
    else {
        lv_bidi_dir_t base_dir = lv_obj_get_base_dir(qtable);
        if(base_dir == LV_BIDI_DIR_LTR) format.s.align = LV_LABEL_ALIGN_LEFT;
        else if(base_dir == LV_BIDI_DIR_RTL) format.s.align = LV_LABEL_ALIGN_RIGHT;
        else if(base_dir == LV_BIDI_DIR_AUTO)
#if LV_USE_BIDI
            format.s.align = _lv_bidi_detect_base_dir(fmt);
#else
            format.s.align = LV_LABEL_ALIGN_LEFT;
#endif
        format.s.right_merge = 0;
        format.s.type        = 0;
        format.s.crop        = CROP_MODE;
    }

    va_list ap, ap2;
    va_start(ap, fmt);
    va_copy(ap2, ap);

    /*Allocate space for the new text by using trick from C99 standard section 7.19.6.12 */
    uint32_t len = lv_vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

	
#if USE_COL_BUFFER

	if (ext->col_buf[col] && ext->col_buf_size[col])
	{
		uint16_t gap = ext->col_buf_size[col] / ext->row_cnt;
		if (ext->cell_data[cell] && (ext->cell_data[cell] < ext->col_buf[col] || ext->cell_data[cell] > ext->col_buf[col] + ext->col_buf_size[col]))
			lv_mem_free(ext->cell_data[cell]);
		ext->cell_data[cell] = ext->col_buf[col] + gap * row;
		len = LV_MATH_MIN(len, gap - 2);
		ext->cell_data[cell][len + 1] = '\0'; /* Ensure NULL termination */
		lv_vsnprintf(&ext->cell_data[cell][1], len + 1, fmt, ap2);
	}
	else
	{
		ext->cell_data[cell] = lv_mem_realloc(ext->cell_data[cell], len + 2); /*+1: trailing '\0; +1: format byte*/
		LV_ASSERT_MEM(ext->cell_data[cell]);
		if(ext->cell_data[cell] == NULL) {
			va_end(ap2);
			return;
		}
		ext->cell_data[cell][len + 1] = 0; /* Ensure NULL termination */
		lv_vsnprintf(&ext->cell_data[cell][1], len + 1, fmt, ap2);
	}
#else
	#if LV_USE_ARABIC_PERSIAN_CHARS
		/*Put together the text according to the format string*/
		char * raw_txt = _lv_mem_buf_get(len + 1);
		LV_ASSERT_MEM(raw_txt);
		if(raw_txt == NULL) {
			va_end(ap2);
			return;
		}

		lv_vsnprintf(raw_txt, len + 1, fmt, ap2);

		/*Get the size of the Arabic text and process it*/
		size_t len_ap = _lv_txt_ap_calc_bytes_cnt(raw_txt);
		ext->cell_data[cell] = lv_mem_realloc(ext->cell_data[cell], len_ap + 1);
		LV_ASSERT_MEM(ext->cell_data[cell]);
		if(ext->cell_data[cell] == NULL) {
			va_end(ap2);
			return;
		}
		_lv_txt_ap_proc(raw_txt, &ext->cell_data[cell][1]);

		_lv_mem_buf_release(raw_txt);
	#else
			ext->cell_data[cell] = lv_mem_realloc(ext->cell_data[cell], len + 2); /*+1: trailing '\0; +1: format byte*/
			LV_ASSERT_MEM(ext->cell_data[cell]);
			if(ext->cell_data[cell] == NULL) {
				va_end(ap2);
				return;
			}
			ext->cell_data[cell][len + 1] = 0; /* Ensure NULL termination */
			lv_vsnprintf(&ext->cell_data[cell][1], len + 1, fmt, ap2);
	#endif
#endif
    va_end(ap2);

    ext->cell_data[cell][0] = format.format_byte;
#if USE_QUICK_REFR
	__quick_refr_cell_height(qtable, row, col);
#else
    refr_size(qtable);
#endif
}

/**
 * Set the number of rows
 * @param qtable qtable pointer to a qTable object
 * @param row_cnt number of rows
 */
void lv_qtable_set_row_cnt(lv_obj_t * qtable, uint16_t row_cnt)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    uint16_t old_row_cnt = ext->row_cnt;
    ext->row_cnt         = row_cnt;
#if USE_QUICK_REFR
    for (uint16_t row = row_cnt; row < old_row_cnt; ++row)
        __quick_refr_row_height(qtable, row);
#endif
#if USE_COL_BUFFER
    if (_lv_mem_get_size(ext->cell_data) >= ext->row_cnt * ext->col_cnt * sizeof(char*))
    {
        for (uint16_t row = old_row_cnt; row < row_cnt; ++row)
            __quick_refr_row_height(qtable, row);
        return;
    }
#endif

    if(ext->row_cnt > 0) {
        ext->row_h = lv_mem_realloc(ext->row_h, ext->row_cnt * sizeof(ext->row_h[0]));
        LV_ASSERT_MEM(ext->row_h);
        if(ext->row_h == NULL) return;
#if USE_QUICK_REFR
        for (uint16_t row = old_row_cnt; row < row_cnt; ++row)
            ext->row_h[row] = 0;
#endif
    }
    else {
        lv_mem_free(ext->row_h);
        ext->row_h = NULL;
    }
    if(ext->row_cnt > 0 && ext->col_cnt > 0) {
        /*Free the unused cells*/
        if(old_row_cnt > row_cnt) {
#if USE_COL_BUFFER
			for (uint32_t col = 0, row, cell; col < ext->col_cnt; col++)
				for (row = row_cnt - 1; row > 0; row--)
				{
					cell = row * ext->col_cnt + col;
					if (ext->col_buf[col] && ext->col_buf_size[col])
					{
						if (ext->cell_data[cell] && (ext->cell_data[cell] < ext->col_buf[col] || ext->cell_data[cell] > ext->col_buf[col] + ext->col_buf_size[col]))
							lv_mem_free(ext->cell_data[cell]);
						if (ext->row_cnt)
						{
							uint16_t gap = ext->col_buf_size[col] / ext->row_cnt;
							uint16_t oldgap = ext->col_buf_size[col] / old_row_cnt;
							memcpy(ext->col_buf[col] + row * gap, ext->col_buf[col] + row * oldgap, oldgap);
							ext->cell_data[cell] = ext->col_buf[col] + row * gap;
							ext->cell_data[cell][gap - 1] = '\0';
							continue;
						}
					}
					else if (ext->cell_data[cell])
						lv_mem_free(ext->cell_data[cell]);
				}
#else
            uint16_t old_cell_cnt = old_row_cnt * ext->col_cnt;
            uint32_t new_cell_cnt = ext->col_cnt * ext->row_cnt;
            uint32_t i;
            for(i = new_cell_cnt; i < old_cell_cnt; i++) {
                lv_mem_free(ext->cell_data[i]);
            }
#endif
        }
        ext->cell_data = lv_mem_realloc(ext->cell_data, ext->row_cnt * ext->col_cnt * sizeof(char *));
        LV_ASSERT_MEM(ext->cell_data);
        if(ext->cell_data == NULL) return;

        /*Initialize the new fields*/
        if (old_row_cnt < row_cnt) {
#if USE_COL_BUFFER
            uint16_t old_cell_cnt = old_row_cnt * ext->col_cnt;
            uint32_t new_cell_cnt = ext->col_cnt * ext->row_cnt;
            _lv_memset_00(&ext->cell_data[old_cell_cnt], (new_cell_cnt - old_cell_cnt) * sizeof(ext->cell_data[0]));
            for (uint32_t col = 0, row, cell; col < ext->col_cnt; col++)
                if (ext->col_buf[col] && ext->col_buf_size[col])
                    for (row = 0; row < old_row_cnt; row++)
                    {
                        cell = row * ext->col_cnt + col;
                        if (ext->cell_data[cell] && (ext->cell_data[cell] < ext->col_buf[col] || ext->cell_data[cell] > ext->col_buf[col] + ext->col_buf_size[col]))
                            lv_mem_free(ext->cell_data[cell]);
                        uint16_t gap = ext->col_buf_size[col] / ext->row_cnt;
                        uint16_t oldgap = ext->col_buf_size[col] / old_row_cnt;
                        memcpy(ext->col_buf[col] + row * gap, ext->col_buf[col] + row * oldgap, gap);
                        ext->cell_data[cell] = ext->col_buf[col] + row * gap;
                        ext->cell_data[cell][gap - 1] = '\0';
                    }
#else
            uint16_t old_cell_cnt = old_row_cnt * ext->col_cnt;
            uint32_t new_cell_cnt = ext->col_cnt * ext->row_cnt;
            _lv_memset_00(&ext->cell_data[old_cell_cnt], (new_cell_cnt - old_cell_cnt) * sizeof(ext->cell_data[0]));
#endif
        }
    }
    else {
        lv_mem_free(ext->cell_data);
        ext->cell_data = NULL;
    }

#if USE_QUICK_REFR
    for (uint16_t row = old_row_cnt; row < row_cnt; ++row)
        __quick_refr_row_height(qtable, row);
#else
    refr_size(qtable);
#endif
}

/**
 * Set the number of columns
 * @param qtable qtable pointer to a qTable object
 * @param col_cnt number of columns. Must be < LV_QTABLE_COL_MAX
 */
void lv_qtable_set_col_cnt(lv_obj_t * qtable, uint16_t col_cnt)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    if(col_cnt >= LV_QTABLE_COL_MAX) {
        LV_LOG_WARN("lv_qtable_set_col_cnt: too many columns. Must be < LV_QTABLE_COL_MAX.");
        return;
    }

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    uint16_t old_col_cnt = ext->col_cnt;
    ext->col_cnt         = col_cnt;

#if USE_COL_BUFFER
    if (old_col_cnt == ext->col_cnt) return;
#endif

    if(ext->row_cnt > 0 && ext->col_cnt > 0) {
        /*Free the unused cells*/
        if(old_col_cnt > col_cnt) {
           uint16_t old_cell_cnt = old_col_cnt * ext->row_cnt;
           uint32_t new_cell_cnt = ext->col_cnt * ext->row_cnt;
           uint32_t i;
           for(i = new_cell_cnt; i < old_cell_cnt; i++) {
#if USE_COL_BUFFER
               if (!ext->col_buf[i] || !ext->col_buf_size[i])
                   lv_mem_free(ext->cell_data[i]);
#else
               lv_mem_free(ext->cell_data[i]);
#endif
           }
       }

        ext->cell_data = lv_mem_realloc(ext->cell_data, ext->row_cnt * ext->col_cnt * sizeof(char *));
        LV_ASSERT_MEM(ext->cell_data);
        if(ext->cell_data == NULL) return;
#if USE_COL_BUFFER
		for (uint32_t col = 0, row, cell; col < ext->col_cnt; col++)
            if (ext->col_buf[col] && ext->col_buf_size[col])
            {
                uint16_t gap = ext->col_buf_size[col] / ext->row_cnt;
			    for (row = 0; row < ext->row_cnt; row++)
			    {
				    cell = row * ext->col_cnt + col;
					ext->cell_data[cell] = ext->col_buf[col] + row * gap;
				}
			}
#endif

        /*Initialize the new fields*/
        if (old_col_cnt < col_cnt) {
            uint16_t old_cell_cnt = old_col_cnt * ext->row_cnt;
            uint32_t new_cell_cnt = ext->col_cnt * ext->row_cnt;
            _lv_memset_00(&ext->cell_data[old_cell_cnt], (new_cell_cnt - old_cell_cnt) * sizeof(ext->cell_data[0]));
        }
    }
    else {
        lv_mem_free(ext->cell_data);
        ext->cell_data = NULL;
    }
#if USE_QUICK_REFR
	__quick_refr_width(qtable);
#else
    refr_size(qtable);
#endif
}

/**
 * Set the width of a column
 * @param qtable qtable pointer to a qTable object
 * @param col_id id of the column [0 .. LV_QTABLE_COL_MAX -1]
 * @param w width of the column
 */
void lv_qtable_set_col_width(lv_obj_t * qtable, uint16_t col_id, lv_coord_t w)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    if(col_id >= LV_QTABLE_COL_MAX) {
        LV_LOG_WARN("lv_qtable_set_col_width: too big 'col_id'. Must be < LV_QTABLE_COL_MAX.");
        return;
    }

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    ext->col_w[col_id]   = w;
#if USE_QUICK_REFR
	__quick_refr_width(qtable);
#else
    refr_size(qtable);
#endif
}

/**
 * Set the text align in a cell
 * @param qtable pointer to a qTable object
 * @param row id of the row [0 .. row_cnt -1]
 * @param col id of the column [0 .. col_cnt -1]
 * @param align LV_LABEL_ALIGN_LEFT or LV_LABEL_ALIGN_CENTER or LV_LABEL_ALIGN_RIGHT
 */
void lv_qtable_set_cell_align(lv_obj_t * qtable, uint16_t row, uint16_t col, lv_label_align_t align)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    if(col >= ext->col_cnt) {
        LV_LOG_WARN("lv_qtable_set_cell_align: invalid column");
        return;
    }

    /*Auto expand*/
    if(row >= ext->row_cnt) {
        lv_qtable_set_row_cnt(qtable, row + 1);
    }
    uint32_t cell = row * ext->col_cnt + col;

    if(ext->cell_data[cell] == NULL) {
#if USE_COL_BUFFER
		if (ext->col_buf[col] && ext->col_buf_size[col])
		{
			uint16_t gap = ext->col_buf_size[col] / ext->row_cnt;
			if (ext->cell_data[cell] && (ext->cell_data[cell] < ext->col_buf[col] || ext->cell_data[cell] > ext->col_buf[col] + ext->col_buf_size[col]))
				lv_mem_free(ext->cell_data[cell]);
			
			ext->cell_data[cell] = ext->col_buf[col] + gap * row;
			
			ext->cell_data[cell][0] = 0;
			ext->cell_data[cell][1] = '\0';
		}
		else
		{
			ext->cell_data[cell]    = lv_mem_alloc(2); /*+1: trailing '\0; +1: format byte*/
			LV_ASSERT_MEM(ext->cell_data[cell]);
			if(ext->cell_data[cell] == NULL) return;

			ext->cell_data[cell][0] = 0;
			ext->cell_data[cell][1] = '\0';
		}
#else
        ext->cell_data[cell]    = lv_mem_alloc(2); /*+1: trailing '\0; +1: format byte*/
        LV_ASSERT_MEM(ext->cell_data[cell]);
        if(ext->cell_data[cell] == NULL) return;

        ext->cell_data[cell][0] = 0;
        ext->cell_data[cell][1] = '\0';
#endif
    }

    lv_qtable_cell_format_t format;
    format.format_byte      = ext->cell_data[cell][0];
    format.s.align          = align;
    ext->cell_data[cell][0] = format.format_byte;
}

/**
 * Set the type of a cell.
 * @param qtable pointer to a qTable object
 * @param row id of the row [0 .. row_cnt -1]
 * @param col id of the column [0 .. col_cnt -1]
 * @param type 1,2,3 or 4. The cell style will be chosen accordingly.
 */
void lv_qtable_set_cell_type(lv_obj_t * qtable, uint16_t row, uint16_t col, uint8_t type)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    if(col >= ext->col_cnt) {
        LV_LOG_WARN("lv_qtable_set_cell_type: invalid column");
        return;
    }

    /*Auto expand*/
    if(row >= ext->row_cnt) {
        lv_qtable_set_row_cnt(qtable, row + 1);
    }

    uint32_t cell = row * ext->col_cnt + col;

	if(ext->cell_data[cell] == NULL) {
#if USE_COL_BUFFER
		if (ext->col_buf[col] && ext->col_buf_size[col])
		{
			uint16_t gap = ext->col_buf_size[col] / ext->row_cnt;
			if (ext->cell_data[cell] && (ext->cell_data[cell] < ext->col_buf[col] || ext->cell_data[cell] > ext->col_buf[col] + ext->col_buf_size[col]))
				lv_mem_free(ext->cell_data[cell]);
			
			ext->cell_data[cell] = ext->col_buf[col] + gap * row;
			
			ext->cell_data[cell][0] = 0;
			ext->cell_data[cell][1] = '\0';
		}
		else
		{
			ext->cell_data[cell]    = lv_mem_alloc(2); /*+1: trailing '\0; +1: format byte*/
			LV_ASSERT_MEM(ext->cell_data[cell]);
			if(ext->cell_data[cell] == NULL) return;

			ext->cell_data[cell][0] = 0;
			ext->cell_data[cell][1] = '\0';
		}
#else
        ext->cell_data[cell]    = lv_mem_alloc(2); /*+1: trailing '\0; +1: format byte*/
        LV_ASSERT_MEM(ext->cell_data[cell]);
        if(ext->cell_data[cell] == NULL) return;

        ext->cell_data[cell][0] = 0;
        ext->cell_data[cell][1] = '\0';
#endif
    }

    if(type > 0) type--; /*User gives 1,2,3,4 but easier to handle 0, 1, 2, 3*/
    if(type >= LV_QTABLE_CELL_STYLE_CNT) type = LV_QTABLE_CELL_STYLE_CNT - 1;

    lv_qtable_cell_format_t format;
    format.format_byte      = ext->cell_data[cell][0];
    format.s.type           = type;
	format.s.crop        	= CROP_MODE;
    ext->cell_data[cell][0] = format.format_byte;

    ext->cell_types |= 1 << type;

    lv_obj_invalidate(qtable);
}

/**
 * Set the cell crop. (Don't adjust the height of the cell according to its content)
 * @param qtable pointer to a qTable object
 * @param row id of the row [0 .. row_cnt -1]
 * @param col id of the column [0 .. col_cnt -1]
 * @param crop true: crop the cell content; false: set the cell height to the content.
 */
void lv_qtable_set_cell_crop(lv_obj_t * qtable, uint16_t row, uint16_t col, bool crop)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    if(col >= ext->col_cnt) {
        LV_LOG_WARN("lv_qtable_set_cell_crop: invalid column");
        return;
    }

    /*Auto expand*/
    if(row >= ext->row_cnt) {
        lv_qtable_set_row_cnt(qtable, row + 1);
    }

    uint32_t cell = row * ext->col_cnt + col;

    if(ext->cell_data[cell] == NULL) {

#if USE_COL_BUFFER
		if (ext->col_buf[col] && ext->col_buf_size[col])
		{
			uint16_t gap = ext->col_buf_size[col] / ext->row_cnt;
			if (ext->cell_data[cell] && (ext->cell_data[cell] < ext->col_buf[col] || ext->cell_data[cell] > ext->col_buf[col] + ext->col_buf_size[col]))
				lv_mem_free(ext->cell_data[cell]);
			
			ext->cell_data[cell] = ext->col_buf[col] + gap * row;
			
			ext->cell_data[cell][0] = 0;
			ext->cell_data[cell][1] = '\0';
		}
		else
		{
			ext->cell_data[cell]    = lv_mem_alloc(2); /*+1: trailing '\0; +1: format byte*/
			LV_ASSERT_MEM(ext->cell_data[cell]);
			if(ext->cell_data[cell] == NULL) return;

			ext->cell_data[cell][0] = 0;
			ext->cell_data[cell][1] = '\0';
		}
#else
        ext->cell_data[cell]    = lv_mem_alloc(2); /*+1: trailing '\0; +1: format byte*/
        LV_ASSERT_MEM(ext->cell_data[cell]);
        if(ext->cell_data[cell] == NULL) return;

        ext->cell_data[cell][0] = 0;
        ext->cell_data[cell][1] = '\0';
#endif
    }

    lv_qtable_cell_format_t format;
    format.format_byte      = ext->cell_data[cell][0];
    format.s.crop           = crop;
    ext->cell_data[cell][0] = format.format_byte;
}

/**
 * Merge a cell with the right neighbor. The value of the cell to the right won't be displayed.
 * @param qtable qtable pointer to a qTable object
 * @param row id of the row [0 .. row_cnt -1]
 * @param col id of the column [0 .. col_cnt -1]
 * @param en true: merge right; false: don't merge right
 */
void lv_qtable_set_cell_merge_right(lv_obj_t * qtable, uint16_t row, uint16_t col, bool en)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    if(col >= ext->col_cnt) {
        LV_LOG_WARN("lv_qtable_set_cell_merge_right: invalid column");
        return;
    }

    /*Auto expand*/
    if(row >= ext->row_cnt) {
        lv_qtable_set_row_cnt(qtable, row + 1);
    }

    uint32_t cell = row * ext->col_cnt + col;

    if(ext->cell_data[cell] == NULL) {
#if USE_COL_BUFFER
		if (ext->col_buf[col] && ext->col_buf_size[col])
		{
			uint16_t gap = ext->col_buf_size[col] / ext->row_cnt;
			if (ext->cell_data[cell] && (ext->cell_data[cell] < ext->col_buf[col] || ext->cell_data[cell] > ext->col_buf[col] + ext->col_buf_size[col]))
				lv_mem_free(ext->cell_data[cell]);
			
			ext->cell_data[cell] = ext->col_buf[col] + gap * row;
			
			ext->cell_data[cell][0] = 0;
			ext->cell_data[cell][1] = '\0';
		}
		else
		{
			ext->cell_data[cell]    = lv_mem_alloc(2); /*+1: trailing '\0; +1: format byte*/
			LV_ASSERT_MEM(ext->cell_data[cell]);
			if(ext->cell_data[cell] == NULL) return;

			ext->cell_data[cell][0] = 0;
			ext->cell_data[cell][1] = '\0';
		}
#else
        ext->cell_data[cell]    = lv_mem_alloc(2); /*+1: trailing '\0; +1: format byte*/
        LV_ASSERT_MEM(ext->cell_data[cell]);
        if(ext->cell_data[cell] == NULL) return;

        ext->cell_data[cell][0] = 0;
        ext->cell_data[cell][1] = '\0';
#endif
    }

    lv_qtable_cell_format_t format;
    format.format_byte      = ext->cell_data[cell][0];
    format.s.right_merge    = en ? 1 : 0;
    ext->cell_data[cell][0] = format.format_byte;
#if USE_QUICK_REFR
	__quick_refr_row_height(qtable, row);
#else
    refr_size(qtable);
#endif
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the value of a cell.
 * @param qtable pointer to a qTable object
 * @param row id of the row [0 .. row_cnt -1]
 * @param col id of the column [0 .. col_cnt -1]
 * @return text in the cell
 */
const char * lv_qtable_get_cell_value(lv_obj_t * qtable, uint16_t row, uint16_t col)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    if(row >= ext->row_cnt || col >= ext->col_cnt) {
        LV_LOG_WARN("lv_qtable_set_cell_value: invalid row or column");
        return "";
    }
    uint32_t cell = row * ext->col_cnt + col;

    if(ext->cell_data[cell] == NULL) return "";

    return &ext->cell_data[cell][1]; /*Skip the format byte*/
}

/**
 * Get the number of rows.
 * @param qtable qtable pointer to a qTable object
 * @return number of rows.
 */
uint16_t lv_qtable_get_row_cnt(lv_obj_t * qtable)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    return ext->row_cnt;
}

/**
 * Get the number of columns.
 * @param qtable qtable pointer to a qTable object
 * @return number of columns.
 */
uint16_t lv_qtable_get_col_cnt(lv_obj_t * qtable)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    return ext->col_cnt;
}

/**
 * Get the width of a column
 * @param qtable qtable pointer to a qTable object
 * @param col_id id of the column [0 .. LV_QTABLE_COL_MAX -1]
 * @return width of the column
 */
lv_coord_t lv_qtable_get_col_width(lv_obj_t * qtable, uint16_t col_id)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    if(col_id >= LV_QTABLE_COL_MAX) {
        LV_LOG_WARN("lv_qtable_set_col_width: too big 'col_id'. Must be < LV_QTABLE_COL_MAX.");
        return 0;
    }

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    return ext->col_w[col_id];
}

/**
 * Get the text align of a cell
 * @param qtable pointer to a qTable object
 * @param row id of the row [0 .. row_cnt -1]
 * @param col id of the column [0 .. col_cnt -1]
 * @return LV_LABEL_ALIGN_LEFT (default in case of error) or LV_LABEL_ALIGN_CENTER or
 * LV_LABEL_ALIGN_RIGHT
 */
lv_label_align_t lv_qtable_get_cell_align(lv_obj_t * qtable, uint16_t row, uint16_t col)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    if(row >= ext->row_cnt || col >= ext->col_cnt) {
        LV_LOG_WARN("lv_qtable_set_cell_align: invalid row or column");
        return LV_LABEL_ALIGN_LEFT; /*Just return with something*/
    }
    uint32_t cell = row * ext->col_cnt + col;

    if(ext->cell_data[cell] == NULL)
        return LV_LABEL_ALIGN_LEFT; /*Just return with something*/
    else {
        lv_qtable_cell_format_t format;
        format.format_byte = ext->cell_data[cell][0];
        return format.s.align;
    }
}

/**
 * Get the type of a cell
 * @param qtable pointer to a qTable object
 * @param row id of the row [0 .. row_cnt -1]
 * @param col id of the column [0 .. col_cnt -1]
 * @return 1,2,3 or 4
 */
lv_label_align_t lv_qtable_get_cell_type(lv_obj_t * qtable, uint16_t row, uint16_t col)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    if(row >= ext->row_cnt || col >= ext->col_cnt) {
        LV_LOG_WARN("lv_qtable_get_cell_type: invalid row or column");
        return 1; /*Just return with something*/
    }
    uint32_t cell = row * ext->col_cnt + col;

    if(ext->cell_data[cell] == NULL)
        return 1; /*Just return with something*/
    else {
        lv_qtable_cell_format_t format;
        format.format_byte = ext->cell_data[cell][0];
        return format.s.type + 1; /*0,1,2,3 is stored but user sees 1,2,3,4*/
    }
}

/**
 * Get the crop property of a cell
 * @param qtable pointer to a qTable object
 * @param row id of the row [0 .. row_cnt -1]
 * @param col id of the column [0 .. col_cnt -1]
 * @return true: text crop enabled; false: disabled
 */
lv_label_align_t lv_qtable_get_cell_crop(lv_obj_t * qtable, uint16_t row, uint16_t col)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    if(row >= ext->row_cnt || col >= ext->col_cnt) {
        LV_LOG_WARN("lv_qtable_get_cell_crop: invalid row or column");
        return false; /*Just return with something*/
    }
    uint32_t cell = row * ext->col_cnt + col;

    if(ext->cell_data[cell] == NULL)
        return false; /*Just return with something*/
    else {
        lv_qtable_cell_format_t format;
        format.format_byte = ext->cell_data[cell][0];
        return format.s.crop;
    }
}

/**
 * Get the cell merge attribute.
 * @param qtable qtable pointer to a qTable object
 * @param row id of the row [0 .. row_cnt -1]
 * @param col id of the column [0 .. col_cnt -1]
 * @return true: merge right; false: don't merge right
 */
bool lv_qtable_get_cell_merge_right(lv_obj_t * qtable, uint16_t row, uint16_t col)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    if(row >= ext->row_cnt || col >= ext->col_cnt) {
        LV_LOG_WARN("lv_qtable_get_cell_merge_right: invalid row or column");
        return false;
    }

    uint32_t cell = row * ext->col_cnt + col;

    if(ext->cell_data[cell] == NULL)
        return false;
    else {
        lv_qtable_cell_format_t format;
        format.format_byte = ext->cell_data[cell][0];
        return format.s.right_merge ? true : false;
    }
}

/**
 * Get the last pressed or being pressed cell
 * @param qtable pointer to a qtable object
 * @param row pointer to variable to store the pressed row
 * @param col pointer to variable to store the pressed column
 * @return LV_RES_OK: a valid pressed cell was found, LV_RES_INV: no valid cell is pressed
 */
lv_res_t lv_qtable_get_pressed_cell(lv_obj_t * qtable, uint16_t * row, uint16_t * col)
{
    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);

    lv_indev_type_t type = lv_indev_get_type(lv_indev_get_act());
    if(type != LV_INDEV_TYPE_POINTER && type != LV_INDEV_TYPE_BUTTON) {
        if(col) *col = 0xFFFF;
        if(row) *row = 0xFFFF;
        return LV_RES_INV;
    }

    lv_point_t p;
    lv_indev_get_point(lv_indev_get_act(), &p);

    lv_coord_t tmp;
    if(col) {
        lv_coord_t x = p.x;
        x -= qtable->coords.x1;
        x -= lv_obj_get_style_pad_left(qtable, LV_QTABLE_PART_BG);
        *col = 0;
        tmp = 0;
        for(*col = 0; *col < ext->col_cnt; (*col)++) {
            tmp += ext->col_w[*col];
            if(x < tmp) break;
        }
    }

    if(row) {
        lv_coord_t y = p.y;
        y -= qtable->coords.y1;
        y -= lv_obj_get_style_pad_top(qtable, LV_QTABLE_PART_BG);

        *row = 0;
        tmp = 0;

        for(*row = 0; *row < ext->row_cnt; (*row)++) {
            tmp += ext->row_h[*row];
            if(y < tmp) break;
        }
    }

    return LV_RES_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the tables
 * @param qtable pointer to an object
 * @param clip_area the object will be drawn only in this area
 * @param mode LV_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             LV_DESIGN_DRAW: draw the object (always return 'true')
 *             LV_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return an element of `lv_design_res_t`
 */
static lv_design_res_t lv_qtable_design(lv_obj_t * qtable, const lv_area_t * clip_area, lv_design_mode_t mode)
{
    /*Return false if the object is not covers the mask_p area*/
    if(mode == LV_DESIGN_COVER_CHK) {
        return ancestor_design(qtable, clip_area, mode);
    }
    /*Draw the object*/
    else if(mode == LV_DESIGN_DRAW_MAIN) {
        /*Draw the background*/
        ancestor_design(qtable, clip_area, mode);

        lv_qtable_ext_t * ext        = lv_obj_get_ext_attr(qtable);

        lv_point_t txt_size;
        lv_area_t cell_area;
        lv_area_t txt_area;
        lv_txt_flag_t txt_flags;

        lv_style_int_t bg_top = lv_obj_get_style_pad_top(qtable, LV_QTABLE_PART_BG);
        lv_style_int_t bg_bottom = lv_obj_get_style_pad_bottom(qtable, LV_QTABLE_PART_BG);
        lv_style_int_t bg_left = lv_obj_get_style_pad_left(qtable, LV_QTABLE_PART_BG);
        lv_style_int_t bg_right = lv_obj_get_style_pad_right(qtable, LV_QTABLE_PART_BG);

        lv_draw_rect_dsc_t rect_dsc[LV_QTABLE_CELL_STYLE_CNT];
        lv_draw_label_dsc_t label_dsc[LV_QTABLE_CELL_STYLE_CNT];
        lv_draw_line_dsc_t line_dsc[LV_QTABLE_CELL_STYLE_CNT];
        lv_style_int_t cell_left[LV_QTABLE_CELL_STYLE_CNT];
        lv_style_int_t cell_right[LV_QTABLE_CELL_STYLE_CNT];
        lv_style_int_t cell_top[LV_QTABLE_CELL_STYLE_CNT];
        lv_style_int_t cell_bottom[LV_QTABLE_CELL_STYLE_CNT];

        uint16_t i;
        for(i = 0; i < LV_QTABLE_CELL_STYLE_CNT; i++) {
            if((ext->cell_types & (1 << i)) == 0) continue; /*Skip unused cell types*/
            lv_draw_rect_dsc_init(&rect_dsc[i]);
            lv_obj_init_draw_rect_dsc(qtable, LV_QTABLE_PART_CELL1 + i, &rect_dsc[i]);

            lv_draw_label_dsc_init(&label_dsc[i]);
            lv_obj_init_draw_label_dsc(qtable, LV_QTABLE_PART_CELL1 + i, &label_dsc[i]);

            lv_draw_line_dsc_init(&line_dsc[i]);
            lv_obj_init_draw_line_dsc(qtable, LV_QTABLE_PART_CELL1 + i, &line_dsc[i]);

            cell_left[i] = lv_obj_get_style_pad_left(qtable, LV_QTABLE_PART_CELL1 + i);
            cell_right[i] = lv_obj_get_style_pad_right(qtable, LV_QTABLE_PART_CELL1 + i);
            cell_top[i] = lv_obj_get_style_pad_top(qtable, LV_QTABLE_PART_CELL1 + i);
            cell_bottom[i] = lv_obj_get_style_pad_bottom(qtable, LV_QTABLE_PART_CELL1 + i);
        }

        uint16_t col;
        uint16_t row;
        uint16_t cell = 0;

        bool rtl = lv_obj_get_base_dir(qtable) == LV_BIDI_DIR_RTL ? true : false;

        cell_area.y2 = qtable->coords.y1 + bg_top - 1;
        for(row = 0; row < ext->row_cnt; row++) {
            lv_coord_t h_row = ext->row_h[row];

            cell_area.y1 = cell_area.y2 + 1;
            cell_area.y2 = cell_area.y1 + h_row - 1;

            if(cell_area.y1 > clip_area->y2) return LV_DESIGN_RES_OK;

            if(rtl) cell_area.x1 = qtable->coords.x2 - bg_right - 1;
            else cell_area.x2 = qtable->coords.x1 + bg_left - 1;

            for(col = 0; col < ext->col_cnt; col++) {

                lv_qtable_cell_format_t format;
                if(ext->cell_data[cell]) {
                    format.format_byte = ext->cell_data[cell][0];
                }
                else {
                    format.s.right_merge = 0;
                    format.s.align       = LV_LABEL_ALIGN_LEFT;
                    format.s.type        = 0;
                    format.s.crop        = 1;
                }

                if(rtl) {
                    cell_area.x2 = cell_area.x1 - 1;
                    cell_area.x1 = cell_area.x2 - ext->col_w[col] + 1;
                }
                else {
                    cell_area.x1 = cell_area.x2 + 1;
                    cell_area.x2 = cell_area.x1 + ext->col_w[col] - 1;
                }

                uint16_t col_merge = 0;
                for(col_merge = 0; col_merge + col < ext->col_cnt - 1; col_merge++) {
                    if(ext->cell_data[cell + col_merge] != NULL) {
                        format.format_byte = ext->cell_data[cell + col_merge][0];
                        if(format.s.right_merge)
                            if(rtl) cell_area.x1 -= ext->col_w[col + col_merge + 1];
                            else cell_area.x2 += ext->col_w[col + col_merge + 1];
                        else
                            break;
                    }
                    else {
                        break;
                    }
                }

                if(cell_area.y2 < clip_area->y1) {
                    cell += col_merge + 1;
                    col += col_merge;
                    continue;
                }

                uint8_t cell_type = format.s.type;

                /*Expand the cell area with a half border to avoid drawing 2 borders next to each other*/
                lv_area_t cell_area_border;
                lv_area_copy(&cell_area_border, &cell_area);
                if((rect_dsc[cell_type].border_side & LV_BORDER_SIDE_LEFT) && cell_area_border.x1 > qtable->coords.x1 + bg_left) {
                    cell_area_border.x1 -= rect_dsc[cell_type].border_width / 2;
                }
                if((rect_dsc[cell_type].border_side & LV_BORDER_SIDE_TOP) && cell_area_border.y1 > qtable->coords.y1 + bg_top) {
                    cell_area_border.y1 -= rect_dsc[cell_type].border_width / 2;
                }
                if((rect_dsc[cell_type].border_side & LV_BORDER_SIDE_RIGHT) && cell_area_border.x2 < qtable->coords.x2 - bg_right - 1) {
                    cell_area_border.x2 += rect_dsc[cell_type].border_width / 2 + (rect_dsc[cell_type].border_width & 0x1);
                }
                if((rect_dsc[cell_type].border_side & LV_BORDER_SIDE_BOTTOM) &&
                   cell_area_border.y2 < qtable->coords.y2 - bg_bottom - 1) {
                    cell_area_border.y2 += rect_dsc[cell_type].border_width / 2 + (rect_dsc[cell_type].border_width & 0x1);
                }

                lv_draw_rect(&cell_area_border, clip_area, &rect_dsc[cell_type]);

                if(ext->cell_data[cell]) {
                    txt_area.x1 = cell_area.x1 + cell_left[cell_type];
                    txt_area.x2 = cell_area.x2 - cell_right[cell_type];
                    txt_area.y1 = cell_area.y1 + cell_top[cell_type];
                    txt_area.y2 = cell_area.y2 - cell_bottom[cell_type];

                    /*Align the content to the middle if not cropped*/
                    if(format.s.crop == 0) {
                        txt_flags = LV_TXT_FLAG_NONE;
                    }
                    else {
                        txt_flags = LV_TXT_FLAG_EXPAND;
#if CROP_MODE
						label_dsc[cell_type].line_space += txt_area.y2 - txt_area.y1;
#endif
                    }

                    _lv_txt_get_size(&txt_size, ext->cell_data[cell] + 1, label_dsc[cell_type].font,
                                     label_dsc[cell_type].letter_space, label_dsc[cell_type].line_space,
                                     lv_area_get_width(&txt_area), txt_flags);

                    label_dsc[cell_type].flag = 0;
                    /*Align the content to the middle if not cropped*/
                    if(format.s.crop == 0) {
                        txt_area.y1 = cell_area.y1 + h_row / 2 - txt_size.y / 2;
                        txt_area.y2 = cell_area.y1 + h_row / 2 + txt_size.y / 2;
                    }
#if CROP_MODE		// Always content to the middle
					txt_area.y1 = cell_area.y1 + h_row / 2 - txt_size.y / 2;
					txt_area.y2 = cell_area.y1 + h_row / 2 + txt_size.y / 2;
#endif

                    switch(format.s.align) {
                        default:
                        case LV_LABEL_ALIGN_LEFT:
                            label_dsc[cell_type].flag |= LV_TXT_FLAG_NONE;
                            break;
                        case LV_LABEL_ALIGN_RIGHT:
                            label_dsc[cell_type].flag |= LV_TXT_FLAG_RIGHT;
                            break;
                        case LV_LABEL_ALIGN_CENTER:
                            label_dsc[cell_type].flag |= LV_TXT_FLAG_CENTER;
                            break;
                    }

                    lv_area_t label_mask;
                    bool label_mask_ok;
                    label_mask_ok = _lv_area_intersect(&label_mask, clip_area, &cell_area);
                    if(label_mask_ok) {
                        lv_draw_label(&txt_area, &label_mask, &label_dsc[cell_type], ext->cell_data[cell] + 1, NULL);
                    }

                    /*Draw lines after '\n's*/
                    lv_point_t p1;
                    lv_point_t p2;
                    p1.x = cell_area.x1;
                    p2.x = cell_area.x2;
                    for(i = 1; ext->cell_data[cell][i] != '\0'; i++) {
                        if(ext->cell_data[cell][i] == '\n') {
                            ext->cell_data[cell][i] = '\0';
                            _lv_txt_get_size(&txt_size, ext->cell_data[cell] + 1, label_dsc[cell_type].font,
                                             label_dsc[cell_type].letter_space, label_dsc[cell_type].line_space,
                                             lv_area_get_width(&txt_area), txt_flags);

                            p1.y = txt_area.y1 + txt_size.y + label_dsc[cell_type].line_space / 2;
                            p2.y = txt_area.y1 + txt_size.y + label_dsc[cell_type].line_space / 2;
                            lv_draw_line(&p1, &p2, clip_area, &line_dsc[cell_type]);

                            ext->cell_data[cell][i] = '\n';
                        }
                    }
                }

                cell += col_merge + 1;
                col += col_merge;
            }
        }
    }
    /*Post draw when the children are drawn*/
    else if(mode == LV_DESIGN_DRAW_POST) {
        ancestor_design(qtable, clip_area, mode);
    }

    return LV_DESIGN_RES_OK;
}

/**
 * Signal function of the qtable
 * @param qtable pointer to a qtable object
 * @param sign a signal type from lv_signal_t enum
 * @param param pointer to a signal specific variable
 * @return LV_RES_OK: the object is not deleted in the function; LV_RES_INV: the object is deleted
 */
static lv_res_t lv_qtable_signal(lv_obj_t * qtable, lv_signal_t sign, void * param)
{
    lv_res_t res;
    if(sign == LV_SIGNAL_GET_STYLE) {
        lv_get_style_info_t * info = param;
        info->result = lv_qtable_get_style(qtable, info->part);
        if(info->result != NULL) return LV_RES_OK;
        else return ancestor_signal(qtable, sign, param);
    }

    /* Include the ancient signal function */
    res = ancestor_signal(qtable, sign, param);
    if(res != LV_RES_OK) return res;
    if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);

    if(sign == LV_SIGNAL_CLEANUP) {
        /*Free the cell texts*/
        lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
        uint16_t i;
		
#if USE_COL_BUFFER
		for (uint32_t col = 0, row, cell; col < ext->col_cnt; col++)
		{
			for (row = 0; row < ext->row_cnt; row++)
			{
				cell = row * ext->col_cnt + col;
				if (ext->col_buf[col] && ext->col_buf_size[col])
					if (ext->cell_data[cell] && (ext->cell_data[cell] < ext->col_buf[col] || ext->cell_data[cell] > ext->col_buf[col] + ext->col_buf_size[col]))
						lv_mem_free(ext->cell_data[cell]);
                ext->cell_data[cell] = NULL;
			}
			if (ext->col_buf[col]) lv_mem_free(ext->col_buf[col]);
		}
#else
        for(i = 0; i < ext->col_cnt * ext->row_cnt; i++) {
            if(ext->cell_data[i]) {
                lv_mem_free(ext->cell_data[i]);
                ext->cell_data[i] = NULL;
            }
        }
#endif
        if(ext->cell_data) lv_mem_free(ext->cell_data);
        if(ext->row_h) lv_mem_free(ext->row_h);

        for(i = 0; i < LV_QTABLE_CELL_STYLE_CNT; i++) {
            lv_obj_clean_style_list(qtable, LV_QTABLE_PART_CELL1 + i);
        }
    }
    else if(sign == LV_SIGNAL_STYLE_CHG) {
#if USE_QUICK_REFR
        __quick_refr_height(qtable);
        __quick_refr_width(qtable);
#else
        refr_size(qtable);
#endif
    }

    return res;
}

/**
 * Get the style descriptor of a part of the object
 * @param qtable pointer the object
 * @param part the part from. (LV_QTABLE_PART_...)
 * @return pointer to the style descriptor of the specified part
 */
static lv_style_list_t * lv_qtable_get_style(lv_obj_t * qtable, uint8_t part)
{
    LV_ASSERT_OBJ(qtable, LV_OBJX_NAME);

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);

    /* Because of the presence of LV_QTABLE_PART_BG, LV_QTABLE_PART_CELL<i> has an integer value
       of <i>. This comes in useful to extend above code with more cell types as follows */
    if(part == LV_QTABLE_PART_BG) {
        return &qtable->style_list;
    }
    else if(part >= 1 && part <= LV_QTABLE_CELL_STYLE_CNT) {
        return &ext->cell_style[part - 1];
    }

    return NULL;
}

static void refr_size(lv_obj_t * qtable)
{
    lv_coord_t h = 0;
    lv_coord_t w = 0;

    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    if(ext->row_cnt == 0 || ext->col_cnt == 0) {
        lv_obj_set_size(qtable, w, h);
        return;
    }

    uint16_t i;
    for(i = 0; i < ext->col_cnt; i++) {
        w += ext->col_w[i];
    }

    lv_style_int_t cell_left[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t cell_right[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t cell_top[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t cell_bottom[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t letter_space[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t line_space[LV_QTABLE_CELL_STYLE_CNT];
    const lv_font_t * font[LV_QTABLE_CELL_STYLE_CNT];

    for(i = 0; i < LV_QTABLE_CELL_STYLE_CNT; i++) {
        if((ext->cell_types & (1 << i)) == 0) continue; /*Skip unused cell types*/
        cell_left[i] = lv_obj_get_style_pad_left(qtable, LV_QTABLE_PART_CELL1 + i);
        cell_right[i] = lv_obj_get_style_pad_right(qtable, LV_QTABLE_PART_CELL1 + i);
        cell_top[i] = lv_obj_get_style_pad_top(qtable, LV_QTABLE_PART_CELL1 + i);
        cell_bottom[i] = lv_obj_get_style_pad_bottom(qtable, LV_QTABLE_PART_CELL1 + i);
        letter_space[i] = lv_obj_get_style_text_letter_space(qtable, LV_QTABLE_PART_CELL1 + i);
        line_space[i] = lv_obj_get_style_text_line_space(qtable, LV_QTABLE_PART_CELL1 + i);
        font[i] = lv_obj_get_style_text_font(qtable, LV_QTABLE_PART_CELL1 + i);
    }

    for(i = 0; i < ext->row_cnt; i++) {
        ext->row_h[i] = get_row_height(qtable, i, font, letter_space, line_space,
                                       cell_left, cell_right, cell_top, cell_bottom);
        h += ext->row_h[i];
    }

    lv_style_int_t bg_top = lv_obj_get_style_pad_top(qtable, LV_QTABLE_PART_BG);
    lv_style_int_t bg_bottom = lv_obj_get_style_pad_bottom(qtable, LV_QTABLE_PART_BG);
    lv_style_int_t bg_left = lv_obj_get_style_pad_left(qtable, LV_QTABLE_PART_BG);
    lv_style_int_t bg_right = lv_obj_get_style_pad_right(qtable, LV_QTABLE_PART_BG);
    w += bg_left + bg_right;
    h += bg_top + bg_bottom;

    lv_obj_set_size(qtable, w + 1, h + 1);
    lv_obj_invalidate(qtable); /*Always invalidate even if the size hasn't changed*/
}

static lv_coord_t get_row_height(lv_obj_t * qtable, uint16_t row_id, const lv_font_t ** font,
                                 lv_style_int_t * letter_space, lv_style_int_t * line_space,
                                 lv_style_int_t * cell_left, lv_style_int_t * cell_right, lv_style_int_t * cell_top, lv_style_int_t * cell_bottom)
{
    lv_qtable_ext_t * ext = lv_obj_get_ext_attr(qtable);
    lv_point_t txt_size;
    lv_coord_t txt_w;

    uint16_t row_start = row_id * ext->col_cnt;
    uint16_t cell;
    uint16_t col;
    lv_coord_t h_max = lv_font_get_line_height(font[0]) + cell_top[0] + cell_bottom[0];

    for(cell = row_start, col = 0; cell < row_start + ext->col_cnt; cell++, col++) {
        if(ext->cell_data[cell] != NULL) {
            txt_w              = ext->col_w[col];
            uint16_t col_merge = 0;
            for(col_merge = 0; col_merge + col < ext->col_cnt - 1; col_merge++) {

                if(ext->cell_data[cell + col_merge] != NULL) {
                    lv_qtable_cell_format_t format;
                    format.format_byte = ext->cell_data[cell + col_merge][0];
                    if(format.s.right_merge)
                        txt_w += ext->col_w[col + col_merge + 1];
                    else
                        break;
                }
                else {
                    break;
                }
            }

            lv_qtable_cell_format_t format;
            format.format_byte = ext->cell_data[cell][0];
            uint8_t cell_type  = format.s.type;

            /*With text crop assume 1 line*/
            if(format.s.crop) {
                h_max = LV_MATH_MAX(lv_font_get_line_height(font[cell_type]) + cell_top[cell_type] + cell_bottom[cell_type],
                                    h_max);
            }
            /*Without text crop calculate the height of the text in the cell*/
            else {
                txt_w -= cell_left[cell_type] + cell_right[cell_type];

                _lv_txt_get_size(&txt_size, ext->cell_data[cell] + 1, font[cell_type],
                                 letter_space[cell_type], line_space[cell_type], txt_w, LV_TXT_FLAG_NONE);

                h_max = LV_MATH_MAX(txt_size.y + cell_top[cell_type] + cell_bottom[cell_type], h_max);
                cell += col_merge;
                col += col_merge;
            }
        }
    }

    return h_max;
}


static void __quick_refr_height(lv_obj_t * qtable)
{
    lv_qtable_ext_t * ext = (lv_qtable_ext_t*)lv_obj_get_ext_attr(qtable);

    lv_coord_t h_max = 0;
    lv_coord_t tableHeight = lv_obj_get_height(qtable);
    lv_style_int_t cell_left[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t cell_right[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t cell_top[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t cell_bottom[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t letter_space[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t line_space[LV_QTABLE_CELL_STYLE_CNT];
    const lv_font_t* font[LV_QTABLE_CELL_STYLE_CNT];

    for (int i = 0; i < LV_QTABLE_CELL_STYLE_CNT; i++) {
        if ((ext->cell_types & (1 << i)) == 0) continue; /*Skip unused cell types*/
        cell_left[i] = lv_obj_get_style_pad_left(qtable, LV_QTABLE_PART_CELL1 + i);
        cell_right[i] = lv_obj_get_style_pad_right(qtable, LV_QTABLE_PART_CELL1 + i);
        cell_top[i] = lv_obj_get_style_pad_top(qtable, LV_QTABLE_PART_CELL1 + i);
        cell_bottom[i] = lv_obj_get_style_pad_bottom(qtable, LV_QTABLE_PART_CELL1 + i);
        letter_space[i] = lv_obj_get_style_text_letter_space(qtable, LV_QTABLE_PART_CELL1 + i);
        line_space[i] = lv_obj_get_style_text_line_space(qtable, LV_QTABLE_PART_CELL1 + i);
        font[i] = lv_obj_get_style_text_font(qtable, LV_QTABLE_PART_CELL1 + i);
    }
    for (uint16_t row = 0; row < ext->row_cnt; row++)
    {
        if (ext->col_cnt)
        {
            h_max = 0;
            for (uint16_t col = 0; col < ext->col_cnt; col++)
                h_max = LV_MATH_MAX(__qucik_get_row_height(qtable, row, col, font, letter_space, line_space,
                    cell_left, cell_right, cell_top, cell_bottom), h_max);
        }
        else
            h_max = __qucik_get_row_height(qtable, row, 0, font, letter_space, line_space,
                cell_left, cell_right, cell_top, cell_bottom);

        tableHeight += h_max - ext->row_h[row];
        ext->row_h[row] = h_max;
    }
    lv_obj_set_height(qtable, tableHeight);
	
    lv_obj_invalidate(qtable); /*Always invalidate even if the size hasn't changed*/
}

static void __quick_refr_width(lv_obj_t * qtable)
{
    lv_qtable_ext_t * ext = (lv_qtable_ext_t*)lv_obj_get_ext_attr(qtable);
    lv_style_int_t bg_left = lv_obj_get_style_pad_left(qtable, LV_QTABLE_PART_BG);
    lv_style_int_t bg_right = lv_obj_get_style_pad_right(qtable, LV_QTABLE_PART_BG);
    lv_coord_t w = bg_left + bg_right;
	
    for (uint16_t i = 0; i < ext->col_cnt; i++)
        w += ext->col_w[i];
    lv_obj_set_width(qtable, w + 1);
	
    lv_obj_invalidate(qtable); /*Always invalidate even if the size hasn't changed*/
}

static void __quick_refr_row_height(lv_obj_t * qtable, uint16_t row)
{
    lv_qtable_ext_t * ext = (lv_qtable_ext_t*)lv_obj_get_ext_attr(qtable);
    lv_style_int_t cell_left[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t cell_right[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t cell_top[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t cell_bottom[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t letter_space[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t line_space[LV_QTABLE_CELL_STYLE_CNT];
    const lv_font_t* font[LV_QTABLE_CELL_STYLE_CNT];
    for (int i = 0; i < LV_QTABLE_CELL_STYLE_CNT; i++) {
        if ((ext->cell_types & (1 << i)) == 0) continue; /*Skip unused cell types*/
        cell_left[i] = lv_obj_get_style_pad_left(qtable, LV_QTABLE_PART_CELL1 + i);
        cell_right[i] = lv_obj_get_style_pad_right(qtable, LV_QTABLE_PART_CELL1 + i);
        cell_top[i] = lv_obj_get_style_pad_top(qtable, LV_QTABLE_PART_CELL1 + i);
        cell_bottom[i] = lv_obj_get_style_pad_bottom(qtable, LV_QTABLE_PART_CELL1 + i);
        letter_space[i] = lv_obj_get_style_text_letter_space(qtable, LV_QTABLE_PART_CELL1 + i);
        line_space[i] = lv_obj_get_style_text_line_space(qtable, LV_QTABLE_PART_CELL1 + i);
        font[i] = lv_obj_get_style_text_font(qtable, LV_QTABLE_PART_CELL1 + i);
    }
    lv_coord_t h_max = 0;
	if (ext->col_cnt)
	{
        h_max = 0;
		for (uint16_t i = 0; i < ext->col_cnt; i++)
			h_max = LV_MATH_MAX(__qucik_get_row_height(qtable, row, i, font, letter_space, line_space,
                cell_left, cell_right, cell_top, cell_bottom), h_max);
	}
    else
        h_max = __qucik_get_row_height(qtable, row, 0, font, letter_space, line_space,
            cell_left, cell_right, cell_top, cell_bottom);

    lv_obj_set_height(qtable, lv_obj_get_height(qtable) + h_max - ext->row_h[row]);
    ext->row_h[row] = h_max;
	
    lv_obj_invalidate(qtable); /*Always invalidate even if the size hasn't changed*/
}

static void __quick_refr_cell_height(lv_obj_t * qtable, uint16_t row, uint16_t col)
{
    lv_qtable_ext_t * ext = (lv_qtable_ext_t*)lv_obj_get_ext_attr(qtable);
    lv_style_int_t cell_left[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t cell_right[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t cell_top[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t cell_bottom[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t letter_space[LV_QTABLE_CELL_STYLE_CNT];
    lv_style_int_t line_space[LV_QTABLE_CELL_STYLE_CNT];
    const lv_font_t* font[LV_QTABLE_CELL_STYLE_CNT];

    for (int i = 0; i < LV_QTABLE_CELL_STYLE_CNT; i++) {
        if ((ext->cell_types & (1 << i)) == 0) continue; /*Skip unused cell types*/
        cell_left[i] = lv_obj_get_style_pad_left(qtable, LV_QTABLE_PART_CELL1 + i);
        cell_right[i] = lv_obj_get_style_pad_right(qtable, LV_QTABLE_PART_CELL1 + i);
        cell_top[i] = lv_obj_get_style_pad_top(qtable, LV_QTABLE_PART_CELL1 + i);
        cell_bottom[i] = lv_obj_get_style_pad_bottom(qtable, LV_QTABLE_PART_CELL1 + i);
        letter_space[i] = lv_obj_get_style_text_letter_space(qtable, LV_QTABLE_PART_CELL1 + i);
        line_space[i] = lv_obj_get_style_text_line_space(qtable, LV_QTABLE_PART_CELL1 + i);
        font[i] = lv_obj_get_style_text_font(qtable, LV_QTABLE_PART_CELL1 + i);
    }
	lv_coord_t row_h = __qucik_get_row_height(qtable, row, col, font, letter_space, line_space,
        cell_left, cell_right, cell_top, cell_bottom);
    lv_obj_set_height(qtable, lv_obj_get_height(qtable) + row_h - ext->row_h[row]);
	ext->row_h[row] = row_h;
    lv_obj_invalidate(qtable); /*Always invalidate even if the size hasn't changed*/
}

static lv_coord_t __qucik_get_row_height(lv_obj_t * qtable, uint16_t row, uint16_t col, const lv_font_t** font,
    lv_style_int_t* letter_space, lv_style_int_t* line_space,
    lv_style_int_t* cell_left, lv_style_int_t* cell_right, lv_style_int_t* cell_top, lv_style_int_t* cell_bottom)
{
    lv_qtable_ext_t * ext = (lv_qtable_ext_t*)lv_obj_get_ext_attr(qtable);
	if (row >= ext->row_cnt) return 0;
	
    lv_coord_t txt_w, txt_h;
    uint32_t cell = row * ext->col_cnt + col;
    lv_coord_t h_max = lv_font_get_line_height(font[0]) + cell_top[0] + cell_bottom[0];

	if (ext->cell_data && ext->cell_data[cell])
	{
		txt_w = ext->col_w[col];
		uint16_t col_merge = 0;
		for(col_merge = 0; col_merge + col < ext->col_cnt - 1; col_merge++) {

			if(ext->cell_data[cell + col_merge] != NULL) {
				lv_qtable_cell_format_t format;
				format.format_byte = ext->cell_data[cell + col_merge][0];
				if(format.s.right_merge)
					txt_w += ext->col_w[col + col_merge + 1];
				else
					break;
			}
			else {
				break;
			}
		}

		lv_qtable_cell_format_t format;
		format.format_byte = ext->cell_data[cell][0];
		uint8_t cell_type  = format.s.type;
		
		/*With text crop assume 1 line*/
		if(format.s.crop) {
			h_max = LV_MATH_MAX(lv_font_get_line_height(font[cell_type]) + cell_top[cell_type] + cell_bottom[cell_type], h_max);
		}
		/*Without text crop calculate the height of the text in the cell*/
		else {
			txt_w -= cell_left[cell_type] + cell_right[cell_type];

			txt_h = __quick_lv_txt_get_height(ext->cell_data[cell] + 1, font[cell_type], letter_space[cell_type], line_space[cell_type], txt_w, LV_TXT_FLAG_NONE);

			h_max = LV_MATH_MAX(txt_h + cell_top[cell_type] + cell_bottom[cell_type], h_max);
		}
	}
	
    return h_max;
}

#define NO_BREAK_FOUND UINT32_MAX
static inline bool is_break_char(uint32_t letter)
{
    uint8_t i;
    bool ret = false;

    /*Compare the letter to TXT_BREAK_CHARS*/
    for(i = 0; LV_TXT_BREAK_CHARS[i] != '\0'; i++) {
        if(letter == (uint32_t)LV_TXT_BREAK_CHARS[i]) {
            ret = true; /*If match then it is break char*/
            break;
        }
    }

    return ret;
}

static uint32_t __quick_lv_txt_get_next_word(const char * txt, const lv_font_t * font,
                                     lv_coord_t letter_space, lv_coord_t max_width,
                                     lv_txt_flag_t flag, uint32_t * word_w_ptr, lv_txt_cmd_state_t * cmd_state, bool force)
{
    if(txt == NULL || txt[0] == '\0') return 0;
    if(font == NULL) return 0;

    if(flag & LV_TXT_FLAG_EXPAND) max_width = LV_COORD_MAX;

    uint32_t i = 0, i_next = 0, i_next_next = 0;  /* Iterating index into txt */
    uint32_t letter = 0;      /* Letter at i */
    uint32_t letter_next = 0; /* Letter at i_next */
    lv_coord_t letter_w;
    lv_coord_t cur_w = 0;  /* Pixel Width of transversed string */
    uint32_t word_len = 0;   /* Number of characters in the transversed word */
    uint32_t break_index = NO_BREAK_FOUND; /* only used for "long" words */
    uint32_t break_letter_count = 0; /* Number of characters up to the long word break point */

    letter = _lv_txt_encoded_next(txt, &i_next);
    i_next_next = i_next;

    /* Obtain the full word, regardless if it fits or not in max_width */
    while(txt[i] != '\0') {
        letter_next = _lv_txt_encoded_next(txt, &i_next_next);
        word_len++;

        /*Handle the recolor command*/
        if((flag & LV_TXT_FLAG_RECOLOR) != 0) {
            if(_lv_txt_is_cmd(cmd_state, letter) != false) {
                i = i_next;
                i_next = i_next_next;
                letter = letter_next;
                continue;   /*Skip the letter is it is part of a command*/
            }
        }

        // letter_w = lv_font_get_glyph_width(font, letter, letter_next);
		lv_font_glyph_dsc_t g;
		bool ret;
		ret = font->get_glyph_dsc(font, &g, letter, letter_next);
		letter_w = ret ? g.adv_w : 0;
	
        cur_w += letter_w;

        if(letter_w > 0) {
            cur_w += letter_space;
        }

        /* Test if this character fits within max_width */
        if(break_index == NO_BREAK_FOUND && (cur_w - letter_space) > max_width) {
            break_index = i;
            break_letter_count = word_len - 1;
            /* break_index is now pointing at the character that doesn't fit */
        }

        /*Check for new line chars and breakchars*/
        if(letter == '\n' || letter == '\r' || is_break_char(letter)) {
            /* Update the output width on the first character if it fits.
             * Must do this here in case first letter is a break character. */
            if(i == 0 && break_index == NO_BREAK_FOUND && word_w_ptr != NULL) *word_w_ptr = cur_w;
            word_len--;
            break;
        }

        /* Update the output width */
        if(word_w_ptr != NULL && break_index == NO_BREAK_FOUND) *word_w_ptr = cur_w;

        i = i_next;
        i_next = i_next_next;
        letter = letter_next;
    }

    /* Entire Word fits in the provided space */
    if(break_index == NO_BREAK_FOUND) {
        if(word_len == 0 || (letter == '\r' && letter_next == '\n')) i = i_next;
        return i;
    }

    if(force) return break_index;
    if(word_w_ptr != NULL) *word_w_ptr = 0; /* Return no word */
    (void) break_letter_count;
    return 0;
}

uint32_t __quick_lv_txt_get_next_line(const char * txt, const lv_font_t * font,
                               lv_coord_t letter_space, lv_coord_t max_width, lv_txt_flag_t flag)
{
    if(txt == NULL) return 0;
    if(font == NULL) return 0;

    /* If max_width doesn't mater simply find the new line character
     * without thinking about word wrapping*/
    if((flag & LV_TXT_FLAG_EXPAND) || (flag & LV_TXT_FLAG_FIT)) {
        uint32_t i;
        for(i = 0; txt[i] != '\n' && txt[i] != '\r' && txt[i] != '\0'; i++) {
            /*Just find the new line chars or string ends by incrementing `i`*/
        }
        if(txt[i] != '\0') i++;    /*To go beyond `\n`*/
        return i;
    }

    if(flag & LV_TXT_FLAG_EXPAND) max_width = LV_COORD_MAX;
    lv_txt_cmd_state_t cmd_state = LV_TXT_CMD_STATE_WAIT;
    uint32_t i = 0;                                        /* Iterating index into txt */

    while(txt[i] != '\0' && max_width > 0) {
        uint32_t word_w = 0;
        uint32_t advance = __quick_lv_txt_get_next_word(&txt[i], font, letter_space, max_width, flag, &word_w, &cmd_state, i == 0);
        max_width -= word_w;

        if(advance == 0) {
            if(i == 0) _lv_txt_encoded_next(txt, &i); // prevent inf loops
            break;
        }

        i += advance;

        if(txt[0] == '\n' || txt[0] == '\r') break;

        if(txt[i] == '\n' || txt[i] == '\r') {
            i++;  /* Include the following newline in the current line */
            break;
        }
    }

    /* Always step at least one to avoid infinite loops */
    if(i == 0) {
        _lv_txt_encoded_next(txt, &i);
    }

    return i;
}

static lv_coord_t __quick_lv_txt_get_height(const char * text, const lv_font_t * font, lv_coord_t letter_space, lv_coord_t line_space, lv_coord_t max_width, lv_txt_flag_t flag)
{
    lv_coord_t height = 0;

    if(text == NULL) return 0;
    if(font == NULL) return 0;

    if(flag & LV_TXT_FLAG_EXPAND) max_width = LV_COORD_MAX;

    uint32_t line_start     = 0;
    uint32_t new_line_start = 0;
    uint16_t letter_height = lv_font_get_line_height(font);

    /*Calc. the height and longest line*/
    while(text[line_start] != '\0') {
        new_line_start += _lv_txt_get_next_line(&text[line_start], font, letter_space, max_width, flag);

        if((unsigned long)height + (unsigned long)letter_height + (unsigned long)line_space > LV_MAX_OF(lv_coord_t)) {
            LV_LOG_WARN("lv_txt_get_size: integer overflow while calculating text height");
            return 0;
        }
        else {
            height += letter_height;
            height += line_space;
        }

        line_start  = new_line_start;
    }

    /*Make the text one line taller if the last character is '\n' or '\r'*/
    if((line_start != 0) && (text[line_start - 1] == '\n' || text[line_start - 1] == '\r')) {
        height += letter_height + line_space;
    }

    /*Correction with the last line space or set the height manually if the text is empty*/
    if(height == 0)
        height = letter_height;
    else
        height -= line_space;
	return height;
}


#endif
