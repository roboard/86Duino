#ifndef _ROS_monocam_settler_ConfigGoal_h
#define _ROS_monocam_settler_ConfigGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "ros/duration.h"

namespace monocam_settler
{

  class ConfigGoal : public ros::Msg
  {
    public:
      float tolerance;
      uint8_t ignore_failures;
      ros::Duration max_step;
      uint32_t cache_size;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      int32_t * val_tolerance = (int32_t *) &(this->tolerance);
      int32_t exp_tolerance = (((*val_tolerance)>>23)&255);
      if(exp_tolerance != 0)
        exp_tolerance += 1023-127;
      int32_t sig_tolerance = *val_tolerance;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_tolerance<<5) & 0xff;
      *(outbuffer + offset++) = (sig_tolerance>>3) & 0xff;
      *(outbuffer + offset++) = (sig_tolerance>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_tolerance<<4) & 0xF0) | ((sig_tolerance>>19)&0x0F);
      *(outbuffer + offset++) = (exp_tolerance>>4) & 0x7F;
      if(this->tolerance < 0) *(outbuffer + offset -1) |= 0x80;
      *(outbuffer + offset + 0) = (this->ignore_failures >> (8 * 0)) & 0xFF;
      offset += sizeof(this->ignore_failures);
      *(outbuffer + offset + 0) = (this->max_step.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->max_step.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->max_step.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->max_step.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->max_step.sec);
      *(outbuffer + offset + 0) = (this->max_step.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->max_step.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->max_step.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->max_step.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->max_step.nsec);
      *(outbuffer + offset + 0) = (this->cache_size >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->cache_size >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->cache_size >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->cache_size >> (8 * 3)) & 0xFF;
      offset += sizeof(this->cache_size);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t * val_tolerance = (uint32_t*) &(this->tolerance);
      offset += 3;
      *val_tolerance = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_tolerance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_tolerance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_tolerance |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_tolerance = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_tolerance |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_tolerance !=0)
        *val_tolerance |= ((exp_tolerance)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->tolerance = -this->tolerance;
      this->ignore_failures =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->ignore_failures);
      this->max_step.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->max_step.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->max_step.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->max_step.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->max_step.sec);
      this->max_step.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->max_step.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->max_step.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->max_step.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->max_step.nsec);
      this->cache_size =  ((uint32_t) (*(inbuffer + offset)));
      this->cache_size |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->cache_size |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->cache_size |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->cache_size);
     return offset;
    }

    const char * getType(){ return "monocam_settler/ConfigGoal"; };
    const char * getMD5(){ return "f2b47726560448f0dada94e01ba65594"; };

  };

}
#endif