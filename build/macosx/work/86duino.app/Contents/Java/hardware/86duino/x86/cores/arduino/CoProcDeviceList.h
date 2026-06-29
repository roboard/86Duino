#ifndef __COPROC_DEVICE_LIST_H
#define __COPROC_DEVICE_LIST_H

/* If Device has no Event ID, set to DEV_NO_RTEVT. */
#define DEV_NO_RTEVT            (0xFF)

/* DMP CoProc Device ID and Event ID. */
#define DMP_COPROC              (0x0010)
#define DMP_COPROC_RTEVT        (62)

/* DMP EtherCAT Master Device ID and Event ID. */
#define DMP_ECAT_MASTER         (0x0020)
#define DMP_ECAT_MASTER_RTEVT   (DEV_NO_RTEVT)
#define DMP_ECAT_MASTER0        (0x0020)
#define DMP_ECAT_MASTER0_RTEVT  (55)
#define DMP_ECAT_MASTER1        (0x0021)
#define DMP_ECAT_MASTER1_RTEVT  (54)

/* DMP Full Color 3D Printer Device ID and Event ID. */
#define DMP_COLOR_3DP           (0x0030)
#define DMP_COLOR_3DP_RTEVT     (53)

#endif
