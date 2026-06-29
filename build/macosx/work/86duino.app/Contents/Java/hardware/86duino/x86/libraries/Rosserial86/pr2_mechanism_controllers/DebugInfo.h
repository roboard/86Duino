#ifndef _ROS_pr2_mechanism_controllers_DebugInfo_h
#define _ROS_pr2_mechanism_controllers_DebugInfo_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_mechanism_controllers
{

  class DebugInfo : public ros::Msg
  {
    public:
      uint8_t timing_length;
      float st_timing;
      float * timing;
      int32_t sequence;
      bool input_valid;
      float residual;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = timing_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < timing_length; i++){
      int32_t * val_timingi = (int32_t *) &(this->timing[i]);
      int32_t exp_timingi = (((*val_timingi)>>23)&255);
      if(exp_timingi != 0)
        exp_timingi += 1023-127;
      int32_t sig_timingi = *val_timingi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_timingi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_timingi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_timingi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_timingi<<4) & 0xF0) | ((sig_timingi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_timingi>>4) & 0x7F;
      if(this->timing[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      union {
        int32_t real;
        uint32_t base;
      } u_sequence;
      u_sequence.real = this->sequence;
      *(outbuffer + offset + 0) = (u_sequence.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_sequence.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_sequence.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_sequence.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->sequence);
      union {
        bool real;
        uint8_t base;
      } u_input_valid;
      u_input_valid.real = this->input_valid;
      *(outbuffer + offset + 0) = (u_input_valid.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->input_valid);
      int32_t * val_residual = (int32_t *) &(this->residual);
      int32_t exp_residual = (((*val_residual)>>23)&255);
      if(exp_residual != 0)
        exp_residual += 1023-127;
      int32_t sig_residual = *val_residual;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_residual<<5) & 0xff;
      *(outbuffer + offset++) = (sig_residual>>3) & 0xff;
      *(outbuffer + offset++) = (sig_residual>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_residual<<4) & 0xF0) | ((sig_residual>>19)&0x0F);
      *(outbuffer + offset++) = (exp_residual>>4) & 0x7F;
      if(this->residual < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t timing_lengthT = *(inbuffer + offset++);
      if(timing_lengthT > timing_length)
        this->timing = (float*)realloc(this->timing, timing_lengthT * sizeof(float));
      offset += 3;
      timing_length = timing_lengthT;
      for( uint8_t i = 0; i < timing_length; i++){
      uint32_t * val_st_timing = (uint32_t*) &(this->st_timing);
      offset += 3;
      *val_st_timing = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_timing |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_timing |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_timing |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_timing = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_timing |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_timing !=0)
        *val_st_timing |= ((exp_st_timing)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_timing = -this->st_timing;
        memcpy( &(this->timing[i]), &(this->st_timing), sizeof(float));
      }
      union {
        int32_t real;
        uint32_t base;
      } u_sequence;
      u_sequence.base = 0;
      u_sequence.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_sequence.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_sequence.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_sequence.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->sequence = u_sequence.real;
      offset += sizeof(this->sequence);
      union {
        bool real;
        uint8_t base;
      } u_input_valid;
      u_input_valid.base = 0;
      u_input_valid.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->input_valid = u_input_valid.real;
      offset += sizeof(this->input_valid);
      uint32_t * val_residual = (uint32_t*) &(this->residual);
      offset += 3;
      *val_residual = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_residual |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_residual |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_residual |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_residual = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_residual |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_residual !=0)
        *val_residual |= ((exp_residual)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->residual = -this->residual;
     return offset;
    }

    const char * getType(){ return "pr2_mechanism_controllers/DebugInfo"; };
    const char * getMD5(){ return "6281356ce897e33da024b8eb319460f2"; };

  };

}
#endif