#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperGrabCommand_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperGrabCommand_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperGrabCommand : public ros::Msg
  {
    public:
      float hardness_gain;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      int32_t * val_hardness_gain = (int32_t *) &(this->hardness_gain);
      int32_t exp_hardness_gain = (((*val_hardness_gain)>>23)&255);
      if(exp_hardness_gain != 0)
        exp_hardness_gain += 1023-127;
      int32_t sig_hardness_gain = *val_hardness_gain;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_hardness_gain<<5) & 0xff;
      *(outbuffer + offset++) = (sig_hardness_gain>>3) & 0xff;
      *(outbuffer + offset++) = (sig_hardness_gain>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_hardness_gain<<4) & 0xF0) | ((sig_hardness_gain>>19)&0x0F);
      *(outbuffer + offset++) = (exp_hardness_gain>>4) & 0x7F;
      if(this->hardness_gain < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t * val_hardness_gain = (uint32_t*) &(this->hardness_gain);
      offset += 3;
      *val_hardness_gain = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_hardness_gain |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_hardness_gain |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_hardness_gain |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_hardness_gain = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_hardness_gain |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_hardness_gain !=0)
        *val_hardness_gain |= ((exp_hardness_gain)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->hardness_gain = -this->hardness_gain;
     return offset;
    }

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperGrabCommand"; };
    const char * getMD5(){ return "cf286b093615060c79527896d36bf694"; };

  };

}
#endif