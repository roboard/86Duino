#ifndef _ROS_kobuki_msgs_ControllerInfo_h
#define _ROS_kobuki_msgs_ControllerInfo_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace kobuki_msgs
{

  class ControllerInfo : public ros::Msg
  {
    public:
      uint8_t type;
      float p_gain;
      float i_gain;
      float d_gain;
      enum { DEFAULT =   0 };
      enum { USER_CONFIGURED =   1 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->type >> (8 * 0)) & 0xFF;
      offset += sizeof(this->type);
      int32_t * val_p_gain = (int32_t *) &(this->p_gain);
      int32_t exp_p_gain = (((*val_p_gain)>>23)&255);
      if(exp_p_gain != 0)
        exp_p_gain += 1023-127;
      int32_t sig_p_gain = *val_p_gain;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_p_gain<<5) & 0xff;
      *(outbuffer + offset++) = (sig_p_gain>>3) & 0xff;
      *(outbuffer + offset++) = (sig_p_gain>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_p_gain<<4) & 0xF0) | ((sig_p_gain>>19)&0x0F);
      *(outbuffer + offset++) = (exp_p_gain>>4) & 0x7F;
      if(this->p_gain < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_i_gain = (int32_t *) &(this->i_gain);
      int32_t exp_i_gain = (((*val_i_gain)>>23)&255);
      if(exp_i_gain != 0)
        exp_i_gain += 1023-127;
      int32_t sig_i_gain = *val_i_gain;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_i_gain<<5) & 0xff;
      *(outbuffer + offset++) = (sig_i_gain>>3) & 0xff;
      *(outbuffer + offset++) = (sig_i_gain>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_i_gain<<4) & 0xF0) | ((sig_i_gain>>19)&0x0F);
      *(outbuffer + offset++) = (exp_i_gain>>4) & 0x7F;
      if(this->i_gain < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_d_gain = (int32_t *) &(this->d_gain);
      int32_t exp_d_gain = (((*val_d_gain)>>23)&255);
      if(exp_d_gain != 0)
        exp_d_gain += 1023-127;
      int32_t sig_d_gain = *val_d_gain;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_d_gain<<5) & 0xff;
      *(outbuffer + offset++) = (sig_d_gain>>3) & 0xff;
      *(outbuffer + offset++) = (sig_d_gain>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_d_gain<<4) & 0xF0) | ((sig_d_gain>>19)&0x0F);
      *(outbuffer + offset++) = (exp_d_gain>>4) & 0x7F;
      if(this->d_gain < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      this->type =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->type);
      uint32_t * val_p_gain = (uint32_t*) &(this->p_gain);
      offset += 3;
      *val_p_gain = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_p_gain |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_p_gain |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_p_gain |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_p_gain = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_p_gain |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_p_gain !=0)
        *val_p_gain |= ((exp_p_gain)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->p_gain = -this->p_gain;
      uint32_t * val_i_gain = (uint32_t*) &(this->i_gain);
      offset += 3;
      *val_i_gain = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_i_gain |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_i_gain |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_i_gain |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_i_gain = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_i_gain |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_i_gain !=0)
        *val_i_gain |= ((exp_i_gain)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->i_gain = -this->i_gain;
      uint32_t * val_d_gain = (uint32_t*) &(this->d_gain);
      offset += 3;
      *val_d_gain = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_d_gain |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_d_gain |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_d_gain |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_d_gain = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_d_gain |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_d_gain !=0)
        *val_d_gain |= ((exp_d_gain)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->d_gain = -this->d_gain;
     return offset;
    }

    const char * getType(){ return "kobuki_msgs/ControllerInfo"; };
    const char * getMD5(){ return "46af38e72f0640f41ed22174d43a2550"; };

  };

}
#endif