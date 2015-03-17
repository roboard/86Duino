#ifndef _ROS_program_queue_Program_h
#define _ROS_program_queue_Program_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "program_queue/ProgramInfo.h"

namespace program_queue
{

  class Program : public ros::Msg
  {
    public:
      program_queue::ProgramInfo info;
      char * code;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->info.serialize(outbuffer + offset);
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
      offset += this->info.deserialize(inbuffer + offset);
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

    const char * getType(){ return "program_queue/Program"; };
    const char * getMD5(){ return "5220a14bbff3106292668d581215f326"; };

  };

}
#endif