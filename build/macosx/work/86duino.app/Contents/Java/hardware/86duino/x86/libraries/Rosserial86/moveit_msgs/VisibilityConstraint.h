#ifndef _ROS_moveit_msgs_VisibilityConstraint_h
#define _ROS_moveit_msgs_VisibilityConstraint_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/PoseStamped.h"

namespace moveit_msgs
{

  class VisibilityConstraint : public ros::Msg
  {
    public:
      float target_radius;
      geometry_msgs::PoseStamped target_pose;
      int32_t cone_sides;
      geometry_msgs::PoseStamped sensor_pose;
      float max_view_angle;
      float max_range_angle;
      uint8_t sensor_view_direction;
      float weight;
      enum { SENSOR_Z = 0 };
      enum { SENSOR_Y = 1 };
      enum { SENSOR_X = 2 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      int32_t * val_target_radius = (int32_t *) &(this->target_radius);
      int32_t exp_target_radius = (((*val_target_radius)>>23)&255);
      if(exp_target_radius != 0)
        exp_target_radius += 1023-127;
      int32_t sig_target_radius = *val_target_radius;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_target_radius<<5) & 0xff;
      *(outbuffer + offset++) = (sig_target_radius>>3) & 0xff;
      *(outbuffer + offset++) = (sig_target_radius>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_target_radius<<4) & 0xF0) | ((sig_target_radius>>19)&0x0F);
      *(outbuffer + offset++) = (exp_target_radius>>4) & 0x7F;
      if(this->target_radius < 0) *(outbuffer + offset -1) |= 0x80;
      offset += this->target_pose.serialize(outbuffer + offset);
      union {
        int32_t real;
        uint32_t base;
      } u_cone_sides;
      u_cone_sides.real = this->cone_sides;
      *(outbuffer + offset + 0) = (u_cone_sides.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_cone_sides.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_cone_sides.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_cone_sides.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->cone_sides);
      offset += this->sensor_pose.serialize(outbuffer + offset);
      int32_t * val_max_view_angle = (int32_t *) &(this->max_view_angle);
      int32_t exp_max_view_angle = (((*val_max_view_angle)>>23)&255);
      if(exp_max_view_angle != 0)
        exp_max_view_angle += 1023-127;
      int32_t sig_max_view_angle = *val_max_view_angle;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_max_view_angle<<5) & 0xff;
      *(outbuffer + offset++) = (sig_max_view_angle>>3) & 0xff;
      *(outbuffer + offset++) = (sig_max_view_angle>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_max_view_angle<<4) & 0xF0) | ((sig_max_view_angle>>19)&0x0F);
      *(outbuffer + offset++) = (exp_max_view_angle>>4) & 0x7F;
      if(this->max_view_angle < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_max_range_angle = (int32_t *) &(this->max_range_angle);
      int32_t exp_max_range_angle = (((*val_max_range_angle)>>23)&255);
      if(exp_max_range_angle != 0)
        exp_max_range_angle += 1023-127;
      int32_t sig_max_range_angle = *val_max_range_angle;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_max_range_angle<<5) & 0xff;
      *(outbuffer + offset++) = (sig_max_range_angle>>3) & 0xff;
      *(outbuffer + offset++) = (sig_max_range_angle>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_max_range_angle<<4) & 0xF0) | ((sig_max_range_angle>>19)&0x0F);
      *(outbuffer + offset++) = (exp_max_range_angle>>4) & 0x7F;
      if(this->max_range_angle < 0) *(outbuffer + offset -1) |= 0x80;
      *(outbuffer + offset + 0) = (this->sensor_view_direction >> (8 * 0)) & 0xFF;
      offset += sizeof(this->sensor_view_direction);
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
      uint32_t * val_target_radius = (uint32_t*) &(this->target_radius);
      offset += 3;
      *val_target_radius = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_target_radius |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_target_radius |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_target_radius |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_target_radius = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_target_radius |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_target_radius !=0)
        *val_target_radius |= ((exp_target_radius)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->target_radius = -this->target_radius;
      offset += this->target_pose.deserialize(inbuffer + offset);
      union {
        int32_t real;
        uint32_t base;
      } u_cone_sides;
      u_cone_sides.base = 0;
      u_cone_sides.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_cone_sides.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_cone_sides.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_cone_sides.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->cone_sides = u_cone_sides.real;
      offset += sizeof(this->cone_sides);
      offset += this->sensor_pose.deserialize(inbuffer + offset);
      uint32_t * val_max_view_angle = (uint32_t*) &(this->max_view_angle);
      offset += 3;
      *val_max_view_angle = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_max_view_angle |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_max_view_angle |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_max_view_angle |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_max_view_angle = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_max_view_angle |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_max_view_angle !=0)
        *val_max_view_angle |= ((exp_max_view_angle)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->max_view_angle = -this->max_view_angle;
      uint32_t * val_max_range_angle = (uint32_t*) &(this->max_range_angle);
      offset += 3;
      *val_max_range_angle = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_max_range_angle |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_max_range_angle |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_max_range_angle |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_max_range_angle = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_max_range_angle |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_max_range_angle !=0)
        *val_max_range_angle |= ((exp_max_range_angle)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->max_range_angle = -this->max_range_angle;
      this->sensor_view_direction =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->sensor_view_direction);
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

    const char * getType(){ return "moveit_msgs/VisibilityConstraint"; };
    const char * getMD5(){ return "62cda903bfe31ff2e5fcdc3810d577ad"; };

  };

}
#endif