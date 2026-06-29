#ifndef _ROS_industrial_msgs_DeviceInfo_h
#define _ROS_industrial_msgs_DeviceInfo_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace industrial_msgs
{

  class DeviceInfo : public ros::Msg
  {
    public:
      char * model;
      char * serial_number;
      char * hw_version;
      char * sw_version;
      char * address;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_model = strlen( (const char*) this->model);
      memcpy(outbuffer + offset, &length_model, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->model, length_model);
      offset += length_model;
      uint32_t length_serial_number = strlen( (const char*) this->serial_number);
      memcpy(outbuffer + offset, &length_serial_number, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->serial_number, length_serial_number);
      offset += length_serial_number;
      uint32_t length_hw_version = strlen( (const char*) this->hw_version);
      memcpy(outbuffer + offset, &length_hw_version, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->hw_version, length_hw_version);
      offset += length_hw_version;
      uint32_t length_sw_version = strlen( (const char*) this->sw_version);
      memcpy(outbuffer + offset, &length_sw_version, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->sw_version, length_sw_version);
      offset += length_sw_version;
      uint32_t length_address = strlen( (const char*) this->address);
      memcpy(outbuffer + offset, &length_address, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->address, length_address);
      offset += length_address;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_model;
      memcpy(&length_model, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_model; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_model-1]=0;
      this->model = (char *)(inbuffer + offset-1);
      offset += length_model;
      uint32_t length_serial_number;
      memcpy(&length_serial_number, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_serial_number; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_serial_number-1]=0;
      this->serial_number = (char *)(inbuffer + offset-1);
      offset += length_serial_number;
      uint32_t length_hw_version;
      memcpy(&length_hw_version, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_hw_version; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_hw_version-1]=0;
      this->hw_version = (char *)(inbuffer + offset-1);
      offset += length_hw_version;
      uint32_t length_sw_version;
      memcpy(&length_sw_version, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_sw_version; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_sw_version-1]=0;
      this->sw_version = (char *)(inbuffer + offset-1);
      offset += length_sw_version;
      uint32_t length_address;
      memcpy(&length_address, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_address; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_address-1]=0;
      this->address = (char *)(inbuffer + offset-1);
      offset += length_address;
     return offset;
    }

    const char * getType(){ return "industrial_msgs/DeviceInfo"; };
    const char * getMD5(){ return "373ed7fa0fac92d443be9cd5198e80f0"; };

  };

}
#endif