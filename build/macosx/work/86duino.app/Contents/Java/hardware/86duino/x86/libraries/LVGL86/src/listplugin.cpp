#include <stdio.h>
#include <stdlib.h>
#include "listplugin.h"

/* ================================================= */
/*                      List                         */
/* ================================================= */

LIST_ll* lll_head = NULL;

static void add_LIST_LL_node(LIST_ll* node) {
    if (lll_head == NULL) {
        lll_head = node;
        node->next = NULL;
    } else {
        node->next = lll_head;
        lll_head = node;
    }
}

static bool isExistList(lv_obj_t* list, LIST_ll** ret) {
    LIST_ll* p = lll_head;
    if (p == NULL || list == NULL) return false;
    while((p != NULL) && (p->listObj != list)) p = p->next;
    if ((p != NULL) && (p->listObj == list)) {
        if (ret != NULL) *ret = p;
        return true;
    }
    return false;
}

static void remove_LIST_LL_node(LIST_ll* node) {
    int i;
    if (node == NULL) return;
    
    LIST_ll* p = lll_head;
    LIST_ll* prev_node = NULL; // CLI compiler need initial value, so set it as NULL
    if (lll_head == node) {
        lll_head = node->next;
    } else {
        while(p != node && p != NULL) {prev_node = p; p = p->next;}
        if (p == NULL) return; // no found anyone
        prev_node->next = node->next;
    }
    free(node);
}

/* ================================================= */
/*                  List Button                      */
/* ================================================= */

static void add_LISTBTN_LL_node(lv_obj_t* listObj, LISTBTN_ll* node) {
    LIST_ll* ret = NULL;
    if (!isExistList(listObj, &ret)) return;
    
    if (ret->llb_head == NULL) {
        ret->llb_head = node;
        node->next = NULL;
    } else {
        node->next = ret->llb_head;
        ret->llb_head = node;
    }
}

static bool isExistListBtn(lv_obj_t* listObj, lv_obj_t* listBtn, LISTBTN_ll** ret) {
    LIST_ll* lret = NULL;
    if (!isExistList(listObj, &lret)) return false;
    
    LISTBTN_ll* p = lret->llb_head;
    if (p == NULL || listBtn == NULL) return false;
    while((p != NULL) && (p->btnObj != listBtn)) p = p->next;
    if ((p != NULL) && (p->btnObj == listBtn)) {
        if (ret != NULL) *ret = p;
        return true;
    }
    return false;
}

static void remove_LISTBTN_LL_node(lv_obj_t* listObj, LISTBTN_ll* node) {
    LIST_ll* ret = NULL;
    if (!isExistList(listObj, &ret)) return;
    
    LISTBTN_ll* p = ret->llb_head;
    LISTBTN_ll* prev_node = NULL; // CLI compiler need initial value, so set it as NULL
    if (ret->llb_head == node) {
        ret->llb_head = node->next;
    } else {
        while(p != node && p != NULL) {prev_node = p; p = p->next;}
        if (p == NULL) return; // no found anyone
        prev_node->next = node->next;
    }
    free(node);
}



/* ================================================= */
/*                     MY APIs                       */
/* ================================================= */
static void myListEvent(lv_obj_t* obj, lv_event_t event) {
    LIST_ll* list = (LIST_ll*)lv_obj_get_user_data(obj);
    
    // call user data
    if (list->userCallback) list->userCallback(obj, event);
}

void list_settings(lv_obj_t* listObj, int mode, void (*userCallback)(lv_obj_t* obj, lv_event_t event), void* listUserData) {
    LIST_ll* newNode = NULL;
    LIST_ll* ret = NULL;
    
    if (!isExistList(listObj, &ret)) {
        newNode = (LIST_ll*)malloc(sizeof(LIST_ll));
        memset(newNode, 0, sizeof(*newNode));
        newNode->listObj = listObj;
        newNode->llb_head = NULL;
        if (mode > 0) newNode->mode = mode;
        if (userCallback) newNode->userCallback = userCallback;
        if (listUserData) newNode->userData = listUserData;
        
        lv_obj_set_user_data(listObj, (void*)newNode);
        lv_obj_set_event_cb(listObj, myListEvent);
        
        add_LIST_LL_node(newNode);
    } else { // update data
        if (mode > 0) ret->mode = mode;
        // ret->llb_head = NULL; // 不動它
        if (userCallback) ret->userCallback = userCallback;
        if (listUserData) ret->userData = listUserData;
    }
}

void set_list_mode(lv_obj_t* listObj, int mode) {
    list_settings(listObj, mode, NULL, NULL);
}

void set_list_event(lv_obj_t* listObj, void (*userCallback)(lv_obj_t* obj, lv_event_t event)) {
    list_settings(listObj, -1, userCallback, NULL);
}

void set_list_user_data(lv_obj_t* listObj, void* listUserData) {
    list_settings(listObj, -1, NULL, listUserData);
}

void free_list_all_btns(lv_obj_t* listObj, bool removeList) {
    LIST_ll* ret = NULL;
    if (isExistList(listObj, &ret)) {
        LISTBTN_ll* tmp = ret->llb_head; // Free All Buttun state
        while (tmp != NULL) {
            LISTBTN_ll* _needFree = tmp;
            tmp = tmp->next;
            free(_needFree);
        }
        ret->llb_head = NULL;
    }
    
    if (removeList) remove_LIST_LL_node(ret);
}

void* get_list_user_data(lv_obj_t* listObj) {
    LIST_ll* p = lll_head;
    
    while (p != NULL && p->listObj != listObj) p = p->next;
    if (p != NULL) return p->userData;
    
    return NULL;
}

static void myListBtnEvent(lv_obj_t* obj, lv_event_t event) {
    LISTBTN_ll* listBtn = (LISTBTN_ll*)lv_obj_get_user_data(obj);
    
    if (event == LV_EVENT_CLICKED) {
        if (event == LV_EVENT_CLICKED && listBtn) listBtn->isSelected = !listBtn->isSelected;
        
        lv_obj_t* list = lv_obj_get_parent(lv_obj_get_parent(obj));
        LIST_ll* listll = (LIST_ll*)lv_obj_get_user_data(list);
        lv_obj_t* prev_btn = NULL;
        
        if (listll->mode == MODE_DEFAULT) {
            
            // Nothing Change
            
        } else if (listll->mode == MODE_ONESEL) {
            prev_btn = lv_list_get_next_btn(list, NULL);
            while (prev_btn != NULL) {
                listBtn = (LISTBTN_ll*)lv_obj_get_user_data(prev_btn);
                if (listBtn && prev_btn == obj) {
                    if (listBtn->isSelected)
                        lv_obj_set_state(prev_btn, LV_STATE_CHECKED);
                    else
                        lv_obj_set_state(prev_btn, LV_STATE_DEFAULT);
                } else {
                    listBtn->isSelected = false;
                    lv_obj_set_state(prev_btn, LV_STATE_DEFAULT);
                }
                prev_btn = lv_list_get_next_btn(list, prev_btn); // get next one
            }
        } else if (listll->mode == MODE_MULTISEL) {
            prev_btn = lv_list_get_next_btn(list, NULL);
            while (prev_btn != NULL) {
                listBtn = (LISTBTN_ll*)lv_obj_get_user_data(prev_btn);
                if (listBtn && listBtn->isSelected)
                    lv_obj_set_state(prev_btn, LV_STATE_CHECKED);
                else
                    lv_obj_set_state(prev_btn, LV_STATE_DEFAULT);
                prev_btn = lv_list_get_next_btn(list, prev_btn); // get next one
            }
        }
    }
    
    // Call user function
    if (listBtn && listBtn->userCallback) listBtn->userCallback(obj, event);
}

lv_obj_t* list_btn_settings(lv_obj_t* btnObj,
                 void (*userCallback)(lv_obj_t* obj, lv_event_t event),
                 void* listBtnUserData) {

    LIST_ll* ret = NULL;
    LISTBTN_ll* bret = NULL;
    lv_obj_t* listObj = lv_obj_get_parent(lv_obj_get_parent(btnObj));
    
    if (isExistList(listObj, &ret)) {
        if (!isExistListBtn(listObj, btnObj, &bret)) {
            LISTBTN_ll* newNode = (LISTBTN_ll*)malloc(sizeof(LISTBTN_ll));
            memset(newNode, 0, sizeof(*newNode));
            newNode->btnObj = btnObj;
            newNode->isSelected = false;
            if (userCallback) newNode->userCallback = userCallback;
            if (listBtnUserData) newNode->userData = listBtnUserData;
            
            lv_obj_set_user_data(btnObj, (void*)newNode);
            lv_obj_set_event_cb(btnObj, myListBtnEvent);
            
            add_LISTBTN_LL_node(listObj, newNode);
        } else {
            if (userCallback) bret->userCallback = userCallback;
            if (listBtnUserData) bret->userData = listBtnUserData;
        }
    }
    
    return btnObj;
}

void set_list_btn_event(lv_obj_t* btnObj, void (*userCallback)(lv_obj_t* obj, lv_event_t event)) {
    list_btn_settings(btnObj, userCallback, NULL);
}

void set_list_btn_user_data(lv_obj_t* btnObj, void* listBtnUserData) {
    list_btn_settings(btnObj, NULL, listBtnUserData);
}

void* get_list_btn_user_data(lv_obj_t* btnObj) {
    LIST_ll* p = lll_head;
    LISTBTN_ll* n = NULL;
    
    while (p != NULL) {
        if (p->llb_head != NULL) {
            n = p->llb_head;
            while (n != NULL && n->btnObj != btnObj) n = n->next;
            if (n != NULL) return n->userData;
        }
        p = p->next;
    }
    
    return NULL;
}

bool is_list_btn_selected(lv_obj_t* btnObj) {
    LIST_ll* p = lll_head;
    LISTBTN_ll* n = NULL;
    
    while (p != NULL) {
        if (p->llb_head != NULL) {
            n = p->llb_head;
            while (n != NULL && n->btnObj != btnObj) n = n->next;
            if (n != NULL) return n->isSelected;
        }
        p = p->next;
    }
    
    return false;
}

void list_clean(lv_obj_t* listObj) {
    lv_list_clean(listObj);
    free_list_all_btns(listObj, false);
}

void list_destroy(lv_obj_t* listObj) {
    lv_obj_del(listObj);
    free_list_all_btns(listObj, true);
}
