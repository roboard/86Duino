#include "io.h"
#include "irq.h"

static int _irq_usingFPU[16] = {0};
static int _prev_irq_setting[16] = {0};

bool irq_Setting_RequestFPU(int irq) {
    switch (irq) {
    case 3:case 4:case 5: case 6:case 7:
        if (_irq_usingFPU[irq] == 0)
        {
            _prev_irq_setting[irq] = irq_GetSetting(irq);
            if(irq_Setting(irq, _prev_irq_setting[irq] | IRQ_USE_FPU) == false)
            {
                printf("IRQ %d request FPU is fail\n", irq);
                _prev_irq_setting[irq] = 0;
                return false;
            }
        }
        _irq_usingFPU[irq]++;
        break;
    default:
        return false;
    }
    
    return true;
}

void irq_Setting_QuitFPU(int irq) {
    switch (irq) {
    case 3:case 4:case 5: case 6:case 7:
        if (_irq_usingFPU[irq] == 1)
        {
            if(irq_Setting(irq, _prev_irq_setting[irq]) == false)
            {
                printf("IRQ %d quit FPU is fail\n", irq); return;
            }
            _prev_irq_setting[irq] = 0;
        }
        if (_irq_usingFPU[irq] > 0)
            _irq_usingFPU[irq]--;
        break;
    default:
        break;
    }
}