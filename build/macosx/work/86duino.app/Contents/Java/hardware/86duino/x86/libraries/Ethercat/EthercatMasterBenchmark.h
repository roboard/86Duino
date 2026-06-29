#ifndef ___WZ_ETHERCAT_MASTER_BENCHMARK_H
#define ___WZ_ETHERCAT_MASTER_BENCHMARK_H

#include "EthercatOsDefinitions.h"
#include <stdint.h>

PACKED_BEGIN_4
typedef struct PACKED_4 {

    int64_t min;
    int64_t avg;
    int64_t max;

} EthercatBenchmarkTSC;

typedef struct PACKED_4 {

    EthercatBenchmarkTSC CycleTime;
    EthercatBenchmarkTSC SendCyclicFrame;
    EthercatBenchmarkTSC ReceiveCyclicFrame;
    EthercatBenchmarkTSC ProcessAcyclicFrame;
    EthercatBenchmarkTSC UserApp;

} EthercatBenchmark;
PACKED_END

#endif
