/**
 * @file lv_freetype.c
 *
 */
 
#ifndef WIN32
#define USE_HASHLIB	1
#endif
/*********************
*      INCLUDES
*********************/
#include "lv_freetype.h"

#if USE_HASHLIB
#include "uthash.h"
#endif
/*********************
*      DEFINES
*********************/
static char* fa_path = NULL; // "fa-solid.ttf", "fa-brands.ttf", "fa-regular.ttf"
typedef struct _FT_ll {
    lv_font_t* f;
    FT_Library* library;
    uint8_t* bitmap[65536];
    lv_font_glyph_dsc_t* dsc[65536];
    struct _FT_ll* next;
    struct _FT_ll* fontawesome;
#if USE_HASHLIB
    UT_hash_handle hh;
#endif
} FT_LL;

FT_LL* ll_head = NULL;

/**********************
 *      TYPEDEFS
 **********************/

/**********************
*  STATIC PROTOTYPES
**********************/
static const uint8_t* lv_font_get_bitmap_fmt_freetype(const lv_font_t* font, uint32_t unicode_letter);
static bool lv_font_get_glyph_dsc_fmt_freetype(const lv_font_t* font, lv_font_glyph_dsc_t* dsc_out, uint32_t unicode_letter, uint32_t unicode_letter_next);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
*      MACROS
**********************/

/**********************
*   GLOBAL FUNCTIONS
**********************/

static void add_FT_LL_node(FT_LL* node) {

#if USE_HASHLIB
    HASH_ADD_PTR(ll_head, f, node);
#else
    if (ll_head == NULL) {
        ll_head = node;
        node->next = NULL;
    }
    else {
        node->next = ll_head;
        ll_head = node;
    }
#endif
}

static bool isExistFont(lv_font_t* font, FT_LL** dec) {
#if USE_HASHLIB
    FT_LL* p;
    HASH_FIND_PTR(ll_head, &font, p);
    if (dec != NULL) *dec = p;
    return p != NULL;
#else
    FT_LL* p = ll_head;
    if (p == NULL || font == NULL) return false;
    while ((p != NULL) && (p->f != font)) p = p->next;
    if ((p != NULL) && (p->f == font)) {
        if (dec != NULL) *dec = p;
        return true;
    }
    return false;
#endif
}

static void remove_FT_LL_node(FT_LL* node) {
#if USE_HASHLIB
    int i;
    if (node == NULL) return;
    FT_LL* p;
    HASH_FIND_PTR(ll_head, &node, p);
    if (p)
    {
        HASH_DEL(ll_head, p);
        free(p);
    }
#else
    FT_LL* p = ll_head;
    FT_LL* prev_node = NULL; // CLI compiler need initial value, so set it as NULL
    if (ll_head == node) {
        ll_head = node->next;
    }
    else {
        while (p != node && p != NULL) { prev_node = p; p = p->next; }
        if (p == NULL) return; // no found anyone
        prev_node->next = node->next;
    }
    free(node);
#endif
}

/*uint32_t to_unicode(char* utf8)
{
    uint32_t unicode = 0;
    uint8_t first_byte = utf8[0];
    uint8_t len = 
        (first_byte >> 7) == 0 ? 1 :
        (first_byte & 0xf0) == 0xf0 ? 4 :
        (first_byte & 0xe0) == 0xe0 ? 3 :
        (first_byte & 0xc0) == 0xc0 ? 2 : 0
        ;
    unicode += (uint8_t)(first_byte << len) >> len;
    for(int i = 1; i < len; ++i)
    {
      unicode <<= 6;
      unicode += ((uint8_t)utf8[i]) & 0x3F;
    }
    
    return unicode;
}*/


static FT_LL * useFontAwesome(int size);
/*
void changeIconSize(int size) {
    // reload different size fontawesome : 
    // 1. free it
    // 2. init it with new size
    // 3. registe it
    if (fontawesome != NULL) {
        lv_freetype_font_free(fontawesome);
        useFontAwesome(size);
    }
}
*/

int lv_freetype_init(char * icon_path)
{
    int i;
    char *temp = NULL;

    if (icon_path != NULL) {
        for (i=0; icon_path[i] != '\0'; i++);
        temp = (char*) malloc(sizeof(char)*(i+1));
        if (temp == NULL) return 0xE0;
        if (fa_path != NULL) {
            free(fa_path);
        }
        fa_path = temp;
        for (i=0; icon_path[i] != '\0'; i++) fa_path[i] = icon_path[i];
        fa_path[i] = '\0';
    }
    
    return FT_Err_Ok;
}

static int _lv_freetype_font_init(lv_font_t * font, const char * font_path, uint16_t size)
{
    int i, error;
    
    if (font == NULL) return 0xE0;
    
    if (isExistFont(font, NULL)) {
        printf("The font is exist and initialize before.\n");
        return 0xE0;
    }
    
    lv_font_fmt_freetype_dsc_t * dsc = lv_mem_alloc(sizeof(lv_font_fmt_freetype_dsc_t));
    LV_ASSERT_MEM(dsc);
    if(dsc == NULL) return FT_Err_Out_Of_Memory;

    FT_Library * library = lv_mem_alloc(sizeof(FT_Library));
    if (library == NULL) 
    {
        printf("Init freetype font library fail.\n");
        return FT_Err_Out_Of_Memory;
    }
    
    error = FT_Init_FreeType( library );
    if ( error )
    {
        printf("Error in FT_Init_FreeType: %d\n", error);
        return error;
    }
    
    error = FT_New_Face( *library,
            font_path,    /* first byte in memory */
            0,         /* face_index           */
            &dsc->face );
    if ( error ) {
        printf("Error in FT_New_Face: %d\n", error);
        return error;
    }

    error = FT_Set_Char_Size(
            dsc->face,    /* handle to face object           */
            0,       /* char_width in 1/64th of points  */
            size*64,   /* char_height in 1/64th of points */
            300,     /* horizontal device resolution    */
            300 );   /* vertical device resolution      */

    if ( error ) {
        printf("Error in FT_Set_Char_Size: %d\n", error);
        return error;
    }

    error = FT_Set_Pixel_Sizes(
            dsc->face,   /* handle to face object */
            0,      /* pixel_width           */
            size );   /* pixel_height          */

    if ( error ) {
        printf("Error in FT_Set_Char_Size: %d\n", error);
        return error;
    }

    font->get_glyph_dsc = lv_font_get_glyph_dsc_fmt_freetype;    /*Function pointer to get glyph's data*/
    font->get_glyph_bitmap = lv_font_get_bitmap_fmt_freetype;    /*Function pointer to get glyph's bitmap*/
    font->line_height = dsc->face->size->metrics.height/64;          /*The maximum line height required by the font*/
    font->base_line = -dsc->face->size->metrics.descender / 64;             /*Baseline measured from the bottom of the line*/
    font->dsc = dsc;           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
    font->subpx = 0;

    FT_LL* newNode = (FT_LL*) malloc(sizeof(FT_LL));
    if (newNode != NULL) {
        newNode->f = font;
        newNode->library = library;
        for (i=0; i<65536; i++) newNode->bitmap[i] = NULL;
        for (i=0; i<65536; i++) newNode->dsc[i] = NULL;
        newNode->fontawesome = NULL;
        add_FT_LL_node(newNode);
    } else {
        printf("malloc FT_LL struct fail: got NULL\n");
    }
    
    return FT_Err_Ok;
}

int lv_freetype_font_init(lv_font_t * font, const char * font_path, uint16_t size) {
    int res;
    FT_LL * p;
    if (size == 0) return 0xE0;
    
    res = _lv_freetype_font_init(font, font_path, size);
    if (res == FT_Err_Ok && fa_path != NULL && isExistFont(font, &p)) p->fontawesome = useFontAwesome(size); // Note : the "icon font" always follows before font
    return res;
}

int lv_freetype_font_free(lv_font_t * font) {
    FT_LL* p;
    FT_LL* fa;
    int i;
    
    if (font == NULL) return 0xE0;
    
    if (!isExistFont(font, &p)) {
        printf("The font is not exist.\n");
        return 0xE0;
    }
    
    // free the fontawesome if we have one
    if ((fa = p->fontawesome) != NULL) {
        for (i=0; i<65536; i++) {
            if (fa->bitmap[i] != NULL) free(fa->bitmap[i]);
            if (fa->dsc[i] != NULL) free(fa->dsc[i]);
        }
        lv_mem_free(fa->f->dsc);
        free(fa->f);
        remove_FT_LL_node(fa);
    }
    
    for (i=0; i<65536; i++) {
        if (p->bitmap[i] != NULL) free(p->bitmap[i]);
        if (p->dsc[i] != NULL) free(p->dsc[i]);
    }
    
    FT_Done_FreeType(*(p->library));
    remove_FT_LL_node(p);
    lv_mem_free(font->dsc);
    
    return FT_Err_Ok;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static FT_LL * useFontAwesome(int size) {
    // init LVGL86 Icon font : free fontawesome 5.X, default size is 14
    FT_LL * p;
    lv_font_t * fontawesome = (lv_font_t*) malloc(sizeof(lv_font_t));
    if (fontawesome != NULL && _lv_freetype_font_init(fontawesome, fa_path, size) != FT_Err_Ok) {// 14: small font    
        free(fontawesome);
        fontawesome = NULL;
    }
    if (fontawesome != NULL && isExistFont(fontawesome, &p)) return p;
    return NULL;
    // Other fonAwesome files
    // lv_font_t* fontawesome2 = (lv_font_t*) malloc(sizeof(lv_font_t));
    // lv_freetype_font_init(fontawesome2, fa_path2, size);
    // lv_font_t* fontawesome3 = (lv_font_t*) malloc(sizeof(lv_font_t));
    // lv_freetype_font_init(fontawesome3, fa_path3, size);
}

static void my_add_font_bitmap(FT_LL * p, uint32_t unicode_letter) {
    int i, error;
    
    lv_font_fmt_freetype_dsc_t * fdsc = (lv_font_fmt_freetype_dsc_t *) p->f->dsc;

    FT_UInt glyph_index = FT_Get_Char_Index(fdsc->face, unicode_letter );

    error = FT_Load_Glyph(
            fdsc->face,          /* handle to face object */
            glyph_index,   /* glyph index           */
            FT_LOAD_DEFAULT );  /* load flags, see below */

    if ( error ) {
        printf("Error in FT_Load_Glyph: %d\n", error);
        return;
    }

    error = FT_Render_Glyph(fdsc->face->glyph,   /* glyph slot  */
            FT_RENDER_MODE_NORMAL ); /* render mode */

    if ( error ) {
        printf("Error in FT_Render_Glyph: %d\n", error);
        return;
    }
    
    
    //printf("bmap_rows: %d, width:%d, box_w:%d, box_h: %d\n",fdsc->face->glyph->bitmap.rows, fdsc->face->glyph->bitmap.width, fdsc->face->glyph->metrics.width / 64, fdsc->face->glyph->metrics.height / 64);
    int buf_size = fdsc->face->glyph->bitmap.rows * fdsc->face->glyph->bitmap.width;
    uint8_t* buf = (uint8_t*) malloc(sizeof(uint8_t) * buf_size);
    if (buf == NULL)
    {
        printf("malloc buffer fail\n");
        return;
    }
    
    for (i=0; i<buf_size; i++)
        buf[i] = fdsc->face->glyph->bitmap.buffer[i];
    
    if (unicode_letter > 0xFFFF)
    {
        printf("The number of UNICODE letter > 0xFFFF, it is not added in _ft_bitmap[].\n");
        free(buf);
        return;
    }
    
    p->bitmap[unicode_letter] = buf;
}


/**
 * Used as `get_glyph_bitmap` callback in LittelvGL's native font format if the font is uncompressed.
 * @param font pointer to font
 * @param unicode_letter an unicode letter which bitmap should be get
 * @return pointer to the bitmap or NULL if not found
 */
static const uint8_t * lv_font_get_bitmap_fmt_freetype(const lv_font_t * font, uint32_t unicode_letter)
{
    int i;
    uint8_t* d;
    FT_LL * p = NULL;
    
    isExistFont(font, &p);
    
    // if 0xF000 < unicode < 0xF900, use the free FontAwesome ttf automatically.
    if (unicode_letter > 0xF000 && unicode_letter < 0xF900 && p->fontawesome != NULL) {
        font = p->fontawesome->f;
        p = p->fontawesome;
    }
    
    if (p) {
        if (p->bitmap[unicode_letter] == NULL)
            my_add_font_bitmap(p, unicode_letter);
    } else {
        printf("The font type that you want to get is not found in bitmap list\n");
        return NULL;
    }

    return p->bitmap[unicode_letter];
}

lv_font_glyph_dsc_t* my_add_font_dsc(FT_LL * p, uint32_t unicode_letter, uint32_t unicode_letter_next) {
    lv_font_fmt_freetype_dsc_t * fdsc;
    
    fdsc = (lv_font_fmt_freetype_dsc_t *) p->f->dsc;

    int i, error = 0;
//    error = render_glyph(unicode_letter, fdsc);

    FT_UInt glyph_index = FT_Get_Char_Index(fdsc->face, unicode_letter );

    error = FT_Load_Glyph(
            fdsc->face,          /* handle to face object */
            glyph_index,   /* glyph index           */
            FT_LOAD_DEFAULT );  /* load flags, see below */
    if ( error )
    {
        printf("Error in FT_Load_Glyph: %d\n", error);
        return NULL;
    }


    if (error) return NULL;

    FT_Vector akern;
    error = FT_Get_Kerning(fdsc->face, unicode_letter, unicode_letter_next,
            FT_KERNING_DEFAULT, &akern);

    if (error) return NULL;

    int32_t adv_w = (fdsc->face->glyph->advance.x + akern.x + 32) / 64;
    
    lv_font_glyph_dsc_t* d = (lv_font_glyph_dsc_t*) malloc(sizeof(lv_font_glyph_dsc_t));
    if (d == NULL)
    {
        printf("malloc lv_font_glyph_dsc_t fail\n");
        return NULL;
    }
    
    d->adv_w = adv_w;
    d->box_w = fdsc->face->glyph->metrics.width / 64;
    d->box_h = fdsc->face->glyph->metrics.height / 64;
    d->ofs_x = fdsc->face->glyph->metrics.horiBearingX / 64;
    d->ofs_y = fdsc->face->glyph->metrics.horiBearingY / 64;
    d->bpp   = 8;
    
    p->dsc[unicode_letter] = d;
    
    return d;
}

/**
 * Used as `get_glyph_dsc` callback in LittelvGL's native font format if the font is uncompressed.
 * @param font_p pointer to font
 * @param dsc_out store the result descriptor here
 * @param letter an UNICODE letter code
 * @return true: descriptor is successfully loaded into `dsc_out`.
 *         false: the letter was not found, no data is loaded to `dsc_out`
 */
static bool lv_font_get_glyph_dsc_fmt_freetype(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter, uint32_t unicode_letter_next)
{
    // printf("dsc: %08X %08X\n", unicode_letter, unicode_letter_next);
    int i;
    FT_LL * p = NULL;
    if(unicode_letter < 0x20) return false;

    isExistFont(font, &p);
    
    // if 0xF000 < unicode < 0xF900, use the free FontAwesome ttf automatically.
    if (unicode_letter > 0xF000 && unicode_letter < 0xF900 && p->fontawesome != NULL) {
        font = p->fontawesome->f;
        p = p->fontawesome;
    }
    
    if (p) { // Find this font
      if (p->dsc[unicode_letter] == NULL) // but DSC is NULL
        my_add_font_dsc(p, unicode_letter, unicode_letter_next); // Add NEW DSC in FT_LL->DSC
    } else { // NO find this font
        printf("The font type that you want to get is not found in DSC list.\n");
        return false;
    }

    dsc_out->adv_w = p->dsc[unicode_letter]->adv_w;
    dsc_out->box_w = p->dsc[unicode_letter]->box_w;
    dsc_out->box_h = p->dsc[unicode_letter]->box_h;
    dsc_out->ofs_x = p->dsc[unicode_letter]->ofs_x;
    dsc_out->ofs_y = p->dsc[unicode_letter]->ofs_y - dsc_out->box_h;
    dsc_out->bpp   = p->dsc[unicode_letter]->bpp;

    return true;
}