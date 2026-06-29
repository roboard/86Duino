#ifndef _ROS_pr2_msgs_PeriodicCmd_h
#define _ROS_pr2_msgs_PeriodicCmd_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"

namespace pr2_msgs
{

  class PeriodicCmd : public ros::Msg
  {
    public:
      std_msgs::Header header;
      char * profile;
      float period;
      float amplitude;
      float offset;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      uint32_t length_profile = strlen( (const char*) this->profile);
      memcpy(outbuffer + offset, &length_profile, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->profile, length_profile);
      offset += length_profile;
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
      int32_t * val_amplitude = (int32_t *) &(this->amplitude);
      int32_t exp_amplitude = (((*val_amplitude)>>23)&255);
      if(exp_amplitude != 0)
        exp_amplitude += 1023-127;
      int32_t sig_amplitude = *val_amplitude;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_amplitude<<5) & 0xff;
      *(outbuffer + offset++) = (sig_amplitude>>3) & 0xff;
      *(outbuffer + offset++) = (sig_amplitude>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_amplitude<<4) & 0xF0) | ((sig_amplitude>>19)&0x0F);
      *(outbuffer + offset++) = (exp_amplitude>>4) & 0x7F;
      if(this->amplitude < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_offset = (int32_t *) &(this->offset);
      int32_t exp_offset = (((*val_offset)>>23)&255);
      if(exp_offset != 0)
        exp_offset += 1023-127;
      int32_t sig_offset = *val_offset;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_offset<<5) & 0xff;
      *(outbuffer + offset++) = (sig_offset>>3) & 0xff;
      *(outbuffer + offset++) = (sig_offset>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_offset<<4) & 0xF0) | ((sig_offset>>19)&0x0F);
      *(outbuffer + offset++) = (exp_offset>>4) & 0x7F;
      if(this->offset < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint32_t length_profile;
      memcpy(&length_profile, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_profile; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_profile-1]=0;
      this->profile = (char *)(inbuffer + offset-1);
      offset += length_profile;
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
      uint32_t * val_amplitude = (uint32_t*) &(this->amplitude);
      offset += 3;
      *val_amplitude = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_amplitude |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_amplitude |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_amplitude |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_amplitude = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_amplitude |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_amplitude !=0)
        *val_amplitude |= ((exp_amplitude)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->amplitude = -this->amplitude;
      uint32_t * val_offset = (uint32_t*) &(this->offset);
      offset += 3;
      *val_offset = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_offset |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_offset |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_offset |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_offset = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_offset |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_offset !=0)
        *val_offset |= ((exp_offset)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->offset = -this->offset;
     return offset;
    }

    const char * getType(){ return "pr2_msgs/PeriodicCmd"; };
    const char * getMD5(){ return "95ab7e548e3d4274f83393129dd96c2e"; };

  };

}
#endif