#ifndef _ROS_moveit_msgs_OrientationConstraint_h
#define _ROS_moveit_msgs_OrientationConstraint_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "geometry_msgs/Quaternion.h"

namespace moveit_msgs
{

  class OrientationConstraint : public ros::Msg
  {
    public:
      std_msgs::Header header;
      geometry_msgs::Quaternion orientation;
      char * link_name;
      float absolute_x_axis_tolerance;
      float absolute_y_axis_tolerance;
      float absolute_z_axis_tolerance;
      float weight;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->orientation.serialize(outbuffer + offset);
      uint32_t length_link_name = strlen( (const char*) this->link_name);
      memcpy(outbuffer + offset, &length_link_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->link_name, length_link_name);
      offset += length_link_name;
      int32_t * val_absolute_x_axis_tolerance = (int32_t *) &(this->absolute_x_axis_tolerance);
      int32_t exp_absolute_x_axis_tolerance = (((*val_absolute_x_axis_tolerance)>>23)&255);
      if(exp_absolute_x_axis_tolerance != 0)
        exp_absolute_x_axis_tolerance += 1023-127;
      int32_t sig_absolute_x_axis_tolerance = *val_absolute_x_axis_tolerance;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_absolute_x_axis_tolerance<<5) & 0xff;
      *(outbuffer + offset++) = (sig_absolute_x_axis_tolerance>>3) & 0xff;
      *(outbuffer + offset++) = (sig_absolute_x_axis_tolerance>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_absolute_x_axis_tolerance<<4) & 0xF0) | ((sig_absolute_x_axis_tolerance>>19)&0x0F);
      *(outbuffer + offset++) = (exp_absolute_x_axis_tolerance>>4) & 0x7F;
      if(this->absolute_x_axis_tolerance < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_absolute_y_axis_tolerance = (int32_t *) &(this->absolute_y_axis_tolerance);
      int32_t exp_absolute_y_axis_tolerance = (((*val_absolute_y_axis_tolerance)>>23)&255);
      if(exp_absolute_y_axis_tolerance != 0)
        exp_absolute_y_axis_tolerance += 1023-127;
      int32_t sig_absolute_y_axis_tolerance = *val_absolute_y_axis_tolerance;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_absolute_y_axis_tolerance<<5) & 0xff;
      *(outbuffer + offset++) = (sig_absolute_y_axis_tolerance>>3) & 0xff;
      *(outbuffer + offset++) = (sig_absolute_y_axis_tolerance>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_absolute_y_axis_tolerance<<4) & 0xF0) | ((sig_absolute_y_axis_tolerance>>19)&0x0F);
      *(outbuffer + offset++) = (exp_absolute_y_axis_tolerance>>4) & 0x7F;
      if(this->absolute_y_axis_tolerance < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_absolute_z_axis_tolerance = (int32_t *) &(this->absolute_z_axis_tolerance);
      int32_t exp_absolute_z_axis_tolerance = (((*val_absolute_z_axis_tolerance)>>23)&255);
      if(exp_absolute_z_axis_tolerance != 0)
        exp_absolute_z_axis_tolerance += 1023-127;
      int32_t sig_absolute_z_axis_tolerance = *val_absolute_z_axis_tolerance;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_absolute_z_axis_tolerance<<5) & 0xff;
      *(outbuffer + offset++) = (sig_absolute_z_axis_tolerance>>3) & 0xff;
      *(outbuffer + offset++) = (sig_absolute_z_axis_tolerance>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_absolute_z_axis_tolerance<<4) & 0xF0) | ((sig_absolute_z_axis_tolerance>>19)&0x0F);
      *(outbuffer + offset++) = (exp_absolute_z_axis_tolerance>>4) & 0x7F;
      if(this->absolute_z_axis_tolerance < 0) *(outbuffer + offset -1) |= 0x80;
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
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->orientation.deserialize(inbuffer + offset);
      uint32_t length_link_name;
      memcpy(&length_link_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_link_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_link_name-1]=0;
      this->link_name = (char *)(inbuffer + offset-1);
      offset += length_link_name;
      uint32_t * val_absolute_x_axis_tolerance = (uint32_t*) &(this->absolute_x_axis_tolerance);
      offset += 3;
      *val_absolute_x_axis_tolerance = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_absolute_x_axis_tolerance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_absolute_x_axis_tolerance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_absolute_x_axis_tolerance |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_absolute_x_axis_tolerance = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_absolute_x_axis_tolerance |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_absolute_x_axis_tolerance !=0)
        *val_absolute_x_axis_tolerance |= ((exp_absolute_x_axis_tolerance)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->absolute_x_axis_tolerance = -this->absolute_x_axis_tolerance;
      uint32_t * val_absolute_y_axis_tolerance = (uint32_t*) &(this->absolute_y_axis_tolerance);
      offset += 3;
      *val_absolute_y_axis_tolerance = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_absolute_y_axis_tolerance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_absolute_y_axis_tolerance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_absolute_y_axis_tolerance |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_absolute_y_axis_tolerance = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_absolute_y_axis_tolerance |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_absolute_y_axis_tolerance !=0)
        *val_absolute_y_axis_tolerance |= ((exp_absolute_y_axis_tolerance)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->absolute_y_axis_tolerance = -this->absolute_y_axis_tolerance;
      uint32_t * val_absolute_z_axis_tolerance = (uint32_t*) &(this->absolute_z_axis_tolerance);
      offset += 3;
      *val_absolute_z_axis_tolerance = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_absolute_z_axis_tolerance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_absolute_z_axis_tolerance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_absolute_z_axis_tolerance |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_absolute_z_axis_tolerance = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_absolute_z_axis_tolerance |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_absolute_z_axis_tolerance !=0)
        *val_absolute_z_axis_tolerance |= ((exp_absolute_z_axis_tolerance)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->absolute_z_axis_tolerance = -this->absolute_z_axis_tolerance;
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

    const char * getType(){ return "moveit_msgs/OrientationConstraint"; };
    const char * getMD5(){ return "ab5cefb9bc4c0089620f5eb4caf4e59a"; };

  };

}
#endif