#ifndef _ROS_SERVICE_CreateProgram_h
#define _ROS_SERVICE_CreateProgram_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace program_queue
{

static const char CREATEPROGRAM[] = "program_queue/CreateProgram";

  class CreateProgramRequest : public ros::Msg
  {
    public:
      uint64_t token;

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
     return offset;
    }

    const char * getType(){ return CREATEPROGRAM; };
    const char * getMD5(){ return "14c7152ddd08a9946aaadd642a3c327d"; };

  };

  class CreateProgramResponse : public ros::Msg
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

    const char * getType(){ return CREATEPROGRAM; };
    const char * getMD5(){ return "0dba762d409c9ac12516987f0927c77c"; };

  };

  class CreateProgram {
    public:
    typedef CreateProgramRequest Request;
    typedef CreateProgramResponse Response;
  };

}
#endif
