#ifndef _ROS_blob_Blob_h
#define _ROS_blob_Blob_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace blob
{

  class Blob : public ros::Msg
  {
    public:
      bool compressed;
      uint8_t data_length;
      uint8_t st_data;
      uint8_t * data;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_compressed;
      u_compressed.real = this->compressed;
      *(outbuffer + offset + 0) = (u_compressed.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->compressed);
      *(outbuffer + offset++) = data_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < data_length; i++){
      *(outbuffer + offset + 0) = (this->data[i] >> (8 * 0)) & 0xFF;
      offset += sizeof(this->data[i]);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_compressed;
      u_compressed.base = 0;
      u_compressed.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->compressed = u_compressed.real;
      offset += sizeof(this->compressed);
      uint8_t data_lengthT = *(inbuffer + offset++);
      if(data_lengthT > data_length)
        this->data = (uint8_t*)realloc(this->data, data_lengthT * sizeof(uint8_t));
      offset += 3;
      data_length = data_lengthT;
      for( uint8_t i = 0; i < data_length; i++){
      this->st_data =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->st_data);
        memcpy( &(this->data[i]), &(this->st_data), sizeof(uint8_t));
      }
     return offset;
    }

    const char * getType(){ return "blob/Blob"; };
    const char * getMD5(){ return "8115c3ed9d7b2e23c47c6ecaff2d4b13"; };

  };

}
#endif