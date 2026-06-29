#ifndef IMGBTNPLUGIN_H
#define IMGBTNPLUGIN_H

#define SAMEZOOM    (123321.0)
#define SAMEANGLE   (10000)

lv_img_dsc_t* getZoomPicture(const void* d, double zoom, int bits=LV_COLOR_DEPTH, bool center=true, lv_img_dsc_t* prevPic=NULL);
int setImgObjZoomAngle(lv_obj_t* obj, double newZoom=SAMEZOOM, double newAngle=SAMEANGLE);
int setImgObjZoom(lv_obj_t* obj, double newZoom);
int setImgObjAngle(lv_obj_t* obj, double newAngle);
void clearZoomPicture(lv_img_dsc_t* p);

#endif