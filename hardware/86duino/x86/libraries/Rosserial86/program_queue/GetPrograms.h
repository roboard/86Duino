#ifndef _ROS_SERVICE_GetPrograms_h
#define _ROS_SERVICE_GetPrograms_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "program_queue/ProgramInfo.h"

namespace program_queue
{

static const char GETPROGRAMS[] = "program_queue/GetPrograms";

  class GetProgramsRequest : public ros::Msg
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

    const char * getType(){ return GETPROGRAMS; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class GetProgramsResponse : public ros::Msg
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

    const char * getType(){ return GETPROGRAMS; };
    const char * getMD5(){ return "dad5a81c3a97b95a95aa3a5efed712de"; };

  };

  class GetPrograms {
    public:
    typedef GetProgramsRequest Request;
    typedef GetProgramsResponse Response;
  };

}
#endif
