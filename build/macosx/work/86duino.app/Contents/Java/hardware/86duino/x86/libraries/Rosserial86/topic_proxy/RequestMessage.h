#ifndef _ROS_SERVICE_RequestMessage_h
#define _ROS_SERVICE_RequestMessage_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "ros/duration.h"

namespace topic_proxy
{

static const char REQUESTMESSAGE[] = "topic_proxy/RequestMessage";

  class RequestMessageRequest : public ros::Msg
  {
    public:
      char * topic;
      char * remote_topic;
      bool compressed;
      ros::Duration timeout;
      ros::Duration interval;
      bool latch;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_topic = strlen( (const char*) this->topic);
      memcpy(outbuffer + offset, &length_topic, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->topic, length_topic);
      offset += length_topic;
      uint32_t length_remote_topic = strlen( (const char*) this->remote_topic);
      memcpy(outbuffer + offset, &length_remote_topic, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->remote_topic, length_remote_topic);
      offset += length_remote_topic;
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
      *(outbuffer + offset + 0) = (this->interval.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->interval.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->interval.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->interval.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->interval.sec);
      *(outbuffer + offset + 0) = (this->interval.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->interval.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->interval.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->interval.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->interval.nsec);
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
      uint32_t length_topic;
      memcpy(&length_topic, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_topic; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_topic-1]=0;
      this->topic = (char *)(inbuffer + offset-1);
      offset += length_topic;
      uint32_t length_remote_topic;
      memcpy(&length_remote_topic, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_remote_topic; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_remote_topic-1]=0;
      this->remote_topic = (char *)(inbuffer + offset-1);
      offset += length_remote_topic;
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
      this->interval.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->interval.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->interval.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->interval.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->interval.sec);
      this->interval.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->interval.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->interval.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->interval.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->interval.nsec);
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

    const char * getType(){ return REQUESTMESSAGE; };
    const char * getMD5(){ return "a9efb95582ffc17bbc0b4de0200e72d0"; };

  };

  class RequestMessageResponse : public ros::Msg
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

    const char * getType(){ return REQUESTMESSAGE; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class RequestMessage {
    public:
    typedef RequestMessageRequest Request;
    typedef RequestMessageResponse Response;
  };

}
#endif
