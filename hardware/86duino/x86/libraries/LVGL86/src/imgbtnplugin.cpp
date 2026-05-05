#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "lvgl.h"
#include "imgbtnplugin.h"

#define AUTOFREE_PREV_BITMAP // if call getZoomPicture() continually, it will auto clear the previous zoomed picture by itself.

static void BMzoom16_ALPHA(uint8_t* srcBmp, int srcwidth, int srcheight, double percent, bool center, uint8_t** outBmp) {
  double factor;
  double fi, fi2;
  double sx1, sx2, sy1, sy2, dx, dy;
  double destR, destG, destB, destA, sR, sG, sB, sA, PC;
  int destwidth, destheight;
  int ofs;
  uint8_t* destBmp;
  uint8_t* dest2Bmp;

  destwidth = round(((double)srcwidth) * percent);
  factor = (double)srcwidth / (double)destwidth;
  fi = 1.0/factor;
  fi2 = fi*fi;
  destheight = trunc((double)srcheight/factor);
  destBmp = (uint8_t*) malloc((sizeof(lv_color16_t)+1)*destwidth*destheight);
  
  for (int y=0; y<destheight; y++) {         //vertical destination pixels
    sy1 = factor * y;
    sy2 = sy1 + factor;
    for (int x=0; x<destwidth; x++) {        //horizontal destination pixels
      sx1 = factor * x;
      sx2 = sx1 + factor;
      destR=0.0; destG=0.0; destB=0.0; destA=0.0;
      for (int j=floor(sy1); j<ceil(sy2); j++) {  //vertical source pixels
        dy = 1.0;
        if (sy1 > j) dy = dy-(sy1-j);
        if (sy2 < (j+1)) dy = dy-(j+1-sy2);
        for (int i=floor(sx1); i<ceil(sx2); i++) { //horizontal source pixels
          dx = 1.0;
          if (sx1 > i) dx = dx-(sx1-i);
          if (sx2 < (i+1)) dx = dx-(i+1-sx2);
          ofs = (i+j*srcwidth)*3;
          sR = srcBmp[ofs] & 0x1f;
          sG = ((srcBmp[ofs+1] & 0x07) << 3) | ((srcBmp[ofs] & 0xE0) >> 5) ;
          sB = (srcBmp[ofs+1] & 0xf8) >> 3;
          sA = srcBmp[ofs+2];
          PC = dx*dy*fi2;
          destR = destR + sR*PC;
          destG = destG + sG*PC;
          destB = destB + sB*PC;
          destA = destA + sA*PC;
        }
      }
      ofs = (x+y*destwidth)*3;
      *(destBmp+ofs) = ((uint8_t)trunc(destR) & 0x1f) | (((uint8_t)trunc(destG) & 0x07) << 5);
      *(destBmp+ofs+1) = (((uint8_t)trunc(destG) & 0x38) >> 3) | (((uint8_t)trunc(destB) & 0x1f) << 3);
      *(destBmp+ofs+2) = (uint8_t)trunc(destA);
    }
  }

  if (srcwidth > destwidth && center) {
    dest2Bmp = (uint8_t*) malloc((sizeof(lv_color16_t)+1)*srcwidth*srcheight);
    // shift my pic to new start point
    dx = (srcwidth-destwidth)/2;
    dy = (srcheight-destheight)/2;
    ofs = (dx+dy*srcwidth)*3;
    for (int i=0; i<srcwidth*srcheight*3; i++) dest2Bmp[i] = 0x00;// fill transp point
    for (int i=0; i<destwidth*destheight; i++, ofs+=3) {
      dest2Bmp[ofs] = destBmp[i*3];
      dest2Bmp[ofs+1] = destBmp[i*3+1];
      dest2Bmp[ofs+2] = destBmp[i*3+2];
      if (i!=0 && (i%destwidth) == (destwidth-1)) {
        ofs = ofs + (srcwidth-destwidth)*3;
      }
    }
    free(destBmp);
    *outBmp = dest2Bmp;
    return;
  }
  
  *outBmp = destBmp;
}

static void BMzoom32_ALPHA(uint8_t* srcBmp, int srcwidth, int srcheight, double percent, bool center, uint8_t** outBmp) {
  double factor;
  double fi, fi2;
  double sx1, sx2, sy1, sy2, dx, dy;
  double destR, destG, destB, destA, sR, sG, sB, sA, PC;
  int destwidth, destheight;
  int ofs;
  uint8_t* destBmp;
  uint8_t* dest2Bmp;

  destwidth = round(((double)srcwidth) * percent);
  factor = (double)srcwidth / (double)destwidth;
  fi = 1.0/factor;
  fi2 = fi*fi;
  destheight = trunc((double)srcheight/factor);
  destBmp = (uint8_t*) malloc(sizeof(lv_color32_t)*destwidth*destheight);
  
  for (int y=0; y<destheight; y++) {         //vertical destination pixels
    sy1 = factor * y;
    sy2 = sy1 + factor;
    for (int x=0; x<destwidth; x++) {        //horizontal destination pixels
      sx1 = factor * x;
      sx2 = sx1 + factor;
      destR=0.0; destG=0.0; destB=0.0; destA=0.0;
      for (int j=floor(sy1); j<ceil(sy2); j++) {  //vertical source pixels
        dy = 1.0;
        if (sy1 > j) dy = dy-(sy1-j);
        if (sy2 < (j+1)) dy = dy-(j+1-sy2);
        for (int i=floor(sx1); i<ceil(sx2); i++) { //horizontal source pixels
          dx = 1.0;
          if (sx1 > i) dx = dx-(sx1-i);
          if (sx2 < (i+1)) dx = dx-(i+1-sx2);
          ofs = (i+j*srcwidth)*4;
          sB = srcBmp[ofs];
          sG = srcBmp[ofs+1];
          sR = srcBmp[ofs+2];
          sA = srcBmp[ofs+3];
          PC = dx*dy*fi2;
          destR = destR + sR*PC;
          destG = destG + sG*PC;
          destB = destB + sB*PC;
          destA = destA + sA*PC;
        }
      }
      ofs = (x+y*destwidth)*4;
      *(destBmp+ofs) = (uint8_t)trunc(destB);
      *(destBmp+ofs+1) = (uint8_t)trunc(destG);
      *(destBmp+ofs+2) = (uint8_t)trunc(destR);
      *(destBmp+ofs+3) = (uint8_t)trunc(destA);
    }
  }

  if (srcwidth > destwidth && center) {
    dest2Bmp = (uint8_t*) malloc(sizeof(lv_color32_t)*srcwidth*srcheight);
    // shift my pic to new start point
    dx = (srcwidth-destwidth)/2;
    dy = (srcheight-destheight)/2;
    ofs = (dx+dy*srcwidth)*4;
    for (int i=0; i<srcwidth*srcheight*4; i++) dest2Bmp[i] = 0x00;// fill transp point
    for (int i=0; i<destwidth*destheight; i++, ofs+=4) {
      dest2Bmp[ofs] = destBmp[i*4];
      dest2Bmp[ofs+1] = destBmp[i*4+1];
      dest2Bmp[ofs+2] = destBmp[i*4+2];
      dest2Bmp[ofs+3] = destBmp[i*4+3];
      if (i!=0 && (i%destwidth) == (destwidth-1)) {
        ofs = ofs + (srcwidth-destwidth)*4;
      }
    }
    free(destBmp);
    *outBmp = dest2Bmp;
    return;
  }
  
  *outBmp = destBmp;
}

static void BMzoom16(uint8_t* srcBmp, int srcwidth, int srcheight, double percent, bool center, uint8_t** outBmp) {
  double factor;
  double fi, fi2;
  double sx1, sx2, sy1, sy2, dx, dy;
  double destR, destG, destB, sR, sG, sB, PC;
  int destwidth, destheight;
  int ofs;
  uint8_t* destBmp;
  uint8_t* dest2Bmp;

  destwidth = round(((double)srcwidth) * percent);
  factor = (double)srcwidth / (double)destwidth;
  fi = 1.0/factor;
  fi2 = fi*fi;
  destheight = trunc((double)srcheight/factor);
  destBmp = (uint8_t*) malloc(sizeof(lv_color16_t)*destwidth*destheight);
  
  for (int y=0; y<destheight; y++) {         //vertical destination pixels
    sy1 = factor * y;
    sy2 = sy1 + factor;
    for (int x=0; x<destwidth; x++) {        //horizontal destination pixels
      sx1 = factor * x;
      sx2 = sx1 + factor;
      destR=0.0; destG=0.0; destB=0.0;
      for (int j=floor(sy1); j<ceil(sy2); j++) {  //vertical source pixels
        dy = 1.0;
        if (sy1 > j) dy = dy-(sy1-j);
        if (sy2 < (j+1)) dy = dy-(j+1-sy2);
        for (int i=floor(sx1); i<ceil(sx2); i++) { //horizontal source pixels
          dx = 1.0;
          if (sx1 > i) dx = dx-(sx1-i);
          if (sx2 < (i+1)) dx = dx-(i+1-sx2);
          ofs = (i+j*srcwidth)*2;
          sR = srcBmp[ofs] & 0x1f;
          sG = ((srcBmp[ofs+1] & 0x07) << 3) | ((srcBmp[ofs] & 0xE0) >> 5) ;
          sB = (srcBmp[ofs+1] & 0xf8) >> 3;
          PC = dx*dy*fi2;
          destR = destR + sR*PC;
          destG = destG + sG*PC;
          destB = destB + sB*PC;
        }
      }
      ofs = (x+y*destwidth)*2;
      *(destBmp+ofs) = ((uint8_t)trunc(destR) & 0x1f) | (((uint8_t)trunc(destG) & 0x07) << 5);
      *(destBmp+ofs+1) = (((uint8_t)trunc(destG) & 0x38) >> 3) | (((uint8_t)trunc(destB) & 0x1f) << 3);
    }
  }

  if (srcwidth > destwidth && center) {
    dest2Bmp = (uint8_t*) malloc((sizeof(lv_color16_t)+1)*srcwidth*srcheight);
    // shift my pic to new start point
    dx = (srcwidth-destwidth)/2;
    dy = (srcheight-destheight)/2;
    ofs = (dx+dy*srcwidth)*3;
    for (int i=0; i<srcwidth*srcheight*3; i++) dest2Bmp[i] = 0x00;// fill transp point
    for (int i=0; i<destwidth*destheight; i++, ofs+=3) {
      dest2Bmp[ofs] = destBmp[i*2];
      dest2Bmp[ofs+1] = destBmp[i*2+1];
      dest2Bmp[ofs+2] = 0xFF;
      if (i!=0 && (i%destwidth) == (destwidth-1)) {
        ofs = ofs + (srcwidth-destwidth)*3;
      }
    }
    free(destBmp);
    *outBmp = dest2Bmp;
    return;
  }
  
  *outBmp = destBmp;
}

static void BMzoom32(uint8_t* srcBmp, int srcwidth, int srcheight, double percent, bool center, uint8_t** outBmp) {
  double factor;
  double fi, fi2;
  double sx1, sx2, sy1, sy2, dx, dy;
  double destR, destG, destB, sR, sG, sB, PC;
  int destwidth, destheight;
  int ofs;
  uint8_t* destBmp;
  uint8_t* dest2Bmp;

  destwidth = round(((double)srcwidth) * percent);
  factor = (double)srcwidth / (double)destwidth;
  fi = 1.0/factor;
  fi2 = fi*fi;
  destheight = trunc((double)srcheight/factor);
  destBmp = (uint8_t*) malloc(sizeof(lv_color32_t)*destwidth*destheight);
  if (!destBmp) {*outBmp = NULL; return;}
  
  for (int y=0; y<destheight; y++) {         //vertical destination pixels
    sy1 = factor * y;
    sy2 = sy1 + factor;
    for (int x=0; x<destwidth; x++) {        //horizontal destination pixels
      sx1 = factor * x;
      sx2 = sx1 + factor;
      destR=0.0; destG=0.0; destB=0.0;
      for (int j=floor(sy1); j<ceil(sy2); j++) {  //vertical source pixels
        dy = 1.0;
        if (sy1 > j) dy = dy-(sy1-j);
        if (sy2 < (j+1)) dy = dy-(j+1-sy2);
        for (int i=floor(sx1); i<ceil(sx2); i++) { //horizontal source pixels
          dx = 1.0;
          if (sx1 > i) dx = dx-(sx1-i);
          if (sx2 < (i+1)) dx = dx-(i+1-sx2);
          ofs = (i+j*srcwidth)*3;
          sB = srcBmp[ofs];
          sG = srcBmp[ofs+1];
          sR = srcBmp[ofs+2];
          PC = dx*dy*fi2;
          destR = destR + sR*PC;
          destG = destG + sG*PC;
          destB = destB + sB*PC;
        }
      }
      ofs = (x+y*destwidth)*3;
      *(destBmp+ofs) = (uint8_t)trunc(destB);
      *(destBmp+ofs+1) = (uint8_t)trunc(destG);
      *(destBmp+ofs+2) = (uint8_t)trunc(destR);
    }
  }

  if (srcwidth > destwidth && center) {
    dest2Bmp = (uint8_t*) malloc(sizeof(lv_color32_t)*srcwidth*srcheight);
    if (!dest2Bmp) {*outBmp = NULL; return;}
    // shift my pic to new start point
    dx = (srcwidth-destwidth)/2;
    dy = (srcheight-destheight)/2;
    ofs = (dx+dy*srcwidth)*4;
    for (int i=0; i<srcwidth*srcheight*4; i++) dest2Bmp[i] = 0x00;// fill transp point
    for (int i=0; i<destwidth*destheight; i++, ofs+=4) {
      dest2Bmp[ofs] = destBmp[i*3];
      dest2Bmp[ofs+1] = destBmp[i*3+1];
      dest2Bmp[ofs+2] = destBmp[i*3+2];
      dest2Bmp[ofs+3] = 0xFF;
      if (i!=0 && (i%destwidth) == (destwidth-1)) {
        ofs = ofs + (srcwidth-destwidth)*4;
      }
    }
    free(destBmp);
    *outBmp = dest2Bmp;
    return;
  }
  
  *outBmp = destBmp;
}

void clearZoomPicture(lv_img_dsc_t* p) {
    if (p != NULL) {
        free((void*)p->data);
        //free((void*)p);
    }
}

lv_img_dsc_t* getZoomPicture(const void* d, double zoom, int bits, bool center, lv_img_dsc_t* prevPic) {
    lv_img_decoder_dsc_t p;
    uint8_t* nbmp = NULL;
    uint8_t* n1bmp = NULL;
    uint8_t* n2bmp = NULL;
    int destwidth, destheight;
    double factor;
    int i, width, height, cf, pixel;
    
    if (bits != 16 && bits != 32) return NULL;
    if (d == NULL) return NULL;
    #if defined (AUTOFREE_PREV_BITMAP)
    clearZoomPicture(prevPic);
    #endif
    
    zoom = ((int)(zoom*100.0))/100.0;
    lv_img_dsc_t* newpic = (lv_img_dsc_t*) malloc(sizeof(lv_img_dsc_t));
    
    if (lv_img_decoder_open(&p, d, LV_COLOR_BLACK) == LV_RES_INV) return NULL;
    
    if (p.src_type == LV_IMG_SRC_FILE) {
        const char * fn = (const char *) p.src;
        if (!strcmp(&fn[strlen(fn) - 4], ".png")) {
            if (bits == 16) BMzoom16_ALPHA((uint8_t*)p.img_data, p.header.w, p.header.h, zoom, center, &nbmp);
            else BMzoom32_ALPHA((uint8_t*)p.img_data, p.header.w, p.header.h, zoom, center, &nbmp);
            if (!nbmp) return NULL;
            cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
        } else if (!strcmp(&fn[strlen(fn) - 4], ".jpg") || !strcmp(&fn[strlen(fn) - 4], ".bmp")) {
            // we read this bitmap by using lv_img_decoder_read_line()
            width = p.header.w;
            height = p.header.h;
            pixel = (bits == 16) ? 2 : 4;
            n1bmp = (uint8_t*)malloc(width * pixel); if (!n1bmp) return NULL;
            n2bmp = (uint8_t*)malloc(width * height * pixel); if (!n2bmp) return NULL;
            i=0;
            for (int y=0; y<height; y++) {
              lv_img_decoder_read_line(&p, 0, y, width, n1bmp);
              for (int x=0; x<width*pixel; x++)
                n2bmp[i++] = n1bmp[x];
            }
            free(n1bmp);
            // start to zoom
            if (bits == 16) BMzoom16(n2bmp, p.header.w, p.header.h, zoom, center, &nbmp);
            else BMzoom32_ALPHA(n2bmp, p.header.w, p.header.h, zoom, center, &nbmp);
            if (zoom < 1.0 && center)
                cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
            else
                cf = LV_IMG_CF_TRUE_COLOR;
            free(n2bmp);
            if (!nbmp) return NULL;
        }
    } else if (p.src_type == LV_IMG_SRC_VARIABLE) {
        if (p.header.cf == LV_IMG_CF_TRUE_COLOR_ALPHA) {
            if (bits == 16) BMzoom16_ALPHA((uint8_t*)p.img_data, p.header.w, p.header.h, zoom, center, &nbmp);
            else BMzoom32_ALPHA((uint8_t*)p.img_data, p.header.w, p.header.h, zoom, center, &nbmp);
            if (!nbmp) return NULL;
            cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
        } else if (p.header.cf == LV_IMG_CF_TRUE_COLOR) {
            if (bits == 16) BMzoom16((uint8_t*)p.img_data, p.header.w, p.header.h, zoom, center, &nbmp);
            else BMzoom32((uint8_t*)p.img_data, p.header.w, p.header.h, zoom, center, &nbmp);
            if (zoom < 1.0 && center)
                cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
            else
                cf = LV_IMG_CF_TRUE_COLOR;
            if (!nbmp) return NULL;
        }
    } else {
        return NULL;
    }
    
    if (zoom > 1.0 || (zoom < 1.0 && !center)) {
        destwidth = round(((double)p.header.w) * zoom); // same as BMzoom
        factor = (double)p.header.w / (double)destwidth; // same as BMzoom
        destheight = trunc(p.header.h/factor); // same as BMzoom
    } else {
        destwidth = p.header.w;
        destheight = p.header.h;
    }
    
    newpic->header.always_zero = 0;
    newpic->header.w = destwidth;
    newpic->header.h = destheight;
    newpic->data_size = lv_img_buf_get_img_size(destwidth, destheight, cf);
    newpic->header.cf = cf;
    newpic->data = nbmp;
    lv_img_decoder_close(&p);

    return newpic;
}


typedef struct _pic {
    lv_obj_t* obj;
    char fileName[512];
    double zoom;
    int angle;
    int bits;
    bool center;
    int org_x;
    int org_y;
    int org_w;
    int org_h;
    lv_img_dsc_t* prevPic;
    struct _pic* next;
    struct _pic* prev;
} PICTURE;

PICTURE* pic_head = NULL;
void addNewPictureNode(PICTURE* pic) {
    if (pic_head == NULL) {
        pic->next = NULL;
        pic->prev = NULL;
    } else {
        pic->next = pic_head;
        pic->prev = NULL;
        pic_head->prev = pic;
    }
    
    pic_head = pic;
}

void destroyPictureNode() {
    PICTURE* p;
    
    while (pic_head != NULL) pic_head = pic_head->prev;
    
    while (pic_head != NULL) {
        p = pic_head;
        pic_head = p->next;
        free(p);
    }
}

PICTURE* getDestNode(lv_obj_t* obj) {
    PICTURE* p = pic_head;
    while (p != NULL) {
        if (p->obj == obj) return p;
        p = p->next;
    }
    
    return p;
}

bool isOverScreen(lv_obj_t* obj, double newZoom, unsigned int width, unsigned int height) {
    if (newZoom > 1.0) {
        lv_disp_t * disp = lv_obj_get_disp(obj);
        if ((width * newZoom > lv_disp_get_hor_res(disp)) ||
            (height * newZoom > lv_disp_get_ver_res(disp)))
            return true;
    }
    
    return false;
}
int setImgObjZoomAngle(lv_obj_t* obj, double newZoom, double newAngle) {
    lv_img_dsc_t* zoomImg = NULL;
    if (obj == NULL) return -1;

    char* fileName = (char*)lv_img_get_file_name(obj);
    if (fileName == NULL) return -2;

    PICTURE* p = getDestNode(obj);
    if (p == NULL) {
        p = (PICTURE*)malloc(sizeof(PICTURE));
        sprintf(p->fileName, "%s", fileName);
        p->obj = obj;
        p->zoom = 1.0;
        p->angle = 0;
        p->bits = LV_COLOR_DEPTH;
        p->prevPic = NULL;
        p->center = true;
        p->org_x = lv_obj_get_x(obj);
        p->org_y = lv_obj_get_y(obj);
        p->org_w = lv_obj_get_width(obj);
        p->org_h = lv_obj_get_height(obj);
        addNewPictureNode(p);
    }

    if (strlen(fileName) != 0 && strcmp(p->fileName, fileName) != 0) { // if user change another imgae using lv_img_set_src() before.
        if (isOverScreen(obj, newZoom, lv_obj_get_width(obj), lv_obj_get_height(obj))) return -3;                                                         
        p->org_x = lv_obj_get_x(obj);
        p->org_y = lv_obj_get_y(obj);
        p->org_w = lv_obj_get_width(obj);
        p->org_h = lv_obj_get_height(obj);
        
        if (newZoom != SAMEZOOM) {
            zoomImg = getZoomPicture(fileName, newZoom, p->bits, p->center, p->prevPic);
            p->zoom = newZoom;
        } 
        else
            zoomImg = getZoomPicture(fileName, p->zoom, p->bits, p->center, p->prevPic);

        lv_img_set_src(obj, zoomImg);
        
        // If newZoom > 1.0, set new x1,y1
        if (p->center && newZoom != SAMEZOOM && newZoom > 1.0) {
            int ofs_x = (lv_obj_get_width(obj) - p->org_w) / 2;
            int ofs_y = (lv_obj_get_height(obj) - p->org_h) / 2;
            lv_obj_set_x(obj, p->org_x - ofs_x);
            lv_obj_set_y(obj, p->org_y - ofs_y);
        }
            
        if ((int)newAngle != SAMEANGLE) {
            lv_img_set_angle(obj, (int)(newAngle * 10.0));
            p->angle = (int)(newAngle * 10.0);
        }
        else
            lv_img_set_angle(obj, p->angle);

        p->prevPic = zoomImg;
        sprintf(p->fileName, "%s", fileName);
        return 0;
    }

    if (p->zoom != newZoom) {
        if (newZoom == SAMEZOOM) { // first set image
            zoomImg = getZoomPicture(p->fileName, p->zoom, p->bits, p->center, p->prevPic);
            lv_img_set_src(obj, zoomImg);
            p->prevPic = zoomImg;
            p->zoom = newZoom;
        } 
        else {
            if (isOverScreen(obj, newZoom, p->org_w, p->org_h)) return -3;
            zoomImg = getZoomPicture(p->fileName, newZoom, p->bits, p->center, p->prevPic);
            lv_img_set_src(obj, zoomImg);
            p->prevPic = zoomImg;
            p->zoom = newZoom;
            
            // If newZoom > 1.0, set new x1,y1
            if (p->center && newZoom > 1.0) {
                int ofs_x = (lv_obj_get_width(obj) - p->org_w) / 2;
                int ofs_y = (lv_obj_get_height(obj) - p->org_h) / 2;
                lv_obj_set_x(obj, p->org_x - ofs_x);
                lv_obj_set_y(obj, p->org_y - ofs_y);
            } else {
                lv_obj_set_x(obj, p->org_x);
                lv_obj_set_y(obj, p->org_y);
            }
        }
    }

    if ((int)newAngle != SAMEANGLE && p->angle != (int)(newAngle * 10.0)) {
        lv_img_set_angle(obj, (int)(newAngle * 10.0));
        p->angle = (int)(newAngle * 10.0);
    }
    return 0;         
}

int setImgObjZoom(lv_obj_t* obj, double newZoom) {
    return setImgObjZoomAngle(obj, newZoom, SAMEANGLE);
}

int setImgObjAngle(lv_obj_t* obj, double newAngle) {
    return setImgObjZoomAngle(obj, SAMEZOOM, newAngle);
}
