#ifndef _ROS_SERVICE_SetProfile_h
#define _ROS_SERVICE_SetProfile_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_mechanism_controllers
{

static const char SETPROFILE[] = "pr2_mechanism_controllers/SetProfile";

  class SetProfileRequest : public ros::Msg
  {
    public:
      float UpperTurnaround;
      float LowerTurnaround;
      float upperDecelBuffer;
      float lowerDecelBuffer;
      float profile;
      float period;
      float amplitude;
      float offset;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      int32_t * val_UpperTurnaround = (int32_t *) &(this->UpperTurnaround);
      int32_t exp_UpperTurnaround = (((*val_UpperTurnaround)>>23)&255);
      if(exp_UpperTurnaround != 0)
        exp_UpperTurnaround += 1023-127;
      int32_t sig_UpperTurnaround = *val_UpperTurnaround;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_UpperTurnaround<<5) & 0xff;
      *(outbuffer + offset++) = (sig_UpperTurnaround>>3) & 0xff;
      *(outbuffer + offset++) = (sig_UpperTurnaround>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_UpperTurnaround<<4) & 0xF0) | ((sig_UpperTurnaround>>19)&0x0F);
      *(outbuffer + offset++) = (exp_UpperTurnaround>>4) & 0x7F;
      if(this->UpperTurnaround < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_LowerTurnaround = (int32_t *) &(this->LowerTurnaround);
      int32_t exp_LowerTurnaround = (((*val_LowerTurnaround)>>23)&255);
      if(exp_LowerTurnaround != 0)
        exp_LowerTurnaround += 1023-127;
      int32_t sig_LowerTurnaround = *val_LowerTurnaround;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_LowerTurnaround<<5) & 0xff;
      *(outbuffer + offset++) = (sig_LowerTurnaround>>3) & 0xff;
      *(outbuffer + offset++) = (sig_LowerTurnaround>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_LowerTurnaround<<4) & 0xF0) | ((sig_LowerTurnaround>>19)&0x0F);
      *(outbuffer + offset++) = (exp_LowerTurnaround>>4) & 0x7F;
      if(this->LowerTurnaround < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_upperDecelBuffer = (int32_t *) &(this->upperDecelBuffer);
      int32_t exp_upperDecelBuffer = (((*val_upperDecelBuffer)>>23)&255);
      if(exp_upperDecelBuffer != 0)
        exp_upperDecelBuffer += 1023-127;
      int32_t sig_upperDecelBuffer = *val_upperDecelBuffer;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_upperDecelBuffer<<5) & 0xff;
      *(outbuffer + offset++) = (sig_upperDecelBuffer>>3) & 0xff;
      *(outbuffer + offset++) = (sig_upperDecelBuffer>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_upperDecelBuffer<<4) & 0xF0) | ((sig_upperDecelBuffer>>19)&0x0F);
      *(outbuffer + offset++) = (exp_upperDecelBuffer>>4) & 0x7F;
      if(this->upperDecelBuffer < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_lowerDecelBuffer = (int32_t *) &(this->lowerDecelBuffer);
      int32_t exp_lowerDecelBuffer = (((*val_lowerDecelBuffer)>>23)&255);
      if(exp_lowerDecelBuffer != 0)
        exp_lowerDecelBuffer += 1023-127;
      int32_t sig_lowerDecelBuffer = *val_lowerDecelBuffer;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_lowerDecelBuffer<<5) & 0xff;
      *(outbuffer + offset++) = (sig_lowerDecelBuffer>>3) & 0xff;
      *(outbuffer + offset++) = (sig_lowerDecelBuffer>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_lowerDecelBuffer<<4) & 0xF0) | ((sig_lowerDecelBuffer>>19)&0x0F);
      *(outbuffer + offset++) = (exp_lowerDecelBuffer>>4) & 0x7F;
      if(this->lowerDecelBuffer < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_profile = (int32_t *) &(this->profile);
      int32_t exp_profile = (((*val_profile)>>23)&255);
      if(exp_profile != 0)
        exp_profile += 1023-127;
      int32_t sig_profile = *val_profile;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_profile<<5) & 0xff;
      *(outbuffer + offset++) = (sig_profile>>3) & 0xff;
      *(outbuffer + offset++) = (sig_profile>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_profile<<4) & 0xF0) | ((sig_profile>>19)&0x0F);
      *(outbuffer + offset++) = (exp_profile>>4) & 0x7F;
      if(this->profile < 0) *(outbuffer + offset -1) |= 0x80;
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
      uint32_t * val_UpperTurnaround = (uint32_t*) &(this->UpperTurnaround);
      offset += 3;
      *val_UpperTurnaround = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_UpperTurnaround |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_UpperTurnaround |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_UpperTurnaround |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_UpperTurnaround = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_UpperTurnaround |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_UpperTurnaround !=0)
        *val_UpperTurnaround |= ((exp_UpperTurnaround)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->UpperTurnaround = -this->UpperTurnaround;
      uint32_t * val_LowerTurnaround = (uint32_t*) &(this->LowerTurnaround);
      offset += 3;
      *val_LowerTurnaround = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_LowerTurnaround |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_LowerTurnaround |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_LowerTurnaround |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_LowerTurnaround = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_LowerTurnaround |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_LowerTurnaround !=0)
        *val_LowerTurnaround |= ((exp_LowerTurnaround)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->LowerTurnaround = -this->LowerTurnaround;
      uint32_t * val_upperDecelBuffer = (uint32_t*) &(this->upperDecelBuffer);
      offset += 3;
      *val_upperDecelBuffer = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_upperDecelBuffer |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_upperDecelBuffer |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_upperDecelBuffer |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_upperDecelBuffer = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_upperDecelBuffer |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_upperDecelBuffer !=0)
        *val_upperDecelBuffer |= ((exp_upperDecelBuffer)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->upperDecelBuffer = -this->upperDecelBuffer;
      uint32_t * val_lowerDecelBuffer = (uint32_t*) &(this->lowerDecelBuffer);
      offset += 3;
      *val_lowerDecelBuffer = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_lowerDecelBuffer |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_lowerDecelBuffer |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_lowerDecelBuffer |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_lowerDecelBuffer = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_lowerDecelBuffer |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_lowerDecelBuffer !=0)
        *val_lowerDecelBuffer |= ((exp_lowerDecelBuffer)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->lowerDecelBuffer = -this->lowerDecelBuffer;
      uint32_t * val_profile = (uint32_t*) &(this->profile);
      offset += 3;
      *val_profile = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_profile |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_profile |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_profile |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_profile = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_profile |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_profile !=0)
        *val_profile |= ((exp_profile)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->profile = -this->profile;
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

    const char * getType(){ return SETPROFILE; };
    const char * getMD5(){ return "309001fc196b0094f23b1ae2bd672fb2"; };

  };

  class SetProfileResponse : public ros::Msg
  {
    public:
      float time;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      int32_t * val_time = (int32_t *) &(this->time);
      int32_t exp_time = (((*val_time)>>23)&255);
      if(exp_time != 0)
        exp_time += 1023-127;
      int32_t sig_time = *val_time;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_time<<5) & 0xff;
      *(outbuffer + offset++) = (sig_time>>3) & 0xff;
      *(outbuffer + offset++) = (sig_time>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_time<<4) & 0xF0) | ((sig_time>>19)&0x0F);
      *(outbuffer + offset++) = (exp_time>>4) & 0x7F;
      if(this->time < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t * val_time = (uint32_t*) &(this->time);
      offset += 3;
      *val_time = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_time |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_time = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_time |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_time !=0)
        *val_time |= ((exp_time)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->time = -this->time;
     return offset;
    }

    const char * getType(){ return SETPROFILE; };
    const char * getMD5(){ return "be5310e7aa4c90cdee120add91648cee"; };

  };

  class SetProfile {
    public:
    typedef SetProfileRequest Request;
    typedef SetProfileResponse Response;
  };

}
#endif
