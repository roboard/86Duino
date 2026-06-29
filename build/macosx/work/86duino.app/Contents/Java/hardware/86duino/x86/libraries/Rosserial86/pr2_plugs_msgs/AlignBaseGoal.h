#ifndef _ROS_pr2_plugs_msgs_AlignBaseGoal_h
#define _ROS_pr2_plugs_msgs_AlignBaseGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/PointStamped.h"

namespace pr2_plugs_msgs
{

  class AlignBaseGoal : public ros::Msg
  {
    public:
      float offset;
      float desired_distance;
      geometry_msgs::PointStamped look_point;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
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
      int32_t * val_desired_distance = (int32_t *) &(this->desired_distance);
      int32_t exp_desired_distance = (((*val_desired_distance)>>23)&255);
      if(exp_desired_distance != 0)
        exp_desired_distance += 1023-127;
      int32_t sig_desired_distance = *val_desired_distance;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_desired_distance<<5) & 0xff;
      *(outbuffer + offset++) = (sig_desired_distance>>3) & 0xff;
      *(outbuffer + offset++) = (sig_desired_distance>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_desired_distance<<4) & 0xF0) | ((sig_desired_distance>>19)&0x0F);
      *(outbuffer + offset++) = (exp_desired_distance>>4) & 0x7F;
      if(this->desired_distance < 0) *(outbuffer + offset -1) |= 0x80;
      offset += this->look_point.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
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
      uint32_t * val_desired_distance = (uint32_t*) &(this->desired_distance);
      offset += 3;
      *val_desired_distance = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_desired_distance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_desired_distance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_desired_distance |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_desired_distance = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_desired_distance |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_desired_distance !=0)
        *val_desired_distance |= ((exp_desired_distance)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->desired_distance = -this->desired_distance;
      offset += this->look_point.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/AlignBaseGoal"; };
    const char * getMD5(){ return "466c82e85a2e949afed3e41902c58a3f"; };

  };

}
#endif