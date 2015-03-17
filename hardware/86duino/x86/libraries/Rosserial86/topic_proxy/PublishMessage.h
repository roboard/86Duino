#ifndef _ROS_SERVICE_PublishMessage_h
#define _ROS_SERVICE_PublishMessage_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "topic_proxy/MessageInstance.h"

namespace topic_proxy
{

static const char PUBLISHMESSAGE[] = "topic_proxy/PublishMessage";

  class PublishMessageRequest : public ros::Msg
  {
    public:
      topic_proxy::MessageInstance message;
      bool latch;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->message.serialize(outbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_latch;
      u_latch.real = this->latch;
      *(outbuffer + offset + 0) = (u_latch.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->latch);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->message.deserialize(inbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_latch;
      u_latch.base = 0;
      u_latch.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->latch = u_latch.real;
      offset += sizeof(this->latch);
     return offset;
    }

    const char * getType(){ return PUBLISHMESSAGE; };
    const char * getMD5(){ return "0bfb78644634f93633d70b6a4a05176c"; };

  };

  class PublishMessageResponse : public ros::Msg
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

    const char * getType(){ return PUBLISHMESSAGE; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class PublishMessage {
    public:
    typedef PublishMessageRequest Request;
    typedef PublishMessageResponse Response;
  };

}
#endif
