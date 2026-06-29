#ifndef _ROS_SERVICE_GetMessage_h
#define _ROS_SERVICE_GetMessage_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "ros/duration.h"
#include "topic_proxy/MessageInstance.h"

namespace topic_proxy
{

static const char GETMESSAGE[] = "topic_proxy/GetMessage";

  class GetMessageRequest : public ros::Msg
  {
    public:
      char * topic;
      bool compressed;
      ros::Duration timeout;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_topic = strlen( (const char*) this->topic);
      memcpy(outbuffer + offset, &length_topic, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->topic, length_topic);
      offset += length_topic;
      union {
        bool real;
        uint8_t base;
      } u_compressed;
      u_compressed.real = this->compressed;
      *(outbuffer + offset + 0) = (u_compressed.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->compressed);
      *(outbuffer + offset + 0) = (this->timeout.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->timeout.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->timeout.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->timeout.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->timeout.sec);
      *(outbuffer + offset + 0) = (this->timeout.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->timeout.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->timeout.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->timeout.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->timeout.nsec);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_topic;
      memcpy(&length_topic, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_topic; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_topic-1]=0;
      this->topic = (char *)(inbuffer + offset-1);
      offset += length_topic;
      union {
        bool real;
        uint8_t base;
      } u_compressed;
      u_compressed.base = 0;
      u_compressed.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->compressed = u_compressed.real;
      offset += sizeof(this->compressed);
      this->timeout.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->timeout.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->timeout.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->timeout.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->timeout.sec);
      this->timeout.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->timeout.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->timeout.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->timeout.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->timeout.nsec);
     return offset;
    }

    const char * getType(){ return GETMESSAGE; };
    const char * getMD5(){ return "d58786c4419ea2772a24d9424036fbec"; };

  };

  class GetMessageResponse : public ros::Msg
  {
    public:
      topic_proxy::MessageInstance message;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->message.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->message.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return GETMESSAGE; };
    const char * getMD5(){ return "8d90d5a41b9999d61bdf79869246e250"; };

  };

  class GetMessage {
    public:
    typedef GetMessageRequest Request;
    typedef GetMessageResponse Response;
  };

}
#endif
