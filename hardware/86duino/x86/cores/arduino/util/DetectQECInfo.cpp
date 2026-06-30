#include <Arduino.h>
#include "DetectQECCurrent.h"
#include "DetectQECVoltage.h"
#include "_EncoderEX.h"


static double vs = 0.0, is = 0.0, vp = 0.0, ip = 0.0;
static unsigned long intervalTime = 500;
double systemPowerVoltage() {
    return vs;
}

double systemPowerCurrent() {
    return is;
}

double peripheralPowerVoltage() {
    return vp;
}

double peripheralPowerCurrent() {
    return ip;
}

void setIntervalTimeForPowerDetected(unsigned long t) {
    intervalTime = t;
}

#if defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9) || defined (__86DUINO_PPC9S) || defined (__86DUINO_PPC15S)

static _HelperTask* _taskRTC = NULL;

// Red head exchange
// V_t vsRaw[12] = {{6, 1086}, {10, 718}, {15, 488}, {20, 370}, {25, 299}, {30, 250}, {35, 215}, {40, 188}, {45, 168}, {50, 151}, {55, 138}, {60, 127}};
// V_t vpRaw[12] = {{6, 1122}, {10, 716}, {15, 488}, {20, 370}, {25, 298}, {30, 250}, {35, 215}, {40, 188}, {45, 168}, {50, 151}, {55, 138}, {60, 127}};
// I_t isRaw[8] = {{0.0, 501}, {0.5, 527}, {1.0, 552}, {1.5, 577}, {2.0, 604}, {2.5, 630}, {3.0, 654}, {3.5, 680}};
// I_t ipRaw[8] = {{0.0, 504}, {0.5, 530}, {1.0, 556}, {1.5, 581}, {2.0, 607}, {2.5, 633}, {3.0, 657}, {3.5, 683}};

// Red head no exchange
// V_t vsRaw[12] = {{6, 1091}, {10, 704}, {15, 481}, {20, 365}, {25, 294}, {30, 246}, {35, 212}, {40, 186}, {45, 166}, {50, 150}, {55, 136}, {60, 125}};
// V_t vpRaw[12] = {{6, 1103}, {10, 725}, {15, 492}, {20, 373}, {25, 300}, {30, 252}, {35, 217}, {40, 190}, {45, 169}, {50, 152}, {55, 139}, {60, 127}};
// I_t isRaw[8] = {{0.0, 498}, {0.5, 524}, {1.0, 550}, {1.5, 576}, {2.0, 603}, {2.5, 630}, {3.0, 656}, {3.5, 681}};
// I_t ipRaw[8] = {{0.0, 500}, {0.5, 527}, {1.0, 548}, {1.5, 578}, {2.0, 603}, {2.5, 630}, {3.0, 656}, {3.5, 680}};

// V_t vsRaw[10] = {{18, 241545}, {20, 267379}, {25, 332225}, {30, 396825}, {35, 460829}, {40, 523560}, {45, 588235}, {50, 649350}, {55, 714285}, {60, 781250}};
// V_t vpRaw[10] = {{18, 246305}, {20, 272479}, {25, 337837}, {30, 403225}, {35, 469483}, {40, 534759}, {45, 602409}, {50, 666666}, {55, 729927}, {60, 800000}};
// I_t isRaw[8] = {{0.0, 498}, {0.5, 524}, {1.0, 550}, {1.5, 576}, {2.0, 603}, {2.5, 630}, {3.0, 656}, {3.5, 681}};
// I_t ipRaw[8] = {{0.0, 500}, {0.5, 527}, {1.0, 548}, {1.5, 578}, {2.0, 603}, {2.5, 630}, {3.0, 656}, {3.5, 680}};

static void redLED_ON(void) {
    digitalWrite(POWER_LED_RED, HIGH);
}

static void redLED_OFF(void) {
    digitalWrite(POWER_LED_RED, LOW);
}

static void greenLED_ON(void) {
    digitalWrite(POWER_LED_GREEN, HIGH);
}

static void greenLED_OFF(void) {
    digitalWrite(POWER_LED_GREEN, LOW);
}

static void LedState1(void) {
    greenLED_ON();
    redLED_OFF();
}

static void LedState2(void) {
    static int state = 0;
    if (state == 0) {
        greenLED_ON(); redLED_OFF();
        state = 1;
    } else if (state == 1) {
        greenLED_OFF(); redLED_ON();
        state = 0;
    }
}

static void LedState3(void) {
    static int state = 0;
    if (state == 0) {
        greenLED_ON(); redLED_ON();
        state = 1;
    } else if (state == 1) {
        greenLED_OFF(); redLED_OFF();
        state = 0;
    }
}

int getLedState(void) {
    int state = 0;
    
    encoder_isr(readcb);
    
    vs = _getVs();
    is = _getIs();
    vp = _getVp();
    ip = _getIp();

    // First see Vs
    if ((vs < 50.0 && vs > 45.0) || (vs < 26 && vs > 19)) { // Normal
        state = 1;
        // Then see Vp
        if ((vp < 50.0 && vp > 45.0) || (vp < 26 && vp > 19)) { // Normal
            state = 1;
        } else if ((vp < 45.0 && vp > 26.0) || (vp < 19.0 && vp > 12.0)) { // Error 1
            state = 2;
        } else if (vp > 50.0 || vp < 12.0) { // Error 2
            state = 3;
        }
    } else if ((vs < 45.0 && vs > 26.0) || (vs < 19.0 && vs > 12.0)) { // Error 1
        state = 2;
    } else if (vs > 50.0 || vs < 12.0) { // Error 2
        state = 3;
    }
    
    return state;
}

extern V_t vsRaw[10];
extern V_t vpRaw[10];
extern I_t isRaw[8];
extern I_t ipRaw[8];
void reloadVpVsIpIsFromFile() {
    FILE* fp;
    fp = fopen("A:/vs.v86", "r");
    if (fp != NULL) {
        parseV(fp, vsRaw);
        fclose(fp);
    }
    
    fp = fopen("A:/is.v86", "r");
    if (fp != NULL) {
        parseI(fp, isRaw);
        fclose(fp);
    }
    
    fp = fopen("A:/vp.v86", "r");
    if (fp != NULL) {
        parseV(fp, vpRaw);
        fclose(fp);
    }
    
    fp = fopen("A:/ip.v86", "r");
    if (fp != NULL) {
        parseI(fp, ipRaw);
        fclose(fp);
    }
}

#endif

// Release Function as below.
extern unsigned long long _millis(); // in wiring.cpp
void QECPowerLedWork() {
    #if defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9) || defined (__86DUINO_PPC9S) || defined (__86DUINO_PPC15S)
    
        static int t_state = 0, t_led = 0, state = 0;
        
        if ((_millis() - t_state) > intervalTime) {
            state = getLedState();
            t_state = _millis();
        }
        
        #if defined (__86DUINO_QEC_M02)
        if ((_millis() - t_led) > 300) {
            if (state == 1)      LedState1();
            else if (state == 2) LedState2();
            else if (state == 3) LedState3();
            t_led = _millis();
        }
        #endif
        
    #endif
}

void detectQECInfoInit(void) {
    #if defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9) || defined (__86DUINO_PPC9S) || defined (__86DUINO_PPC15S)

        EncExV.begin(MODE_CAPTURE);
        EncExV.attachInterrupt(readcb); // just enable MCM, no interrupt function
        
        #if defined (__86DUINO_QEC_M02)
        pinMode(POWER_LED_RED, OUTPUT); // GPIO Port 6 DIR bit 0、1 => output
        pinMode(POWER_LED_GREEN, OUTPUT);
        #elif defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9) || defined (__86DUINO_PPC9S) || defined (__86DUINO_PPC15S)
        detectIsIp_Init(); delay(20); _getIp(); delay(20); _getIs();
        setIntervalTimeForPowerDetected(2000); // Get Vs, Vp, Is, Ip every 2 seconds.
        #endif
        
        reloadVpVsIpIsFromFile();
        QECPowerLedWork();
        
        // _taskRTC = myHelper.startLoop(100 * 1024);
        // myHelper.start(0);
        // _taskRTC->registerFunc(QECPowerLedWork);
        // _taskRTC->start();
    #elif defined (__86DUINO_QEC_M2)
    
        pinMode(POWER_LED_GREEN, OUTPUT);
        pinMode(POWER_LED_RED, OUTPUT);
        digitalWrite(POWER_LED_GREEN, HIGH);
        digitalWrite(POWER_LED_RED, LOW);
        
    #endif
}

extern bool isDigitalInit();
extern void digitalInit();
extern bool isRun();
extern void setClockShiftMs(unsigned long shift);
extern void qecm02_stop() __attribute__ ((weak));
extern void qecm02_inStop() __attribute__ ((weak));
extern void qecm02_start() __attribute__ ((weak));
void QEC_RoutineWork() {
    
    QECPowerLedWork();
    
    #if defined (__86DUINO_QEC_M02)
    if (!isDigitalInit()) digitalInit();
    if (!isRun()) {
        unsigned long long _t64 = 0LL;
        qecm02_stop();
        _t64 = timer_GetClocks64();
        while (!isRun()) qecm02_inStop();
        setClockShiftMs((timer_GetClocks64() - _t64)/VORTEX86EX_CLOCKS_PER_MS);
        qecm02_start();
    }
    #endif
    
}