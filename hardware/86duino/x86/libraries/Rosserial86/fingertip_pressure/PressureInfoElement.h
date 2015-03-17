#ifndef _ROS_fingertip_pressure_PressureInfoElement_h
#define _ROS_fingertip_pressure_PressureInfoElement_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Vector3.h"

namespace fingertip_pressure
{

  class PressureInfoElement : public ros::Msg
  {
    public:
      char * frame_id;
      uint8_t center_length;
      geometry_msgs::Vector3 st_center;
      geometry_msgs::Vector3 * center;
      uint8_t halfside1_length;
      geometry_msgs::Vector3 st_halfside1;
      geometry_msgs::Vector3 * halfside1;
      uint8_t halfside2_length;
      geometry_msgs::Vector3 st_halfside2;
      geometry_msgs::Vector3 * halfside2;
      uint8_t force_per_unit_length;
      float st_force_per_unit;
      float * force_per_unit;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_frame_id = strlen( (const char*) this->frame_id);
      memcpy(outbuffer + offset, &length_frame_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->frame_id, length_frame_id);
      offset += length_frame_id;
      *(outbuffer + offset++) = center_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < center_length; i++){
      offset += this->center[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = halfside1_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < halfside1_length; i++){
      offset += this->halfside1[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = halfside2_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < halfside2_length; i++){
      offset += this->halfside2[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = force_per_unit_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < force_per_unit_length; i++){
      int32_t * val_force_per_uniti = (int32_t *) &(this->force_per_unit[i]);
      int32_t exp_force_per_uniti = (((*val_force_per_uniti)>>23)&255);
      if(exp_force_per_uniti != 0)
        exp_force_per_uniti += 1023-127;
      int32_t sig_force_per_uniti = *val_force_per_uniti;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_force_per_uniti<<5) & 0xff;
      *(outbuffer + offset++) = (sig_force_per_uniti>>3) & 0xff;
      *(outbuffer + offset++) = (sig_force_per_uniti>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_force_per_uniti<<4) & 0xF0) | ((sig_force_per_uniti>>19)&0x0F);
      *(outbuffer + offset++) = (exp_force_per_uniti>>4) & 0x7F;
      if(this->force_per_unit[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_frame_id;
      memcpy(&length_frame_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_frame_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_frame_id-1]=0;
      this->frame_id = (char *)(inbuffer + offset-1);
      offset += length_frame_id;
      uint8_t center_lengthT = *(inbuffer + offset++);
      if(center_lengthT > center_length)
        this->center = (geometry_msgs::Vector3*)realloc(this->center, center_lengthT * sizeof(geometry_msgs::Vector3));
      offset += 3;
      center_length = center_lengthT;
      for( uint8_t i = 0; i < center_length; i++){
      offset += this->st_center.deserialize(inbuffer + offset);
        memcpy( &(this->center[i]), &(this->st_center), sizeof(geometry_msgs::Vector3));
      }
      uint8_t halfside1_lengthT = *(inbuffer + offset++);
      if(halfside1_lengthT > halfside1_length)
        this->halfside1 = (geometry_msgs::Vector3*)realloc(this->halfside1, halfside1_lengthT * sizeof(geometry_msgs::Vector3));
      offset += 3;
      halfside1_length = halfside1_lengthT;
      for( uint8_t i = 0; i < halfside1_length; i++){
      offset += this->st_halfside1.deserialize(inbuffer + offset);
        memcpy( &(this->halfside1[i]), &(this->st_halfside1), sizeof(geometry_msgs::Vector3));
      }
      uint8_t halfside2_lengthT = *(inbuffer + offset++);
      if(halfside2_lengthT > halfside2_length)
        this->halfside2 = (geometry_msgs::Vector3*)realloc(this->halfside2, halfside2_lengthT * sizeof(geometry_msgs::Vector3));
      offset += 3;
      halfside2_length = halfside2_lengthT;
      for( uint8_t i = 0; i < halfside2_length; i++){
      offset += this->st_halfside2.deserialize(inbuffer + offset);
        memcpy( &(this->halfside2[i]), &(this->st_halfside2), sizeof(geometry_msgs::Vector3));
      }
      uint8_t force_per_unit_lengthT = *(inbuffer + offset++);
      if(force_per_unit_lengthT > force_per_unit_length)
        this->force_per_unit = (float*)realloc(this->force_per_unit, force_per_unit_lengthT * sizeof(float));
      offset += 3;
      force_per_unit_length = force_per_unit_lengthT;
      for( uint8_t i = 0; i < force_per_unit_length; i++){
      uint32_t * val_st_force_per_unit = (uint32_t*) &(this->st_force_per_unit);
      offset += 3;
      *val_st_force_per_unit = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_force_per_unit |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_force_per_unit |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_force_per_unit |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_force_per_unit = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_force_per_unit |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_force_per_unit !=0)
        *val_st_force_per_unit |= ((exp_st_force_per_unit)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_force_per_unit = -this->st_force_per_unit;
        memcpy( &(this->force_per_unit[i]), &(this->st_force_per_unit), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "fingertip_pressure/PressureInfoElement"; };
    const char * getMD5(){ return "1cb486bb542ab85e1ff8d84fe9cc899f"; };

  };

}
#endif