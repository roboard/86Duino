#ifndef _ROS_SERVICE_CallProgram_h
#define _ROS_SERVICE_CallProgram_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace program_queue
{

static const char CALLPROGRAM[] = "program_queue/CallProgram";

  class CallProgramRequest : public ros::Msg
  {
    public:
      char * code;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_code = strlen( (const char*) this->code);
      memcpy(outbuffer + offset, &length_code, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->code, length_code);
      offset += length_code;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_code;
      memcpy(&length_code, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_code; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_code-1]=0;
      this->code = (char *)(inbuffer + offset-1);
      offset += length_code;
     return offset;
    }

    const char * getType(){ return CALLPROGRAM; };
    const char * getMD5(){ return "9d589d39a46fa5aba4838a23b9cc4e62"; };

  };

  class CallProgramResponse : public ros::Msg
  {
    public:
      int8_t ret;
      char * output;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int8_t real;
        uint8_t base;
      } u_ret;
      u_ret.real = this->ret;
      *(outbuffer + offset + 0) = (u_ret.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->ret);
      uint32_t length_output = strlen( (const char*) this->output);
      memcpy(outbuffer + offset, &length_output, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->output, length_output);
      offset += length_output;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int8_t real;
        uint8_t base;
      } u_ret;
      u_ret.base = 0;
      u_ret.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->ret = u_ret.real;
      offset += sizeof(this->ret);
      uint32_t length_output;
      memcpy(&length_output, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_output; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_output-1]=0;
      this->output = (char *)(inbuffer + offset-1);
      offset += length_output;
     return offset;
    }

    const char * getType(){ return CALLPROGRAM; };
    const char * getMD5(){ return "2eff6845394abf92e7d666096639cded"; };

  };

  class CallProgram {
    public:
    typedef CallProgramRequest Request;
    typedef CallProgramResponse Response;
  };

}
#endif
