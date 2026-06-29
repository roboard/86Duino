#ifndef _ROS_SERVICE_AddPublisher_h
#define _ROS_SERVICE_AddPublisher_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace topic_proxy
{

static const char ADDPUBLISHER[] = "topic_proxy/AddPublisher";

  class AddPublisherRequest : public ros::Msg
  {
    public:
      char * topic;
      char * remote_topic;
      bool compressed;
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

    const char * getType(){ return ADDPUBLISHER; };
    const char * getMD5(){ return "c8c0defbf1105280f031bc4a12f024e8"; };

  };

  class AddPublisherResponse : public ros::Msg
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

    const char * getType(){ return ADDPUBLISHER; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class AddPublisher {
    public:
    typedef AddPublisherRequest Request;
    typedef AddPublisherResponse Response;
  };

}
#endif
