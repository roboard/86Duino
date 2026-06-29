#ifndef _ROS_H_
#define _ROS_H_

#include "ros/node_handle.h"
#include "86DuinoHardware.h"

namespace ros
{
typedef NodeHandle_<x86DuinoHardware> NodeHandle;
}

#endif
