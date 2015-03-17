#ifndef _ROS_SERVICE_SetWaveform_h
#define _ROS_SERVICE_SetWaveform_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace ethercat_trigger_controllers
{

static const char SETWAVEFORM[] = "ethercat_trigger_controllers/SetWaveform";

  class SetWaveformRequest : public ros::Msg
  {
    public:
      float rep_rate;
      float phase;
      float duty_cycle;
      int32_t running;
      int32_t active_low;
      int32_t pulsed;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      int32_t * val_rep_rate = (int32_t *) &(this->rep_rate);
      int32_t exp_rep_rate = (((*val_rep_rate)>>23)&255);
      if(exp_rep_rate != 0)
        exp_rep_rate += 1023-127;
      int32_t sig_rep_rate = *val_rep_rate;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_rep_rate<<5) & 0xff;
      *(outbuffer + offset++) = (sig_rep_rate>>3) & 0xff;
      *(outbuffer + offset++) = (sig_rep_rate>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_rep_rate<<4) & 0xF0) | ((sig_rep_rate>>19)&0x0F);
      *(outbuffer + offset++) = (exp_rep_rate>>4) & 0x7F;
      if(this->rep_rate < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_phase = (int32_t *) &(this->phase);
      int32_t exp_phase = (((*val_phase)>>23)&255);
      if(exp_phase != 0)
        exp_phase += 1023-127;
      int32_t sig_phase = *val_phase;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_phase<<5) & 0xff;
      *(outbuffer + offset++) = (sig_phase>>3) & 0xff;
      *(outbuffer + offset++) = (sig_phase>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_phase<<4) & 0xF0) | ((sig_phase>>19)&0x0F);
      *(outbuffer + offset++) = (exp_phase>>4) & 0x7F;
      if(this->phase < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_duty_cycle = (int32_t *) &(this->duty_cycle);
      int32_t exp_duty_cycle = (((*val_duty_cycle)>>23)&255);
      if(exp_duty_cycle != 0)
        exp_duty_cycle += 1023-127;
      int32_t sig_duty_cycle = *val_duty_cycle;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_duty_cycle<<5) & 0xff;
      *(outbuffer + offset++) = (sig_duty_cycle>>3) & 0xff;
      *(outbuffer + offset++) = (sig_duty_cycle>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_duty_cycle<<4) & 0xF0) | ((sig_duty_cycle>>19)&0x0F);
      *(outbuffer + offset++) = (exp_duty_cycle>>4) & 0x7F;
      if(this->duty_cycle < 0) *(outbuffer + offset -1) |= 0x80;
      union {
        int32_t real;
        uint32_t base;
      } u_running;
      u_running.real = this->running;
      *(outbuffer + offset + 0) = (u_running.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_running.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_running.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_running.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->running);
      union {
        int32_t real;
        uint32_t base;
      } u_active_low;
      u_active_low.real = this->active_low;
      *(outbuffer + offset + 0) = (u_active_low.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_active_low.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_active_low.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_active_low.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->active_low);
      union {
        int32_t real;
        uint32_t base;
      } u_pulsed;
      u_pulsed.real = this->pulsed;
      *(outbuffer + offset + 0) = (u_pulsed.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_pulsed.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_pulsed.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_pulsed.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->pulsed);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t * val_rep_rate = (uint32_t*) &(this->rep_rate);
      offset += 3;
      *val_rep_rate = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_rep_rate |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_rep_rate |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_rep_rate |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_rep_rate = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_rep_rate |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_rep_rate !=0)
        *val_rep_rate |= ((exp_rep_rate)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->rep_rate = -this->rep_rate;
      uint32_t * val_phase = (uint32_t*) &(this->phase);
      offset += 3;
      *val_phase = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_phase |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_phase |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_phase |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_phase = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_phase |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_phase !=0)
        *val_phase |= ((exp_phase)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->phase = -this->phase;
      uint32_t * val_duty_cycle = (uint32_t*) &(this->duty_cycle);
      offset += 3;
      *val_duty_cycle = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_duty_cycle |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_duty_cycle |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_duty_cycle |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_duty_cycle = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_duty_cycle |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_duty_cycle !=0)
        *val_duty_cycle |= ((exp_duty_cycle)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->duty_cycle = -this->duty_cycle;
      union {
        int32_t real;
        uint32_t base;
      } u_running;
      u_running.base = 0;
      u_running.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_running.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_running.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_running.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->running = u_running.real;
      offset += sizeof(this->running);
      union {
        int32_t real;
        uint32_t base;
      } u_active_low;
      u_active_low.base = 0;
      u_active_low.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_active_low.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_active_low.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_active_low.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->active_low = u_active_low.real;
      offset += sizeof(this->active_low);
      union {
        int32_t real;
        uint32_t base;
      } u_pulsed;
      u_pulsed.base = 0;
      u_pulsed.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_pulsed.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_pulsed.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_pulsed.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->pulsed = u_pulsed.real;
      offset += sizeof(this->pulsed);
     return offset;
    }

    const char * getType(){ return SETWAVEFORM; };
    const char * getMD5(){ return "988450e1ddd386f3967c381c19b2330c"; };

  };

  class SetWaveformResponse : public ros::Msg
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

    const char * getType(){ return SETWAVEFORM; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class SetWaveform {
    public:
    typedef SetWaveformRequest Request;
    typedef SetWaveformResponse Response;
  };

}
#endif
