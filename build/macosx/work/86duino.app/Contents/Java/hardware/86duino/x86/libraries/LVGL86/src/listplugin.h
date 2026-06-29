#include "lvgl.h"

enum {
    MODE_DEFAULT = 1,
    MODE_ONESEL,
    MODE_MULTISEL
};

typedef struct _LISTBTN_ll {
    lv_obj_t* btnObj;
    bool isSelected;
    void (*userCallback)(lv_obj_t* obj, lv_event_t event);
    void* userData;
    _LISTBTN_ll* next;
} LISTBTN_ll;

typedef struct _LIST_ll {
    lv_obj_t* listObj;
    int mode;
    LISTBTN_ll* llb_head;
    void (*userCallback)(lv_obj_t* obj, lv_event_t event);
    void* userData;
    _LIST_ll* next;
} LIST_ll;




/* ================================================= */
/*                    List APIs                      */
/* ================================================= */
void set_list_mode(lv_obj_t* listObj, int mode);
void set_list_event(lv_obj_t* listObj, void (*userCallback)(lv_obj_t* obj, lv_event_t event));
void set_list_user_data(lv_obj_t* listObj, void* listUserData);
void* get_list_user_data(lv_obj_t* listObj);
void list_settings(lv_obj_t* listObj, int mode, void (*userCallback)(lv_obj_t* obj, lv_event_t event), void* listUserData);
void list_clean(lv_obj_t* listObj);
void list_destroy(lv_obj_t* listObj);

/* ================================================= */
/*                List Button APIs                   */
/* ================================================= */
void set_list_btn_event(lv_obj_t* btnObj, void (*userCallback)(lv_obj_t* obj, lv_event_t event));
void set_list_btn_user_data(lv_obj_t* btnObj, void* listBtnUserData);
void* get_list_btn_user_data(lv_obj_t* btnObj);
bool is_list_btn_selected(lv_obj_t* btnObj);
lv_obj_t* list_btn_settings(lv_obj_t* btnObj, void (*userCallback)(lv_obj_t* obj, lv_event_t event), void* listBtnUserData);

