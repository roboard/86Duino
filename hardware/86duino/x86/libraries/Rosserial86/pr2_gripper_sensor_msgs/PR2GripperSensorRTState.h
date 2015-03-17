#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperSensorRTState_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperSensorRTState_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperSensorRTState : public ros::Msg
  {
    public:
      int8_t realtime_controller_state;
      enum { DISABLED =  0 };
      enum { POSITION_SERVO =  3 };
      enum { FORCE_SERVO =  4 };
      enum { FIND_CONTACT =  5 };
      enum { SLIP_SERVO =  6 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int8_t real;
        uint8_t base;
      } u_realtime_controller_state;
      u_realtime_controller_state.real = this->realtime_controller_state;
      *(outbuffer + offset + 0) = (u_realtime_controller_state.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->realtime_controller_state);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int8_t real;
        uint8_t base;
      } u_realtime_controller_state;
      u_realtime_controller_state.base = 0;
      u_realtime_controller_state.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->realtime_controller_state = u_realtime_controller_state.real;
      offset += sizeof(this->realtime_controller_state);
     return offset;
    }

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperSensorRTState"; };
    const char * getMD5(){ return "8109436c1f7237c52c00d885ed5755d7"; };

  };

}
#endif