#ifndef _ROS_SERVICE_GetMyPrograms_h
#define _ROS_SERVICE_GetMyPrograms_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "program_queue/ProgramInfo.h"

namespace program_queue
{

static const char GETMYPROGRAMS[] = "program_queue/GetMyPrograms";

  class GetMyProgramsRequest : public ros::Msg
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

    const char * getType(){ return GETMYPROGRAMS; };
    const char * getMD5(){ return "14c7152ddd08a9946aaadd642a3c327d"; };

  };

  class GetMyProgramsResponse : public ros::Msg
  {
    public:
      uint8_t programs_length;
      program_queue::ProgramInfo st_programs;
      program_queue::ProgramInfo * programs;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = programs_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < programs_length; i++){
      offset += this->programs[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t programs_lengthT = *(inbuffer + offset++);
      if(programs_lengthT > programs_length)
        this->programs = (program_queue::ProgramInfo*)realloc(this->programs, programs_lengthT * sizeof(program_queue::ProgramInfo));
      offset += 3;
      programs_length = programs_lengthT;
      for( uint8_t i = 0; i < programs_length; i++){
      offset += this->st_programs.deserialize(inbuffer + offset);
        memcpy( &(this->programs[i]), &(this->st_programs), sizeof(program_queue::ProgramInfo));
      }
     return offset;
    }

    const char * getType(){ return GETMYPROGRAMS; };
    const char * getMD5(){ return "dad5a81c3a97b95a95aa3a5efed712de"; };

  };

  class GetMyPrograms {
    public:
    typedef GetMyProgramsRequest Request;
    typedef GetMyProgramsResponse Response;
  };

}
#endif
