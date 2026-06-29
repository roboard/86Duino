#ifndef _ROS_moveit_msgs_ConstraintEvalResult_h
#define _ROS_moveit_msgs_ConstraintEvalResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace moveit_msgs
{

  class ConstraintEvalResult : public ros::Msg
  {
    public:
      bool result;
      float distance;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_result;
      u_result.real = this->result;
      *(outbuffer + offset + 0) = (u_result.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->result);
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
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_result;
      u_result.base = 0;
      u_result.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->result = u_result.real;
      offset += sizeof(this->result);
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
     return offset;
    }

    const char * getType(){ return "moveit_msgs/ConstraintEvalResult"; };
    const char * getMD5(){ return "093643083d24f6488cb5a868bd47c090"; };

  };

}
#endif