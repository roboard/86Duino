#ifndef _ROS_manipulation_msgs_CartesianGains_h
#define _ROS_manipulation_msgs_CartesianGains_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"

namespace manipulation_msgs
{

  class CartesianGains : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t gains_length;
      float st_gains;
      float * gains;
      uint8_t fixed_frame_length;
      float st_fixed_frame;
      float * fixed_frame;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = gains_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < gains_length; i++){
      int32_t * val_gainsi = (int32_t *) &(this->gains[i]);
      int32_t exp_gainsi = (((*val_gainsi)>>23)&255);
      if(exp_gainsi != 0)
        exp_gainsi += 1023-127;
      int32_t sig_gainsi = *val_gainsi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_gainsi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_gainsi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_gainsi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_gainsi<<4) & 0xF0) | ((sig_gainsi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_gainsi>>4) & 0x7F;
      if(this->gains[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = fixed_frame_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < fixed_frame_length; i++){
      int32_t * val_fixed_framei = (int32_t *) &(this->fixed_frame[i]);
      int32_t exp_fixed_framei = (((*val_fixed_framei)>>23)&255);
      if(exp_fixed_framei != 0)
        exp_fixed_framei += 1023-127;
      int32_t sig_fixed_framei = *val_fixed_framei;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_fixed_framei<<5) & 0xff;
      *(outbuffer + offset++) = (sig_fixed_framei>>3) & 0xff;
      *(outbuffer + offset++) = (sig_fixed_framei>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_fixed_framei<<4) & 0xF0) | ((sig_fixed_framei>>19)&0x0F);
      *(outbuffer + offset++) = (exp_fixed_framei>>4) & 0x7F;
      if(this->fixed_frame[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint8_t gains_lengthT = *(inbuffer + offset++);
      if(gains_lengthT > gains_length)
        this->gains = (float*)realloc(this->gains, gains_lengthT * sizeof(float));
      offset += 3;
      gains_length = gains_lengthT;
      for( uint8_t i = 0; i < gains_length; i++){
      uint32_t * val_st_gains = (uint32_t*) &(this->st_gains);
      offset += 3;
      *val_st_gains = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_gains |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_gains |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_gains |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_gains = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_gains |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_gains !=0)
        *val_st_gains |= ((exp_st_gains)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_gains = -this->st_gains;
        memcpy( &(this->gains[i]), &(this->st_gains), sizeof(float));
      }
      uint8_t fixed_frame_lengthT = *(inbuffer + offset++);
      if(fixed_frame_lengthT > fixed_frame_length)
        this->fixed_frame = (float*)realloc(this->fixed_frame, fixed_frame_lengthT * sizeof(float));
      offset += 3;
      fixed_frame_length = fixed_frame_lengthT;
      for( uint8_t i = 0; i < fixed_frame_length; i++){
      uint32_t * val_st_fixed_frame = (uint32_t*) &(this->st_fixed_frame);
      offset += 3;
      *val_st_fixed_frame = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_fixed_frame |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_fixed_frame |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_fixed_frame |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_fixed_frame = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_fixed_frame |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_fixed_frame !=0)
        *val_st_fixed_frame |= ((exp_st_fixed_frame)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_fixed_frame = -this->st_fixed_frame;
        memcpy( &(this->fixed_frame[i]), &(this->st_fixed_frame), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "manipulation_msgs/CartesianGains"; };
    const char * getMD5(){ return "ab347f046ca5736a156ec424cbb63635"; };

  };

}
#endif