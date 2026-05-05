#include <Arduino.h>
#include "_ADS1X15.h"
#include "DetectQECCurrent.h"

#if defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9) || defined (__86DUINO_PPC9S) || defined (__86DUINO_PPC15S)

I_t isRaw[8] = {{0.0, 498}, {0.5, 524}, {1.0, 550}, {1.5, 576}, {2.0, 603}, {2.5, 630}, {3.0, 656}, {3.5, 681}};
I_t ipRaw[8] = {{0.0, 500}, {0.5, 527}, {1.0, 548}, {1.5, 578}, {2.0, 603}, {2.5, 630}, {3.0, 656}, {3.5, 680}};
// I_t isRaw[8] = {{0.0, 12595}, {0.5, 13205}, {1.0, 13815}, {1.5, 14425}, {2.0, 15032}, {2.5, 15633}, {3.0, 16227}, {3.5, 16808}};  // 12 bits
// I_t ipRaw[8] = {{0.0, 12828}, {0.5, 13458}, {1.0, 14086}, {1.5, 14715}, {2.0, 15343}, {2.5, 15950}, {3.0, 16555}, {3.5, 17150}}; // 12 bits

#if defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9) || defined (__86DUINO_PPC9S) || defined (__86DUINO_PPC15S)
_ADS1115 ads;
bool adsInited = false;
void detectIsIp_Init(void) {
    ads.setGain(GAIN_ONE);
    if (!ads.begin()) {
        printf("Failed to initialize ADS.");
        return;
    }
    
    ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, false);
    adsInited = true;
}

void detectIsIp(uint16_t* Ip, uint16_t* Is) {
    static int state = 0;
    static uint16_t prev_Ip = 0, prev_Is = 0;
    int16_t results, adc0, adc1;

    if (!adsInited) return;
    
    // Blocking
    /*
    adc0 = ads.readADC_SingleEnded(0);
    adc1 = ads.readADC_SingleEnded(1);
    *Ip = prev_Ip = adc0;
    *Is = prev_Is = adc1;
    */
    
    // Non-blocking
    switch (state) {
    case 0:
        if (ads.conversionComplete()) {
            results = ads.getLastConversionResults();
            *Ip = prev_Ip = results;
            *Is = prev_Is;
            ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_1, false);
            //printf("state: %d Ip:%d Is:%d\n", state, *Ip, *Is);
            state = 1;
        } else {
            *Ip = prev_Ip;
            *Is = prev_Is;
            //printf("state: %d Ip is prepare\n", state);
        }
    break;
    case 1:
        if (ads.conversionComplete()) {
            results = ads.getLastConversionResults();
            *Ip = prev_Ip;
            *Is = prev_Is = results;
            ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, false);
            //printf("state: %d Ip:%d Is:%d\n", state, *Ip, *Is);
            state = 0;
        } else {
            *Ip = prev_Ip;
            *Is = prev_Is;
            //printf("state: %d Is is prepare\n", state);
        }
    break;
    }
}
#endif

double calculateCurrent(I_t * iRaw, int detectValue) {
    double result;
    int i;
    
    int len = sizeof(isRaw)/sizeof(I_t);
    for (i=0; i<len; i++) {
        if (detectValue > iRaw[i].rawdata) continue;
        if ((i - 1) < 0) return iRaw[0].current;
        result = iRaw[i-1].current + (((detectValue - iRaw[i-1].rawdata) * 0.5) / (iRaw[i].rawdata - iRaw[i-1].rawdata));
        break;
    }
    
    if (i == len) return 0.0;
    return result;
}


double _getIp(void) {
    #if defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_M02)
    double ip = calculateCurrent(ipRaw, analogRead(A0));
    return ip;
    #else
    uint16_t ip = 0, is = 0;
    detectIsIp(&ip, &is);
    //printf("Ip:%d Is:%d\n", ip, is);
    if (ip > 0) return calculateCurrent(ipRaw, ip);
    return 0.0;
    #endif
}

double _getIs(void) {
    #if defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_M02)
    double is = calculateCurrent(isRaw, analogRead(A1));
    return is;
    #else
    uint16_t ip = 0, is = 0;
    detectIsIp(&ip, &is);
    // Serial.print("Ip: ");
    // Serial.print(ip);
    // Serial.print("  Is: ");
    // Serial.println(is);
    if (is > 0) return calculateCurrent(isRaw, is);
    return 0.0;
    #endif
}

void parseI(FILE* fp, I_t* i) {
    int len = sizeof(isRaw)/sizeof(I_t);

    for (int j=0; j<len; j++) {
        fscanf(fp, "%f", &i[j].current);
        fscanf(fp, "%f", &i[j].rawdata);
    }
}

#endif