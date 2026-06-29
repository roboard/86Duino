#ifndef _ROS_moveit_msgs_JointConstraint_h
#define _ROS_moveit_msgs_JointConstraint_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace moveit_msgs
{

  class JointConstraint : public ros::Msg
  {
    public:
      char * joint_name;
      float position;
      float tolerance_above;
      float tolerance_below;
      float weight;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_joint_name = strlen( (const char*) this->joint_name);
      memcpy(outbuffer + offset, &length_joint_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->joint_name, length_joint_name);
      offset += length_joint_name;
      int32_t * val_position = (int32_t *) &(this->position);
      int32_t exp_position = (((*val_position)>>23)&255);
      if(exp_position != 0)
        exp_position += 1023-127;
      int32_t sig_position = *val_position;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_position<<5) & 0xff;
      *(outbuffer + offset++) = (sig_position>>3) & 0xff;
      *(outbuffer + offset++) = (sig_position>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_position<<4) & 0xF0) | ((sig_position>>19)&0x0F);
      *(outbuffer + offset++) = (exp_position>>4) & 0x7F;
      if(this->position < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_tolerance_above = (int32_t *) &(this->tolerance_above);
      int32_t exp_tolerance_above = (((*val_tolerance_above)>>23)&255);
      if(exp_tolerance_above != 0)
        exp_tolerance_above += 1023-127;
      int32_t sig_tolerance_above = *val_tolerance_above;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_tolerance_above<<5) & 0xff;
      *(outbuffer + offset++) = (sig_tolerance_above>>3) & 0xff;
      *(outbuffer + offset++) = (sig_tolerance_above>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_tolerance_above<<4) & 0xF0) | ((sig_tolerance_above>>19)&0x0F);
      *(outbuffer + offset++) = (exp_tolerance_above>>4) & 0x7F;
      if(this->tolerance_above < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_tolerance_below = (int32_t *) &(this->tolerance_below);
      int32_t exp_tolerance_below = (((*val_tolerance_below)>>23)&255);
      if(exp_tolerance_below != 0)
        exp_tolerance_below += 1023-127;
      int32_t sig_tolerance_below = *val_tolerance_below;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_tolerance_below<<5) & 0xff;
      *(outbuffer + offset++) = (sig_tolerance_below>>3) & 0xff;
      *(outbuffer + offset++) = (sig_tolerance_below>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_tolerance_below<<4) & 0xF0) | ((sig_tolerance_below>>19)&0x0F);
      *(outbuffer + offset++) = (exp_tolerance_below>>4) & 0x7F;
      if(this->tolerance_below < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_weight = (int32_t *) &(this->weight);
      int32_t exp_weight = (((*val_weight)>>23)&255);
      if(exp_weight != 0)
        exp_weight += 1023-127;
      int32_t sig_weight = *val_weight;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_weight<<5) & 0xff;
      *(outbuffer + offset++) = (sig_weight>>3) & 0xff;
      *(outbuffer + offset++) = (sig_weight>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_weight<<4) & 0xF0) | ((sig_weight>>19)&0x0F);
      *(outbuffer + offset++) = (exp_weight>>4) & 0x7F;
      if(this->weight < 0) *(outbuffer + offset -1) |= 0x80;
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
      uint32_t * val_position = (uint32_t*) &(this->position);
      offset += 3;
      *val_position = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_position |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_position = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_position |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_position !=0)
        *val_position |= ((exp_position)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->position = -this->position;
      uint32_t * val_tolerance_above = (uint32_t*) &(this->tolerance_above);
      offset += 3;
      *val_tolerance_above = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_tolerance_above |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_tolerance_above |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_tolerance_above |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_tolerance_above = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_tolerance_above |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_tolerance_above !=0)
        *val_tolerance_above |= ((exp_tolerance_above)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->tolerance_above = -this->tolerance_above;
      uint32_t * val_tolerance_below = (uint32_t*) &(this->tolerance_below);
      offset += 3;
      *val_tolerance_below = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_tolerance_below |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_tolerance_below |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_tolerance_below |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_tolerance_below = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_tolerance_below |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_tolerance_below !=0)
        *val_tolerance_below |= ((exp_tolerance_below)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->tolerance_below = -this->tolerance_below;
      uint32_t * val_weight = (uint32_t*) &(this->weight);
      offset += 3;
      *val_weight = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_weight |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_weight |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_weight |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_weight = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_weight |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_weight !=0)
        *val_weight |= ((exp_weight)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->weight = -this->weight;
     return offset;
    }

    const char * getType(){ return "moveit_msgs/JointConstraint"; };
    const char * getMD5(){ return "c02a15146bec0ce13564807805b008f0"; };

  };

}
#endif