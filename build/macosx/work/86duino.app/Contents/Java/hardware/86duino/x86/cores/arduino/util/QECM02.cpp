#include <string.h>
#include <dirent.h>
#include <iostream>
using namespace std;

#include "io.h"
#include "v86board.h"
#include "QECM02.h"

extern char* userInternalResourcePath;
static unsigned char getGpioSlot() {
    unsigned char n = io_inpb(0x7C); // GPIO Port 4
    return (~n) & 0x0F;
}

void changeQECM02envPath() {
    #if defined (__86DUINO_QEC_M02)
    int slotNumber = getGpioSlot();
    // printf("slotNumber=%d\n", slotNumber); getchar();
    if (slotNumber > 0) {
        DIR *myDir;
        char sn[5];
        char *newUIRSPATH;
        int mystrlen;
        string originalPath = string(userInternalResourcePath);
        string slotString = "slot";
        string toInsert = "";
        string numberSlot = "";
        toInsert = slotString + string(itoa(slotNumber, sn, 16)) + "\\";
        originalPath.insert(3, toInsert);
        // printf("Slot Path=%s\n", originalPath.c_str()); getchar();
        mystrlen = strlen(originalPath.c_str());
        newUIRSPATH = (char*)malloc(mystrlen + 1);
        memset(newUIRSPATH, 0, mystrlen + 1);
        strcpy(newUIRSPATH, originalPath.c_str());
        
        userInternalResourcePath = newUIRSPATH; // for this bootlaoder
        setenv("UIRSPATH", newUIRSPATH, 1); // for user program, ex. 86HMI
    }
    #endif
}

void qecm02_stop(void) {}
void qecm02_stop(void) __attribute__ ((weak));
void qecm02_inStop(void) {}
void qecm02_inStop(void) __attribute__ ((weak));
void qecm02_start(void) {}
void qecm02_start(void) __attribute__ ((weak));
