#ifndef _ROS_SERVICE_GetProgram_h
#define _ROS_SERVICE_GetProgram_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "program_queue/Program.h"

namespace program_queue
{

static const char GETPROGRAM[] = "program_queue/GetProgram";

  class GetProgramRequest : public ros::Msg
  {
    public:
      uint64_t id;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        uint64_t real;
        uint32_t base;
      } u_id;
      u_id.real = this->id;
      *(outbuffer + offset + 0) = (u_id.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_id.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_id.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_id.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->id);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        uint64_t real;
        uint32_t base;
      } u_id;
      u_id.base = 0;
      u_id.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_id.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_id.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_id.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->id = u_id.real;
      offset += sizeof(this->id);
     return offset;
    }

    const char * getType(){ return GETPROGRAM; };
    const char * getMD5(){ return "0dba762d409c9ac12516987f0927c77c"; };

  };

  class GetProgramResponse : public ros::Msg
  {
    public:
      program_queue::Program program;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->program.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->program.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return GETPROGRAM; };
    const char * getMD5(){ return "02a43aeaed6b50d9c07b0ac986e2e8c3"; };

  };

  class GetProgram {
    public:
    typedef GetProgramRequest Request;
    typedef GetProgramResponse Response;
  };

}
#endif
