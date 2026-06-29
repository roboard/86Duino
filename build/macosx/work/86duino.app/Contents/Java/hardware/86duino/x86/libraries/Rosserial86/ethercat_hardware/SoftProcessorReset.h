#ifndef _ROS_SERVICE_SoftProcessorReset_h
#define _ROS_SERVICE_SoftProcessorReset_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace ethercat_hardware
{

static const char SOFTPROCESSORRESET[] = "ethercat_hardware/SoftProcessorReset";

  class SoftProcessorResetRequest : public ros::Msg
  {
    public:
      char * actuator_name;
      char * processor_name;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_actuator_name = strlen( (const char*) this->actuator_name);
      memcpy(outbuffer + offset, &length_actuator_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->actuator_name, length_actuator_name);
      offset += length_actuator_name;
      uint32_t length_processor_name = strlen( (const char*) this->processor_name);
      memcpy(outbuffer + offset, &length_processor_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->processor_name, length_processor_name);
      offset += length_processor_name;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_actuator_name;
      memcpy(&length_actuator_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_actuator_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_actuator_name-1]=0;
      this->actuator_name = (char *)(inbuffer + offset-1);
      offset += length_actuator_name;
      uint32_t length_processor_name;
      memcpy(&length_processor_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_processor_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_processor_name-1]=0;
      this->processor_name = (char *)(inbuffer + offset-1);
      offset += length_processor_name;
     return offset;
    }

    const char * getType(){ return SOFTPROCESSORRESET; };
    const char * getMD5(){ return "777be25d71e9e85e62fa14223ffddb6b"; };

  };

  class SoftProcessorResetResponse : public ros::Msg
  {
    public:
      bool success;
      char * error_msg;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_success;
      u_success.real = this->success;
      *(outbuffer + offset + 0) = (u_success.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->success);
      uint32_t length_error_msg = strlen( (const char*) this->error_msg);
      memcpy(outbuffer + offset, &length_error_msg, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->error_msg, length_error_msg);
      offset += length_error_msg;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_success;
      u_success.base = 0;
      u_success.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->success = u_success.real;
      offset += sizeof(this->success);
      uint32_t length_error_msg;
      memcpy(&length_error_msg, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_error_msg; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_error_msg-1]=0;
      this->error_msg = (char *)(inbuffer + offset-1);
      offset += length_error_msg;
     return offset;
    }

    const char * getType(){ return SOFTPROCESSORRESET; };
    const char * getMD5(){ return "d006c48be24db1173a071ca9af4c8179"; };

  };

  class SoftProcessorReset {
    public:
    typedef SoftProcessorResetRequest Request;
    typedef SoftProcessorResetResponse Response;
  };

}
#endif
