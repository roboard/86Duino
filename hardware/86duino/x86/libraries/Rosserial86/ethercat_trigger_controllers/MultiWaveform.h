#ifndef _ROS_ethercat_trigger_controllers_MultiWaveform_h
#define _ROS_ethercat_trigger_controllers_MultiWaveform_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "ethercat_trigger_controllers/MultiWaveformTransition.h"

namespace ethercat_trigger_controllers
{

  class MultiWaveform : public ros::Msg
  {
    public:
      float period;
      float zero_offset;
      uint8_t transitions_length;
      ethercat_trigger_controllers::MultiWaveformTransition st_transitions;
      ethercat_trigger_controllers::MultiWaveformTransition * transitions;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      int32_t * val_period = (int32_t *) &(this->period);
      int32_t exp_period = (((*val_period)>>23)&255);
      if(exp_period != 0)
        exp_period += 1023-127;
      int32_t sig_period = *val_period;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_period<<5) & 0xff;
      *(outbuffer + offset++) = (sig_period>>3) & 0xff;
      *(outbuffer + offset++) = (sig_period>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_period<<4) & 0xF0) | ((sig_period>>19)&0x0F);
      *(outbuffer + offset++) = (exp_period>>4) & 0x7F;
      if(this->period < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_zero_offset = (int32_t *) &(this->zero_offset);
      int32_t exp_zero_offset = (((*val_zero_offset)>>23)&255);
      if(exp_zero_offset != 0)
        exp_zero_offset += 1023-127;
      int32_t sig_zero_offset = *val_zero_offset;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_zero_offset<<5) & 0xff;
      *(outbuffer + offset++) = (sig_zero_offset>>3) & 0xff;
      *(outbuffer + offset++) = (sig_zero_offset>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_zero_offset<<4) & 0xF0) | ((sig_zero_offset>>19)&0x0F);
      *(outbuffer + offset++) = (exp_zero_offset>>4) & 0x7F;
      if(this->zero_offset < 0) *(outbuffer + offset -1) |= 0x80;
      *(outbuffer + offset++) = transitions_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < transitions_length; i++){
      offset += this->transitions[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t * val_period = (uint32_t*) &(this->period);
      offset += 3;
      *val_period = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_period |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_period |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_period |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_period = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_period |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_period !=0)
        *val_period |= ((exp_period)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->period = -this->period;
      uint32_t * val_zero_offset = (uint32_t*) &(this->zero_offset);
      offset += 3;
      *val_zero_offset = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_zero_offset |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_zero_offset |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_zero_offset |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_zero_offset = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_zero_offset |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_zero_offset !=0)
        *val_zero_offset |= ((exp_zero_offset)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->zero_offset = -this->zero_offset;
      uint8_t transitions_lengthT = *(inbuffer + offset++);
      if(transitions_lengthT > transitions_length)
        this->transitions = (ethercat_trigger_controllers::MultiWaveformTransition*)realloc(this->transitions, transitions_lengthT * sizeof(ethercat_trigger_controllers::MultiWaveformTransition));
      offset += 3;
      transitions_length = transitions_lengthT;
      for( uint8_t i = 0; i < transitions_length; i++){
      offset += this->st_transitions.deserialize(inbuffer + offset);
        memcpy( &(this->transitions[i]), &(this->st_transitions), sizeof(ethercat_trigger_controllers::MultiWaveformTransition));
      }
     return offset;
    }

    const char * getType(){ return "ethercat_trigger_controllers/MultiWaveform"; };
    const char * getMD5(){ return "6a8e166563c159e73f391a302e7b37f6"; };

  };

}
#endif