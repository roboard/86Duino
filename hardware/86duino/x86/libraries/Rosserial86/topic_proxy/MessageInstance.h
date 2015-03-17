#ifndef _ROS_topic_proxy_MessageInstance_h
#define _ROS_topic_proxy_MessageInstance_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "blob/Blob.h"

namespace topic_proxy
{

  class MessageInstance : public ros::Msg
  {
    public:
      char * topic;
      char * md5sum;
      char * type;
      char * message_definition;
      blob::Blob blob;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_topic = strlen( (const char*) this->topic);
      memcpy(outbuffer + offset, &length_topic, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->topic, length_topic);
      offset += length_topic;
      uint32_t length_md5sum = strlen( (const char*) this->md5sum);
      memcpy(outbuffer + offset, &length_md5sum, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->md5sum, length_md5sum);
      offset += length_md5sum;
      uint32_t length_type = strlen( (const char*) this->type);
      memcpy(outbuffer + offset, &length_type, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->type, length_type);
      offset += length_type;
      uint32_t length_message_definition = strlen( (const char*) this->message_definition);
      memcpy(outbuffer + offset, &length_message_definition, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->message_definition, length_message_definition);
      offset += length_message_definition;
      offset += this->blob.serialize(outbuffer + offset);
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
      uint32_t length_md5sum;
      memcpy(&length_md5sum, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_md5sum; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_md5sum-1]=0;
      this->md5sum = (char *)(inbuffer + offset-1);
      offset += length_md5sum;
      uint32_t length_type;
      memcpy(&length_type, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_type; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_type-1]=0;
      this->type = (char *)(inbuffer + offset-1);
      offset += length_type;
      uint32_t length_message_definition;
      memcpy(&length_message_definition, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_message_definition; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_message_definition-1]=0;
      this->message_definition = (char *)(inbuffer + offset-1);
      offset += length_message_definition;
      offset += this->blob.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "topic_proxy/MessageInstance"; };
    const char * getMD5(){ return "69465ef2b8f5727b1913d1e3e2ad35bd"; };

  };

}
#endif