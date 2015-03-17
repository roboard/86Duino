#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperEventDetectorCommand_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperEventDetectorCommand_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperEventDetectorCommand : public ros::Msg
  {
    public:
      int8_t trigger_conditions;
      float acceleration_trigger_magnitude;
      float slip_trigger_magnitude;
      enum { FINGER_SIDE_IMPACT_OR_ACC =  0 };
      enum { SLIP_AND_ACC =  1 };
      enum { FINGER_SIDE_IMPACT_OR_SLIP_OR_ACC =  2 };
      enum { SLIP =  3 };
      enum { ACC =  4 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int8_t real;
        uint8_t base;
      } u_trigger_conditions;
      u_trigger_conditions.real = this->trigger_conditions;
      *(outbuffer + offset + 0) = (u_trigger_conditions.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->trigger_conditions);
      int32_t * val_acceleration_trigger_magnitude = (int32_t *) &(this->acceleration_trigger_magnitude);
      int32_t exp_acceleration_trigger_magnitude = (((*val_acceleration_trigger_magnitude)>>23)&255);
      if(exp_acceleration_trigger_magnitude != 0)
        exp_acceleration_trigger_magnitude += 1023-127;
      int32_t sig_acceleration_trigger_magnitude = *val_acceleration_trigger_magnitude;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_acceleration_trigger_magnitude<<5) & 0xff;
      *(outbuffer + offset++) = (sig_acceleration_trigger_magnitude>>3) & 0xff;
      *(outbuffer + offset++) = (sig_acceleration_trigger_magnitude>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_acceleration_trigger_magnitude<<4) & 0xF0) | ((sig_acceleration_trigger_magnitude>>19)&0x0F);
      *(outbuffer + offset++) = (exp_acceleration_trigger_magnitude>>4) & 0x7F;
      if(this->acceleration_trigger_magnitude < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_slip_trigger_magnitude = (int32_t *) &(this->slip_trigger_magnitude);
      int32_t exp_slip_trigger_magnitude = (((*val_slip_trigger_magnitude)>>23)&255);
      if(exp_slip_trigger_magnitude != 0)
        exp_slip_trigger_magnitude += 1023-127;
      int32_t sig_slip_trigger_magnitude = *val_slip_trigger_magnitude;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_slip_trigger_magnitude<<5) & 0xff;
      *(outbuffer + offset++) = (sig_slip_trigger_magnitude>>3) & 0xff;
      *(outbuffer + offset++) = (sig_slip_trigger_magnitude>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_slip_trigger_magnitude<<4) & 0xF0) | ((sig_slip_trigger_magnitude>>19)&0x0F);
      *(outbuffer + offset++) = (exp_slip_trigger_magnitude>>4) & 0x7F;
      if(this->slip_trigger_magnitude < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int8_t real;
        uint8_t base;
      } u_trigger_conditions;
      u_trigger_conditions.base = 0;
      u_trigger_conditions.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->trigger_conditions = u_trigger_conditions.real;
      offset += sizeof(this->trigger_conditions);
      uint32_t * val_acceleration_trigger_magnitude = (uint32_t*) &(this->acceleration_trigger_magnitude);
      offset += 3;
      *val_acceleration_trigger_magnitude = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_acceleration_trigger_magnitude |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_acceleration_trigger_magnitude |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_acceleration_trigger_magnitude |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_acceleration_trigger_magnitude = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_acceleration_trigger_magnitude |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_acceleration_trigger_magnitude !=0)
        *val_acceleration_trigger_magnitude |= ((exp_acceleration_trigger_magnitude)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->acceleration_trigger_magnitude = -this->acceleration_trigger_magnitude;
      uint32_t * val_slip_trigger_magnitude = (uint32_t*) &(this->slip_trigger_magnitude);
      offset += 3;
      *val_slip_trigger_magnitude = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_slip_trigger_magnitude |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_slip_trigger_magnitude |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_slip_trigger_magnitude |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_slip_trigger_magnitude = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_slip_trigger_magnitude |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_slip_trigger_magnitude !=0)
        *val_slip_trigger_magnitude |= ((exp_slip_trigger_magnitude)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->slip_trigger_magnitude = -this->slip_trigger_magnitude;
     return offset;
    }

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperEventDetectorCommand"; };
    const char * getMD5(){ return "b91a7e1e863671a84c1d06e0cac3146e"; };

  };

}
#endif