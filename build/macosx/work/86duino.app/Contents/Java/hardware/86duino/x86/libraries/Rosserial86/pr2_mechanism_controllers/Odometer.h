#ifndef _ROS_pr2_mechanism_controllers_Odometer_h
#define _ROS_pr2_mechanism_controllers_Odometer_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_mechanism_controllers
{

  class Odometer : public ros::Msg
  {
    public:
      float distance;
      float angle;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      int32_t * val_distance = (int32_t *) &(this->distance);
      int32_t exp_distance = (((*val_distance)>>23)&255);
      if(exp_distance != 0)
        exp_distance += 1023-127;
      int32_t sig_distance = *val_distance;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_distance<<5) & 0xff;
      *(outbuffer + offset++) = (sig_distance>>3) & 0xff;
      *(outbuffer + offset++) = (sig_distance>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_distance<<4) & 0xF0) | ((sig_distance>>19)&0x0F);
      *(outbuffer + offset++) = (exp_distance>>4) & 0x7F;
      if(this->distance < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_angle = (int32_t *) &(this->angle);
      int32_t exp_angle = (((*val_angle)>>23)&255);
      if(exp_angle != 0)
        exp_angle += 1023-127;
      int32_t sig_angle = *val_angle;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_angle<<5) & 0xff;
      *(outbuffer + offset++) = (sig_angle>>3) & 0xff;
      *(outbuffer + offset++) = (sig_angle>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_angle<<4) & 0xF0) | ((sig_angle>>19)&0x0F);
      *(outbuffer + offset++) = (exp_angle>>4) & 0x7F;
      if(this->angle < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t * val_distance = (uint32_t*) &(this->distance);
      offset += 3;
      *val_distance = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_distance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_distance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_distance |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_distance = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_distance |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_distance !=0)
        *val_distance |= ((exp_distance)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->distance = -this->distance;
      uint32_t * val_angle = (uint32_t*) &(this->angle);
      offset += 3;
      *val_angle = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_angle |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_angle |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_angle |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_angle = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_angle |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_angle !=0)
        *val_angle |= ((exp_angle)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->angle = -this->angle;
     return offset;
    }

    const char * getType(){ return "pr2_mechanism_controllers/Odometer"; };
    const char * getMD5(){ return "1f1d53743f4592ee455aa3eaf9019457"; };

  };

}
#endif