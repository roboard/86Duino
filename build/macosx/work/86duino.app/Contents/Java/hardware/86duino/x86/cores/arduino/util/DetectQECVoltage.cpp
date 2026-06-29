#include "v86board.h"
#include "_EncoderEX.h"
#include "DetectQECVoltage.h"

#if defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9)
bool isOldFileVersion = false;
unsigned long Vs_num1 = 0L;
unsigned long Vs_num2 = 0L;
unsigned long Vp_num1 = 0L;
unsigned long Vp_num2 = 0L;
V_t vsRaw[10] = {{18, 241545}, {20, 267379}, {25, 332225}, {30, 395256}, {35, 460829}, {40, 523560}, {45, 588235}, {50, 653594}, {55, 714285}, {60, 781250}};
V_t vpRaw[10] = {{18, 244798}, {20, 271002}, {25, 336700}, {30, 401606}, {35, 467289}, {40, 534759}, {45, 598802}, {50, 666666}, {55, 729927}, {60, 793650}};
V_t fakeVsRaw[4] = {{5, 71914}, {10, 137069}, {15, 202224}, {20, 267379}};
V_t fakeVpRaw[4] = {{5, 75537}, {10, 140692}, {15, 205847}, {20, 271002}};

double calculateVoltage(V_t * vRaw, int len, int detectValue) {
    double result;
    int i;
    
    for (i=0; i<len; i++) {
        if (isOldFileVersion) {
            if (detectValue < vRaw[i].rawdata) continue;
            result = vRaw[i].voltage - (((detectValue - vRaw[i].rawdata) * 5.0) / (vRaw[i-1].rawdata - vRaw[i].rawdata));
        } else { // NEW VERSION
            if (detectValue > vRaw[i].rawdata) continue;
            if ((i - 1) < 0) return vRaw[0].voltage;
            result = vRaw[i-1].voltage + (((detectValue - vRaw[i-1].rawdata) * 5.0) / (vRaw[i].rawdata - vRaw[i-1].rawdata));
        }
        break;
    }
    
    if (i == len) return 0.0;
    return result;
}

void readcb(int flag) {
    if (flag == INTR_B_PULSE_LOW) {
        Vs_num1 = EncExV.read();
    } else if(flag == INTR_B_PULSE_HIGH) {
        Vs_num2 = EncExV.read();
    } else if(flag == INTR_Z_PULSE_LOW) {
        Vp_num1 = EncExV.read();
    } else if(flag == INTR_Z_PULSE_HIGH) {
        Vp_num2 = EncExV.read();
    }
}

double _getVs(void) {
    double tempV;
    
    if (isOldFileVersion)
        return calculateVoltage(vsRaw, sizeof(vsRaw)/sizeof(V_t), Vs_num1 + Vs_num2);
    else { // NEW VERSION
        if ((Vs_num1 + Vs_num2) == 0) return 0.0;
        //printf("VS: %ld\n", 100000000L/(Vs_num1 + Vs_num2));
        tempV = calculateVoltage(vsRaw, sizeof(vsRaw)/sizeof(V_t), 100000000L/(Vs_num1 + Vs_num2)); // unit: 10ns, 100000000L/(Vs_num1 + Vs_num2) we will get count
        if (tempV <= (vsRaw[0].voltage + 0.5)) {
            tempV = calculateVoltage(fakeVsRaw, sizeof(fakeVsRaw)/sizeof(V_t), 100000000L/(Vs_num1 + Vs_num2)); // Inaccurate value
            if (tempV < 12.0) return 0.0;
            Vs_num1 = Vs_num2 = 0;
            return tempV;
        } else {
            Vs_num1 = Vs_num2 = 0;
            return tempV;
        }
    }
}

double _getVp(void) {
    double tempV;
    
    if (isOldFileVersion)
        return calculateVoltage(vpRaw, sizeof(vpRaw)/sizeof(V_t), Vp_num1 + Vp_num2);
    else { // NEW VERSION
        if ((Vp_num1 + Vp_num2) == 0) return 0.0;
        //printf("VP: %ld\n", 100000000L/(Vp_num1 + Vp_num2));
        tempV = calculateVoltage(vpRaw, sizeof(vpRaw)/sizeof(V_t), 100000000L/(Vp_num1 + Vp_num2)); // unit: 10ns, 100000000L/(Vp_num1 + Vp_num2) we will get count
        if (tempV <= (vpRaw[0].voltage + 0.5)) {
            tempV = calculateVoltage(fakeVpRaw, sizeof(fakeVpRaw)/sizeof(V_t), 100000000L/(Vp_num1 + Vp_num2)); // Inaccurate value
            if (tempV < 12.0) return 0.0;
            Vp_num1 = Vp_num2 = 0;
            return tempV;
        } else {
            Vp_num1 = Vp_num2 = 0;
            return tempV;
        }
    }
}

void parseV(FILE* fp, V_t* v) {
    int len = sizeof(vsRaw)/sizeof(V_t);

    for (int i=0; i<len; i++) {
        fscanf(fp, "%d", &v[i].voltage);
        fscanf(fp, "%d", &v[i].rawdata);
        if (i == 0) {
            if (v[i].rawdata > 3000 || v[i].rawdata == 0)
                isOldFileVersion = false; // NEW VERSION
            else 
                isOldFileVersion = true;
        }
    }
}

#endif