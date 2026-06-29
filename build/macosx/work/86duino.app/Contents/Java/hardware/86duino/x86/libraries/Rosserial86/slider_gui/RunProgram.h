#ifndef _ROS_SERVICE_RunProgram_h
#define _ROS_SERVICE_RunProgram_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace slider_gui
{

static const char RUNPROGRAM[] = "slider_gui/RunProgram";

  class RunProgramRequest : public ros::Msg
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

    const char * getType(){ return RUNPROGRAM; };
    const char * getMD5(){ return "9d589d39a46fa5aba4838a23b9cc4e62"; };

  };

  class RunProgramResponse : public ros::Msg
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

    const char * getType(){ return RUNPROGRAM; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class RunProgram {
    public:
    typedef RunProgramRequest Request;
    typedef RunProgramResponse Response;
  };

}
#endif
