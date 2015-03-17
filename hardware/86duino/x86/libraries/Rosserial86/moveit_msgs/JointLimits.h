#ifndef _ROS_moveit_msgs_JointLimits_h
#define _ROS_moveit_msgs_JointLimits_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace moveit_msgs
{

  class JointLimits : public ros::Msg
  {
    public:
      char * joint_name;
      bool has_position_limits;
      float min_position;
      float max_position;
      bool has_velocity_limits;
      float max_velocity;
      bool has_acceleration_limits;
      float max_acceleration;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_joint_name = strlen( (const char*) this->joint_name);
      memcpy(outbuffer + offset, &length_joint_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->joint_name, length_joint_name);
      offset += length_joint_name;
      union {
        bool real;
        uint8_t base;
      } u_has_position_limits;
      u_has_position_limits.real = this->has_position_limits;
      *(outbuffer + offset + 0) = (u_has_position_limits.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->has_position_limits);
      int32_t * val_min_position = (int32_t *) &(this->min_position);
      int32_t exp_min_position = (((*val_min_position)>>23)&255);
      if(exp_min_position != 0)
        exp_min_position += 1023-127;
      int32_t sig_min_position = *val_min_position;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_min_position<<5) & 0xff;
      *(outbuffer + offset++) = (sig_min_position>>3) & 0xff;
      *(outbuffer + offset++) = (sig_min_position>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_min_position<<4) & 0xF0) | ((sig_min_position>>19)&0x0F);
      *(outbuffer + offset++) = (exp_min_position>>4) & 0x7F;
      if(this->min_position < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_max_position = (int32_t *) &(this->max_position);
      int32_t exp_max_position = (((*val_max_position)>>23)&255);
      if(exp_max_position != 0)
        exp_max_position += 1023-127;
      int32_t sig_max_position = *val_max_position;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_max_position<<5) & 0xff;
      *(outbuffer + offset++) = (sig_max_position>>3) & 0xff;
      *(outbuffer + offset++) = (sig_max_position>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_max_position<<4) & 0xF0) | ((sig_max_position>>19)&0x0F);
      *(outbuffer + offset++) = (exp_max_position>>4) & 0x7F;
      if(this->max_position < 0) *(outbuffer + offset -1) |= 0x80;
      union {
        bool real;
        uint8_t base;
      } u_has_velocity_limits;
      u_has_velocity_limits.real = this->has_velocity_limits;
      *(outbuffer + offset + 0) = (u_has_velocity_limits.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->has_velocity_limits);
      int32_t * val_max_velocity = (int32_t *) &(this->max_velocity);
      int32_t exp_max_velocity = (((*val_max_velocity)>>23)&255);
      if(exp_max_velocity != 0)
        exp_max_velocity += 1023-127;
      int32_t sig_max_velocity = *val_max_velocity;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_max_velocity<<5) & 0xff;
      *(outbuffer + offset++) = (sig_max_velocity>>3) & 0xff;
      *(outbuffer + offset++) = (sig_max_velocity>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_max_velocity<<4) & 0xF0) | ((sig_max_velocity>>19)&0x0F);
      *(outbuffer + offset++) = (exp_max_velocity>>4) & 0x7F;
      if(this->max_velocity < 0) *(outbuffer + offset -1) |= 0x80;
      union {
        bool real;
        uint8_t base;
      } u_has_acceleration_limits;
      u_has_acceleration_limits.real = this->has_acceleration_limits;
      *(outbuffer + offset + 0) = (u_has_acceleration_limits.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->has_acceleration_limits);
      int32_t * val_max_acceleration = (int32_t *) &(this->max_acceleration);
      int32_t exp_max_acceleration = (((*val_max_acceleration)>>23)&255);
      if(exp_max_acceleration != 0)
        exp_max_acceleration += 1023-127;
      int32_t sig_max_acceleration = *val_max_acceleration;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_max_acceleration<<5) & 0xff;
      *(outbuffer + offset++) = (sig_max_acceleration>>3) & 0xff;
      *(outbuffer + offset++) = (sig_max_acceleration>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_max_acceleration<<4) & 0xF0) | ((sig_max_acceleration>>19)&0x0F);
      *(outbuffer + offset++) = (exp_max_acceleration>>4) & 0x7F;
      if(this->max_acceleration < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_joint_name;
      memcpy(&length_joint_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_joint_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_joint_name-1]=0;
      this->joint_name = (char *)(inbuffer + offset-1);
      offset += length_joint_name;
      union {
        bool real;
        uint8_t base;
      } u_has_position_limits;
      u_has_position_limits.base = 0;
      u_has_position_limits.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->has_position_limits = u_has_position_limits.real;
      offset += sizeof(this->has_position_limits);
      uint32_t * val_min_position = (uint32_t*) &(this->min_position);
      offset += 3;
      *val_min_position = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_min_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_min_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_min_position |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_min_position = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_min_position |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_min_position !=0)
        *val_min_position |= ((exp_min_position)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->min_position = -this->min_position;
      uint32_t * val_max_position = (uint32_t*) &(this->max_position);
      offset += 3;
      *val_max_position = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_max_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_max_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_max_position |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_max_position = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_max_position |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_max_position !=0)
        *val_max_position |= ((exp_max_position)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->max_position = -this->max_position;
      union {
        bool real;
        uint8_t base;
      } u_has_velocity_limits;
      u_has_velocity_limits.base = 0;
      u_has_velocity_limits.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->has_velocity_limits = u_has_velocity_limits.real;
      offset += sizeof(this->has_velocity_limits);
      uint32_t * val_max_velocity = (uint32_t*) &(this->max_velocity);
      offset += 3;
      *val_max_velocity = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_max_velocity |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_max_velocity |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_max_velocity |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_max_velocity = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_max_velocity |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_max_velocity !=0)
        *val_max_velocity |= ((exp_max_velocity)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->max_velocity = -this->max_velocity;
      union {
        bool real;
        uint8_t base;
      } u_has_acceleration_limits;
      u_has_acceleration_limits.base = 0;
      u_has_acceleration_limits.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->has_acceleration_limits = u_has_acceleration_limits.real;
      offset += sizeof(this->has_acceleration_limits);
      uint32_t * val_max_acceleration = (uint32_t*) &(this->max_acceleration);
      offset += 3;
      *val_max_acceleration = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_max_acceleration |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_max_acceleration |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_max_acceleration |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_max_acceleration = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_max_acceleration |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_max_acceleration !=0)
        *val_max_acceleration |= ((exp_max_acceleration)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->max_acceleration = -this->max_acceleration;
     return offset;
    }

    const char * getType(){ return "moveit_msgs/JointLimits"; };
    const char * getMD5(){ return "8ca618c7329ea46142cbc864a2efe856"; };

  };

}
#endif