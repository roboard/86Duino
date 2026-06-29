// libRbdl/src/rbdl/rbdl_config.h
#ifndef RBDL_CONFIG_H
#define RBDL_CONFIG_H

// 版本：0xAABBCC = AA.major, BB.minor, CC.patch
// 例如 RBDL 3.3.1 -> 0x030301
#define RBDL_API_VERSION 0x030301  // 請依你的來源版本更新（見 RBDL release tag）

// 構建資訊（無 CMake 時給預設字串即可）
#define RBDL_BUILD_COMMIT  "unknown"
#define RBDL_BUILD_BRANCH  "unknown"
#define RBDL_BUILD_TYPE    "Release"

// Arduino/靜態連結環境不需要 dllimport/export
#ifndef RBDL_DLLAPI
  #define RBDL_DLLAPI
#endif

// 若你要用 CasADi backend 才定義這個；否則預設走 Eigen
// #define RBDL_USE_CASADI_MATH

// Logging：不定義即關閉（避免嵌入式體積/耗時）
// #define RBDL_ENABLE_LOGGING

// make sure that Eigen is licensed under the MPL2
#define EIGEN_MPL2_ONLY



#endif // RBDL_CONFIG_H
