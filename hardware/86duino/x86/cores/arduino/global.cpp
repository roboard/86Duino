#include "Arduino.h"

bool _G_MCM[3];
bool _G_ADC[2];
bool _G_I2C[2];
bool _G_SPI[2];
bool _G_LAN[2];
bool _G_COM[10];
bool _G_GPIO[16];
bool _G_GPDMA;
bool _G_GPINT[2];
bool _G_USBD;
bool _G_USBH;
bool _G_SD[2];
bool _G_WDT0;
bool _G_WDT1;
bool _G_CANBUS;
bool _G_AUDIO;

void getResource() {
    void *pciDev;
    uint32_t d1 = nb_Read(0x58); // 0x58 ~ 5B: Device System Access Select Register
    uint32_t d2 = nb_Read(0x5C); // 0x5C ~ 5F: PCI Device System Access Select Register
    uint32_t d3; // Hybrid 0x40 ~ 43: System Access Select Register
    int cupid = vx86_CpuID();
    
    if ((cupid == CPU_VORTEX86EX2_MASTER) || (cupid == CPU_VORTEX86EX2_SLAVE) || (cupid == CPU_VORTEX86EX3_MASTER) || (cupid == CPU_VORTEX86EX3_SLAVE)) {
        pciDev = pci_Alloc(0x00, 0x06, 0x00);
        d3 = pci_In32(pciDev, 0x40);
        pci_Free(pciDev);
    }
    
    if ((cupid == CPU_VORTEX86EX2_MASTER) || (cupid == CPU_VORTEX86EX3_MASTER)) {
        _G_WDT0 = GET;
        if ((d1 & 0x00100000L) == 0) _G_WDT1 = GET; else _G_WDT1 = NOTHING;
        if ((d1 & 0x00040000L) == 0) _G_GPDMA = GET; else _G_GPDMA = NOTHING;
        if ((d1 & 0x00020000L) == 0) _G_GPINT[1] = GET; else _G_GPINT[1] = NOTHING;
        if ((d1 & 0x00010000L) == 0) _G_GPINT[0] = GET; else _G_GPINT[0] = NOTHING;
        for (int i = 0x00008000L, j = 15; j >= 0; j--, i>>=1)
            if ((d1 & i) == 0) _G_GPIO[j] = GET; else _G_GPIO[j] = NOTHING;
        if ((d2 & 0x00001000L) == 0) _G_CANBUS = GET; else _G_CANBUS = NOTHING;
        for (int i = 0x00000800L, j = 2; j >= 0; j--, i>>=1)
            if ((d2 & i) == 0) _G_MCM[j] = GET; else _G_MCM[j] = NOTHING;
        if ((d2 & 0x00000100L) == 0) _G_AUDIO = GET; else _G_AUDIO = NOTHING;
        if ((d2 & 0x00000080L) == 0) _G_SD[1] = GET; else _G_SD[1] = NOTHING; // SD2
        if ((d2 & 0x00000040L) == 0) _G_SD[0] = GET; else _G_SD[0] = NOTHING; // SD0/1
        if ((d2 & 0x00000020L) == 0) _G_USBD = GET; else _G_USBD = NOTHING;
        if ((d2 & 0x00000010L) == 0) _G_USBH = GET; else _G_USBH = NOTHING;
        if ((d2 & 0x00000008L) == 0) _G_LAN[1] = GET; else _G_LAN[1] = NOTHING;
        if ((d2 & 0x00000004L) == 0) _G_LAN[0] = GET; else _G_LAN[0] = NOTHING;
        if ((d3 & 0x00008000L) == 0) _G_ADC[1] = GET; else _G_ADC[1] = NOTHING;
        if ((d3 & 0x00004000L) == 0) _G_ADC[0] = GET; else _G_ADC[0] = NOTHING;
        if ((d3 & 0x00002000L) == 0) _G_I2C[1] = GET; else _G_I2C[1] = NOTHING;
        if ((d3 & 0x00001000L) == 0) _G_I2C[0] = GET; else _G_I2C[0] = NOTHING;
        if ((d3 & 0x00000800L) == 0) _G_SPI[1] = GET; else _G_SPI[1] = NOTHING;
        if ((d3 & 0x00000400L) == 0) _G_SPI[0] = GET; else _G_SPI[0] = NOTHING;
        for (int i = 0x00000200L, j = 9; j >= 0; j--, i>>=1)
            if ((d3 & i) == 0) _G_COM[j] = GET; else _G_COM[j] = NOTHING;
    } else if ((cupid == CPU_VORTEX86EX2_SLAVE) || (cupid == CPU_VORTEX86EX3_SLAVE)) {
        _G_WDT0 = NOTHING;
        if ((d1 & 0x00100000L) > 0) _G_WDT1 = GET; else _G_WDT1 = NOTHING;
        if ((d1 & 0x00040000L) > 0) _G_GPDMA = GET; else _G_GPDMA = NOTHING;
        if ((d1 & 0x00020000L) > 0) _G_GPINT[1] = GET; else _G_GPINT[1] = NOTHING;
        if ((d1 & 0x00010000L) > 0) _G_GPINT[0] = GET; else _G_GPINT[0] = NOTHING;
        for (int i = 0x00008000L, j = 15; j >= 0; j--, i>>=1)
            if ((d1 & i) > 0) _G_GPIO[j] = GET; else _G_GPIO[j] = NOTHING;
        if ((d2 & 0x00001000L) > 0) _G_CANBUS = GET; else _G_CANBUS = NOTHING;
        for (int i = 0x00000800L, j = 2; j >= 0; j--, i>>=1)
            if ((d2 & i) > 0) _G_MCM[j] = GET; else _G_MCM[j] = NOTHING;
        if ((d2 & 0x00000100L) > 0) _G_AUDIO = GET; else _G_AUDIO = NOTHING;
        if ((d2 & 0x00000080L) > 0) _G_SD[1] = GET; else _G_SD[1] = NOTHING; // SD2
        if ((d2 & 0x00000040L) > 0) _G_SD[0] = GET; else _G_SD[0] = NOTHING; // SD0/1
        if ((d2 & 0x00000020L) > 0) _G_USBD = GET; else _G_USBD = NOTHING;
        if ((d2 & 0x00000010L) > 0) _G_USBH = GET; else _G_USBH = NOTHING;
        if ((d2 & 0x00000008L) > 0) _G_LAN[1] = GET; else _G_LAN[1] = NOTHING;
        if ((d2 & 0x00000004L) > 0) _G_LAN[0] = GET; else _G_LAN[0] = NOTHING;
        if ((d3 & 0x00008000L) > 0) _G_ADC[1] = GET; else _G_ADC[1] = NOTHING;
        if ((d3 & 0x00004000L) > 0) _G_ADC[0] = GET; else _G_ADC[0] = NOTHING;
        if ((d3 & 0x00002000L) > 0) _G_I2C[1] = GET; else _G_I2C[1] = NOTHING;
        if ((d3 & 0x00001000L) > 0) _G_I2C[0] = GET; else _G_I2C[0] = NOTHING;
        if ((d3 & 0x00000800L) > 0) _G_SPI[1] = GET; else _G_SPI[1] = NOTHING;
        if ((d3 & 0x00000400L) > 0) _G_SPI[0] = GET; else _G_SPI[0] = NOTHING;
        for (int i = 0x00000200L, j = 9; j >= 0; j--, i>>=1)
            if ((d3 & i) > 0) _G_COM[j] = GET; else _G_COM[j] = NOTHING;
    } else if (cupid == CPU_VORTEX86EX) { // or Board
        _G_MCM[0] = GET; _G_MCM[1] = NOTHING; _G_MCM[2] = NOTHING;
        _G_ADC[0] = GET; _G_ADC[1] = NOTHING;
        _G_I2C[0] = GET; _G_I2C[1] = NOTHING;
        _G_SPI[0] = GET; _G_SPI[1] = NOTHING;
        _G_LAN[0] = GET; _G_LAN[1] = NOTHING;
        for (int i=0; i<10; i++) _G_COM[i] = GET;
        for (int i=0; i<10; i++) _G_GPIO[i] = GET;
        for (int i=10; i<16; i++)_G_GPIO[i] = NOTHING;
        _G_USBD    = GET;
        _G_USBH    = GET;
        _G_SD[0]   = GET; _G_SD[1] = NOTHING;
        _G_WDT0    = GET;
        _G_WDT1    = GET;
        _G_CANBUS  = GET;
        _G_AUDIO   = GET;
    } // TODO: DX2, DX3
}