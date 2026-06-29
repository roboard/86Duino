#ifndef __GROBAL_H_
#define __GROBAL_H_

#define GET     (true)
#define NOTHING (false)

extern bool _G_MCM[3];
extern bool _G_ADC[2];
extern bool _G_I2C[2];
extern bool _G_SPI[2];
extern bool _G_LAN[2];
extern bool _G_COM[10];
extern bool _G_GPIO[16];
extern bool _G_GPDMA;
extern bool _G_GPINT[2];
extern bool _G_USBD;
extern bool _G_USBH;
extern bool _G_SD[2];
extern bool _G_WDT0;
extern bool _G_WDT1;
extern bool _G_CANBUS;
extern bool _G_AUDIO;

void getResource();

#endif // __GROBAL_H_