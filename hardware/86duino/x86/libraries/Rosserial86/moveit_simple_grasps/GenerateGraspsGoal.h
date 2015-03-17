#ifndef _ROS_moveit_simple_grasps_GenerateGraspsGoal_h
#define _ROS_moveit_simple_grasps_GenerateGraspsGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Pose.h"
#include "moveit_simple_grasps/GraspGeneratorOptions.h"

namespace moveit_simple_grasps
{

  class GenerateGraspsGoal : public ros::Msg
  {
    public:
      geometry_msgs::Pose pose;
      float width;
      uint8_t options_length;
      moveit_simple_grasps::GraspGeneratorOptions st_options;
      moveit_simple_grasps::GraspGeneratorOptions * options;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->pose.serialize(outbuffer + offset);
      int32_t * val_width = (int32_t *) &(this->width);
      int32_t exp_width = (((*val_width)>>23)&255);
      if(exp_width != 0)
        exp_width += 1023-127;
      int32_t sig_width = *val_width;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_width<<5) & 0xff;
      *(outbuffer + offset++) = (sig_width>>3) & 0xff;
      *(outbuffer + offset++) = (sig_width>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_width<<4) & 0xF0) | ((sig_width>>19)&0x0F);
      *(outbuffer + offset++) = (exp_width>>4) & 0x7F;
      if(this->width < 0) *(outbuffer + offset -1) |= 0x80;
      *(outbuffer + offset++) = options_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < options_length; i++){
      offset += this->options[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->pose.deserialize(inbuffer + offset);
      uint32_t * val_width = (uint32_t*) &(this->width);
      offset += 3;
      *val_width = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_width |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_width |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_width |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_width = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_width |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_width !=0)
        *val_width |= ((exp_width)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->width = -this->width;
      uint8_t options_lengthT = *(inbuffer + offset++);
      if(options_lengthT > options_length)
        this->options = (moveit_simple_grasps::GraspGeneratorOptions*)realloc(this->options, options_lengthT * sizeof(moveit_simple_grasps::GraspGeneratorOptions));
      offset += 3;
      options_length = options_lengthT;
      for( uint8_t i = 0; i < options_length; i++){
      offset += this->st_options.deserialize(inbuffer + offset);
        memcpy( &(this->options[i]), &(this->st_options), sizeof(moveit_simple_grasps::GraspGeneratorOptions));
      }
     return offset;
    }

    const char * getType(){ return "moveit_simple_grasps/GenerateGraspsGoal"; };
    const char * getMD5(){ return "162875df5b62e9fb4be6d2aa5a67176d"; };

  };

}
#endif