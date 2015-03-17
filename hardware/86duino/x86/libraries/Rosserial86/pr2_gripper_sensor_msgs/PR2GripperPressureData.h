#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperPressureData_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperPressureData_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperPressureData : public ros::Msg
  {
    public:
      float pressure_left[22];
      float pressure_right[22];
      float rostime;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      unsigned char * pressure_left_val = (unsigned char *) this->pressure_left;
      for( uint8_t i = 0; i < 22; i++){
      int32_t * val_pressure_lefti = (int32_t *) &(this->pressure_left[i]);
      int32_t exp_pressure_lefti = (((*val_pressure_lefti)>>23)&255);
      if(exp_pressure_lefti != 0)
        exp_pressure_lefti += 1023-127;
      int32_t sig_pressure_lefti = *val_pressure_lefti;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_pressure_lefti<<5) & 0xff;
      *(outbuffer + offset++) = (sig_pressure_lefti>>3) & 0xff;
      *(outbuffer + offset++) = (sig_pressure_lefti>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_pressure_lefti<<4) & 0xF0) | ((sig_pressure_lefti>>19)&0x0F);
      *(outbuffer + offset++) = (exp_pressure_lefti>>4) & 0x7F;
      if(this->pressure_left[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      unsigned char * pressure_right_val = (unsigned char *) this->pressure_right;
      for( uint8_t i = 0; i < 22; i++){
      int32_t * val_pressure_righti = (int32_t *) &(this->pressure_right[i]);
      int32_t exp_pressure_righti = (((*val_pressure_righti)>>23)&255);
      if(exp_pressure_righti != 0)
        exp_pressure_righti += 1023-127;
      int32_t sig_pressure_righti = *val_pressure_righti;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_pressure_righti<<5) & 0xff;
      *(outbuffer + offset++) = (sig_pressure_righti>>3) & 0xff;
      *(outbuffer + offset++) = (sig_pressure_righti>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_pressure_righti<<4) & 0xF0) | ((sig_pressure_righti>>19)&0x0F);
      *(outbuffer + offset++) = (exp_pressure_righti>>4) & 0x7F;
      if(this->pressure_right[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      int32_t * val_rostime = (int32_t *) &(this->rostime);
      int32_t exp_rostime = (((*val_rostime)>>23)&255);
      if(exp_rostime != 0)
        exp_rostime += 1023-127;
      int32_t sig_rostime = *val_rostime;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_rostime<<5) & 0xff;
      *(outbuffer + offset++) = (sig_rostime>>3) & 0xff;
      *(outbuffer + offset++) = (sig_rostime>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_rostime<<4) & 0xF0) | ((sig_rostime>>19)&0x0F);
      *(outbuffer + offset++) = (exp_rostime>>4) & 0x7F;
      if(this->rostime < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t * pressure_left_val = (uint8_t*) this->pressure_left;
      for( uint8_t i = 0; i < 22; i++){
      uint32_t * val_pressure_lefti = (uint32_t*) &(this->pressure_left[i]);
      offset += 3;
      *val_pressure_lefti = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_pressure_lefti |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_pressure_lefti |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_pressure_lefti |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_pressure_lefti = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_pressure_lefti |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_pressure_lefti !=0)
        *val_pressure_lefti |= ((exp_pressure_lefti)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->pressure_left[i] = -this->pressure_left[i];
      }
      uint8_t * pressure_right_val = (uint8_t*) this->pressure_right;
      for( uint8_t i = 0; i < 22; i++){
      uint32_t * val_pressure_righti = (uint32_t*) &(this->pressure_right[i]);
      offset += 3;
      *val_pressure_righti = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_pressure_righti |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_pressure_righti |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_pressure_righti |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_pressure_righti = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_pressure_righti |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_pressure_righti !=0)
        *val_pressure_righti |= ((exp_pressure_righti)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->pressure_right[i] = -this->pressure_right[i];
      }
      uint32_t * val_rostime = (uint32_t*) &(this->rostime);
      offset += 3;
      *val_rostime = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_rostime |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_rostime |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_rostime |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_rostime = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_rostime |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_rostime !=0)
        *val_rostime |= ((exp_rostime)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->rostime = -this->rostime;
     return offset;
    }

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperPressureData"; };
    const char * getMD5(){ return "b69255f5117bf05fdcd1e83d4e6ab779"; };

  };

}
#endif