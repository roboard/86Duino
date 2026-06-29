#ifndef _ROS_octomap_msgs_Octomap_h
#define _ROS_octomap_msgs_Octomap_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"

namespace octomap_msgs
{

  class Octomap : public ros::Msg
  {
    public:
      std_msgs::Header header;
      bool binary;
      char * id;
      float resolution;
      uint8_t data_length;
      int8_t st_data;
      int8_t * data;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_binary;
      u_binary.real = this->binary;
      *(outbuffer + offset + 0) = (u_binary.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->binary);
      uint32_t length_id = strlen( (const char*) this->id);
      memcpy(outbuffer + offset, &length_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->id, length_id);
      offset += length_id;
      int32_t * val_resolution = (int32_t *) &(this->resolution);
      int32_t exp_resolution = (((*val_resolution)>>23)&255);
      if(exp_resolution != 0)
        exp_resolution += 1023-127;
      int32_t sig_resolution = *val_resolution;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_resolution<<5) & 0xff;
      *(outbuffer + offset++) = (sig_resolution>>3) & 0xff;
      *(outbuffer + offset++) = (sig_resolution>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_resolution<<4) & 0xF0) | ((sig_resolution>>19)&0x0F);
      *(outbuffer + offset++) = (exp_resolution>>4) & 0x7F;
      if(this->resolution < 0) *(outbuffer + offset -1) |= 0x80;
      *(outbuffer + offset++) = data_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < data_length; i++){
      union {
        int8_t real;
        uint8_t base;
      } u_datai;
      u_datai.real = this->data[i];
      *(outbuffer + offset + 0) = (u_datai.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->data[i]);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_binary;
      u_binary.base = 0;
      u_binary.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->binary = u_binary.real;
      offset += sizeof(this->binary);
      uint32_t length_id;
      memcpy(&length_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_id-1]=0;
      this->id = (char *)(inbuffer + offset-1);
      offset += length_id;
      uint32_t * val_resolution = (uint32_t*) &(this->resolution);
      offset += 3;
      *val_resolution = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_resolution |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_resolution |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_resolution |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_resolution = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_resolution |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_resolution !=0)
        *val_resolution |= ((exp_resolution)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->resolution = -this->resolution;
      uint8_t data_lengthT = *(inbuffer + offset++);
      if(data_lengthT > data_length)
        this->data = (int8_t*)realloc(this->data, data_lengthT * sizeof(int8_t));
      offset += 3;
      data_length = data_lengthT;
      for( uint8_t i = 0; i < data_length; i++){
      union {
        int8_t real;
        uint8_t base;
      } u_st_data;
      u_st_data.base = 0;
      u_st_data.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->st_data = u_st_data.real;
      offset += sizeof(this->st_data);
        memcpy( &(this->data[i]), &(this->st_data), sizeof(int8_t));
      }
     return offset;
    }

    const char * getType(){ return "octomap_msgs/Octomap"; };
    const char * getMD5(){ return "9a45536b45c5e409cd49f04bb2d9999f"; };

  };

}
#endif