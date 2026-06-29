#ifndef _ROS_ethercat_hardware_RawFTDataSample_h
#define _ROS_ethercat_hardware_RawFTDataSample_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace ethercat_hardware
{

  class RawFTDataSample : public ros::Msg
  {
    public:
      uint64_t sample_count;
      uint8_t data_length;
      int16_t st_data;
      int16_t * data;
      uint16_t vhalf;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        uint64_t real;
        uint32_t base;
      } u_sample_count;
      u_sample_count.real = this->sample_count;
      *(outbuffer + offset + 0) = (u_sample_count.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_sample_count.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_sample_count.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_sample_count.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->sample_count);
      *(outbuffer + offset++) = data_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < data_length; i++){
      union {
        int16_t real;
        uint16_t base;
      } u_datai;
      u_datai.real = this->data[i];
      *(outbuffer + offset + 0) = (u_datai.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_datai.base >> (8 * 1)) & 0xFF;
      offset += sizeof(this->data[i]);
      }
      *(outbuffer + offset + 0) = (this->vhalf >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->vhalf >> (8 * 1)) & 0xFF;
      offset += sizeof(this->vhalf);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        uint64_t real;
        uint32_t base;
      } u_sample_count;
      u_sample_count.base = 0;
      u_sample_count.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_sample_count.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_sample_count.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_sample_count.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->sample_count = u_sample_count.real;
      offset += sizeof(this->sample_count);
      uint8_t data_lengthT = *(inbuffer + offset++);
      if(data_lengthT > data_length)
        this->data = (int16_t*)realloc(this->data, data_lengthT * sizeof(int16_t));
      offset += 3;
      data_length = data_lengthT;
      for( uint8_t i = 0; i < data_length; i++){
      union {
        int16_t real;
        uint16_t base;
      } u_st_data;
      u_st_data.base = 0;
      u_st_data.base |= ((uint16_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_data.base |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->st_data = u_st_data.real;
      offset += sizeof(this->st_data);
        memcpy( &(this->data[i]), &(this->st_data), sizeof(int16_t));
      }
      this->vhalf =  ((uint16_t) (*(inbuffer + offset)));
      this->vhalf |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      offset += sizeof(this->vhalf);
     return offset;
    }

    const char * getType(){ return "ethercat_hardware/RawFTDataSample"; };
    const char * getMD5(){ return "6c3b6e352fd24802b2d95b606df80de6"; };

  };

}
#endif