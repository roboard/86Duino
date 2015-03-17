#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperForceServoCommand_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperForceServoCommand_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperForceServoCommand : public ros::Msg
  {
    public:
      float fingertip_force;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      int32_t * val_fingertip_force = (int32_t *) &(this->fingertip_force);
      int32_t exp_fingertip_force = (((*val_fingertip_force)>>23)&255);
      if(exp_fingertip_force != 0)
        exp_fingertip_force += 1023-127;
      int32_t sig_fingertip_force = *val_fingertip_force;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_fingertip_force<<5) & 0xff;
      *(outbuffer + offset++) = (sig_fingertip_force>>3) & 0xff;
      *(outbuffer + offset++) = (sig_fingertip_force>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_fingertip_force<<4) & 0xF0) | ((sig_fingertip_force>>19)&0x0F);
      *(outbuffer + offset++) = (exp_fingertip_force>>4) & 0x7F;
      if(this->fingertip_force < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t * val_fingertip_force = (uint32_t*) &(this->fingertip_force);
      offset += 3;
      *val_fingertip_force = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_fingertip_force |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_fingertip_force |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_fingertip_force |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_fingertip_force = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_fingertip_force |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_fingertip_force !=0)
        *val_fingertip_force |= ((exp_fingertip_force)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->fingertip_force = -this->fingertip_force;
     return offset;
    }

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperForceServoCommand"; };
    const char * getMD5(){ return "dd4b2a0dfafa27b67d2002841f544379"; };

  };

}
#endif