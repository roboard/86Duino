#ifndef _ROS_SERVICE_UpdateProgram_h
#define _ROS_SERVICE_UpdateProgram_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "program_queue/Program.h"

namespace program_queue
{

static const char UPDATEPROGRAM[] = "program_queue/UpdateProgram";

  class UpdateProgramRequest : public ros::Msg
  {
    public:
      uint64_t token;
      program_queue::Program program;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        uint64_t real;
        uint32_t base;
      } u_token;
      u_token.real = this->token;
      *(outbuffer + offset + 0) = (u_token.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_token.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_token.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_token.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->token);
      offset += this->program.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        uint64_t real;
        uint32_t base;
      } u_token;
      u_token.base = 0;
      u_token.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_token.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_token.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_token.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->token = u_token.real;
      offset += sizeof(this->token);
      offset += this->program.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return UPDATEPROGRAM; };
    const char * getMD5(){ return "95a7f594407f333113b2a1fe09071e8c"; };

  };

  class UpdateProgramResponse : public ros::Msg
  {
    public:

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
     return offset;
    }

    const char * getType(){ return UPDATEPROGRAM; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class UpdateProgram {
    public:
    typedef UpdateProgramRequest Request;
    typedef UpdateProgramResponse Response;
  };

}
#endif
