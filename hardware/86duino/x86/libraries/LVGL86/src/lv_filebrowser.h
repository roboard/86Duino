#ifndef LV_FILEBROWSER_H
#define LV_FILEBROWSER_H


#include <stdio.h>
#include <stdlib.h>
#include "listplugin.h"

#ifndef WIN32
#include "../lv_conf.h"
#include "lvgl.h"
#include <SD.h>
#else
#include "lv_conf.h"
#include "lvgl/lvgl.h"
#endif

namespace LVGL86 {
    class FileBrowser {
    public:
        FileBrowser();
        ~FileBrowser();

        lv_obj_t* begin(lv_obj_t* parent = NULL, int _x = 0, int _y = 0, int _width = 260, int _height = 300, int8_t _color = -1, int8_t _font = -1, int _spacing = 15, int _maxdisplayfiles = 128, bool _enable = true);
        void setFilter(char* filter, int index = 0);
        void removeFilter(int index = -1);
        void setDisk(int bank);

        const char* getCurrentFolderPath(bool fullPath = false, char* buf = NULL);
        const char* getSelectedFilePath(bool fullPath = false, char* buf = NULL, int index = -1);

        void setSelectedFilePath(char* fileName);
        void setCurrentFolderPath(char* path);

        void setXY(int x, int y);
        void setSize(int w, int h);
        void setHMIColor(int colorId);
        void setHMITextFont(int fontId);
        void setEnable(bool enable);
        void setSpacing(int _spacing);

        int getX() { return X; }
        int getY() { return Y; }
        int getWidth() { return Width; }
        int getHeight() { return Height; }
        int getHMIColor() { return get_color_id(obj); }
        int getHMITextFont() { return get_font_id(obj); }
        bool getEnable() { return Enable; }

        lv_obj_t* getObj() { return obj; }

        static void _setDoslfnCodepage(int codepage);
        lv_obj_user_data_t user_data;
    private:
        int X, Y, Width, Height;
        bool Enable;

        lv_obj_t* obj;
        struct Impl;
        Impl* pimpl;
    };
    extern FileBrowser fileBrowser;
}
#endif