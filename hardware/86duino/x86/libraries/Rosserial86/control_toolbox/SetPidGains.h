#ifndef _ROS_SERVICE_SetPidGains_h
#define _ROS_SERVICE_SetPidGains_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace control_toolbox
{

static const char SETPIDGAINS[] = "control_toolbox/SetPidGains";

  class SetPidGainsRequest : public ros::Msg
  {
    public:
      float p;
      float i;
      float d;
      float i_clamp;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      int32_t * val_p = (int32_t *) &(this->p);
      int32_t exp_p = (((*val_p)>>23)&255);
      if(exp_p != 0)
        exp_p += 1023-127;
      int32_t sig_p = *val_p;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_p<<5) & 0xff;
      *(outbuffer + offset++) = (sig_p>>3) & 0xff;
      *(outbuffer + offset++) = (sig_p>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_p<<4) & 0xF0) | ((sig_p>>19)&0x0F);
      *(outbuffer + offset++) = (exp_p>>4) & 0x7F;
      if(this->p < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_i = (int32_t *) &(this->i);
      int32_t exp_i = (((*val_i)>>23)&255);
      if(exp_i != 0)
        exp_i += 1023-127;
      int32_t sig_i = *val_i;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_i<<5) & 0xff;
      *(outbuffer + offset++) = (sig_i>>3) & 0xff;
      *(outbuffer + offset++) = (sig_i>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_i<<4) & 0xF0) | ((sig_i>>19)&0x0F);
      *(outbuffer + offset++) = (exp_i>>4) & 0x7F;
      if(this->i < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_d = (int32_t *) &(this->d);
      int32_t exp_d = (((*val_d)>>23)&255);
      if(exp_d != 0)
        exp_d += 1023-127;
      int32_t sig_d = *val_d;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_d<<5) & 0xff;
      *(outbuffer + offset++) = (sig_d>>3) & 0xff;
      *(outbuffer + offset++) = (sig_d>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_d<<4) & 0xF0) | ((sig_d>>19)&0x0F);
      *(outbuffer + offset++) = (exp_d>>4) & 0x7F;
      if(this->d < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_i_clamp = (int32_t *) &(this->i_clamp);
      int32_t exp_i_clamp = (((*val_i_clamp)>>23)&255);
      if(exp_i_clamp != 0)
        exp_i_clamp += 1023-127;
      int32_t sig_i_clamp = *val_i_clamp;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_i_clamp<<5) & 0xff;
      *(outbuffer + offset++) = (sig_i_clamp>>3) & 0xff;
      *(outbuffer + offset++) = (sig_i_clamp>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_i_clamp<<4) & 0xF0) | ((sig_i_clamp>>19)&0x0F);
      *(outbuffer + offset++) = (exp_i_clamp>>4) & 0x7F;
      if(this->i_clamp < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t * val_p = (uint32_t*) &(this->p);
      offset += 3;
      *val_p = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_p |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_p |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_p |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_p = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_p |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_p !=0)
        *val_p |= ((exp_p)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->p = -this->p;
      uint32_t * val_i = (uint32_t*) &(this->i);
      offset += 3;
      *val_i = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_i |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_i |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_i |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_i = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_i |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_i !=0)
        *val_i |= ((exp_i)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->i = -this->i;
      uint32_t * val_d = (uint32_t*) &(this->d);
      offset += 3;
      *val_d = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_d |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_d |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_d |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_d = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_d |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_d !=0)
        *val_d |= ((exp_d)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->d = -this->d;
      uint32_t * val_i_clamp = (uint32_t*) &(this->i_clamp);
      offset += 3;
      *val_i_clamp = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_i_clamp |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_i_clamp |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_i_clamp |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_i_clamp = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_i_clamp |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_i_clamp !=0)
        *val_i_clamp |= ((exp_i_clamp)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->i_clamp = -this->i_clamp;
     return offset;
    }

    const char * getType(){ return SETPIDGAINS; };
    const char * getMD5(){ return "b06494a6fc3d5b972ded4e2a9a71535a"; };

  };

  class SetPidGainsResponse : public ros::Msg
  {
    public:

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
     return offset;
    }

    const char * getType(){ return SETPIDGAINS; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class SetPidGains {
    public:
    typedef SetPidGainsRequest Request;
    typedef SetPidGainsResponse Response;
  };

}
#endif
