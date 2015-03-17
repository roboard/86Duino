#ifndef _ROS_SERVICE_StringString_h
#define _ROS_SERVICE_StringString_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace roseus
{

static const char STRINGSTRING[] = "roseus/StringString";

  class StringStringRequest : public ros::Msg
  {
    public:
      char * str;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_str = strlen( (const char*) this->str);
      memcpy(outbuffer + offset, &length_str, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->str, length_str);
      offset += length_str;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_str;
      memcpy(&length_str, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_str; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_str-1]=0;
      this->str = (char *)(inbuffer + offset-1);
      offset += length_str;
     return offset;
    }

    const char * getType(){ return STRINGSTRING; };
    const char * getMD5(){ return "994972b6e03928b2476860ce6c4c8e17"; };

  };

  class StringStringResponse : public ros::Msg
  {
    public:
      char * str;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_str = strlen( (const char*) this->str);
      memcpy(outbuffer + offset, &length_str, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->str, length_str);
      offset += length_str;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_str;
      memcpy(&length_str, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_str; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_str-1]=0;
      this->str = (char *)(inbuffer + offset-1);
      offset += length_str;
     return offset;
    }

    const char * getType(){ return STRINGSTRING; };
    const char * getMD5(){ return "994972b6e03928b2476860ce6c4c8e17"; };

  };

  class StringString {
    public:
    typedef StringStringRequest Request;
    typedef StringStringResponse Response;
  };

}
#endif
