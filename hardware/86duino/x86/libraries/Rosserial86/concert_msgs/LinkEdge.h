#ifndef _ROS_concert_msgs_LinkEdge_h
#define _ROS_concert_msgs_LinkEdge_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace concert_msgs
{

  class LinkEdge : public ros::Msg
  {
    public:
      char * start;
      char * finish;
      char * remap_from;
      char * remap_to;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_start = strlen( (const char*) this->start);
      memcpy(outbuffer + offset, &length_start, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->start, length_start);
      offset += length_start;
      uint32_t length_finish = strlen( (const char*) this->finish);
      memcpy(outbuffer + offset, &length_finish, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->finish, length_finish);
      offset += length_finish;
      uint32_t length_remap_from = strlen( (const char*) this->remap_from);
      memcpy(outbuffer + offset, &length_remap_from, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->remap_from, length_remap_from);
      offset += length_remap_from;
      uint32_t length_remap_to = strlen( (const char*) this->remap_to);
      memcpy(outbuffer + offset, &length_remap_to, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->remap_to, length_remap_to);
      offset += length_remap_to;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_start;
      memcpy(&length_start, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_start; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_start-1]=0;
      this->start = (char *)(inbuffer + offset-1);
      offset += length_start;
      uint32_t length_finish;
      memcpy(&length_finish, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_finish; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_finish-1]=0;
      this->finish = (char *)(inbuffer + offset-1);
      offset += length_finish;
      uint32_t length_remap_from;
      memcpy(&length_remap_from, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_remap_from; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_remap_from-1]=0;
      this->remap_from = (char *)(inbuffer + offset-1);
      offset += length_remap_from;
      uint32_t length_remap_to;
      memcpy(&length_remap_to, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_remap_to; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_remap_to-1]=0;
      this->remap_to = (char *)(inbuffer + offset-1);
      offset += length_remap_to;
     return offset;
    }

    const char * getType(){ return "concert_msgs/LinkEdge"; };
    const char * getMD5(){ return "01f86ee3e51dd78d2e2153a94d6f3865"; };

  };

}
#endif