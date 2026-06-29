#ifndef _ROS_SERVICE_SoftProcessorFirmwareWrite_h
#define _ROS_SERVICE_SoftProcessorFirmwareWrite_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace ethercat_hardware
{

static const char SOFTPROCESSORFIRMWAREWRITE[] = "ethercat_hardware/SoftProcessorFirmwareWrite";

  class SoftProcessorFirmwareWriteRequest : public ros::Msg
  {
    public:
      char * actuator_name;
      char * processor_name;
      uint8_t instructions_length;
      uint32_t st_instructions;
      uint32_t * instructions;

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
      *(outbuffer + offset++) = instructions_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < instructions_length; i++){
      *(outbuffer + offset + 0) = (this->instructions[i] >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->instructions[i] >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->instructions[i] >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->instructions[i] >> (8 * 3)) & 0xFF;
      offset += sizeof(this->instructions[i]);
      }
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
      uint8_t instructions_lengthT = *(inbuffer + offset++);
      if(instructions_lengthT > instructions_length)
        this->instructions = (uint32_t*)realloc(this->instructions, instructions_lengthT * sizeof(uint32_t));
      offset += 3;
      instructions_length = instructions_lengthT;
      for( uint8_t i = 0; i < instructions_length; i++){
      this->st_instructions =  ((uint32_t) (*(inbuffer + offset)));
      this->st_instructions |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->st_instructions |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->st_instructions |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->st_instructions);
        memcpy( &(this->instructions[i]), &(this->st_instructions), sizeof(uint32_t));
      }
     return offset;
    }

    const char * getType(){ return SOFTPROCESSORFIRMWAREWRITE; };
    const char * getMD5(){ return "5234f035d5f911e880df479ac901a6e0"; };

  };

  class SoftProcessorFirmwareWriteResponse : public ros::Msg
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

    const char * getType(){ return SOFTPROCESSORFIRMWAREWRITE; };
    const char * getMD5(){ return "d006c48be24db1173a071ca9af4c8179"; };

  };

  class SoftProcessorFirmwareWrite {
    public:
    typedef SoftProcessorFirmwareWriteRequest Request;
    typedef SoftProcessorFirmwareWriteResponse Response;
  };

}
#endif
