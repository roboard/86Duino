#ifndef _ROS_concert_msgs_LinkNode_h
#define _ROS_concert_msgs_LinkNode_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace concert_msgs
{

  class LinkNode : public ros::Msg
  {
    public:
      char * id;
      char * tuple;
      int8_t min;
      int8_t max;
      bool force_name_matching;
      enum { UNLIMITED_RESOURCE = -1 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_id = strlen( (const char*) this->id);
      memcpy(outbuffer + offset, &length_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->id, length_id);
      offset += length_id;
      uint32_t length_tuple = strlen( (const char*) this->tuple);
      memcpy(outbuffer + offset, &length_tuple, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->tuple, length_tuple);
      offset += length_tuple;
      union {
        int8_t real;
        uint8_t base;
      } u_min;
      u_min.real = this->min;
      *(outbuffer + offset + 0) = (u_min.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->min);
      union {
        int8_t real;
        uint8_t base;
      } u_max;
      u_max.real = this->max;
      *(outbuffer + offset + 0) = (u_max.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->max);
      union {
        bool real;
        uint8_t base;
      } u_force_name_matching;
      u_force_name_matching.real = this->force_name_matching;
      *(outbuffer + offset + 0) = (u_force_name_matching.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->force_name_matching);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_id;
      memcpy(&length_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_id-1]=0;
      this->id = (char *)(inbuffer + offset-1);
      offset += length_id;
      uint32_t length_tuple;
      memcpy(&length_tuple, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_tuple; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_tuple-1]=0;
      this->tuple = (char *)(inbuffer + offset-1);
      offset += length_tuple;
      union {
        int8_t real;
        uint8_t base;
      } u_min;
      u_min.base = 0;
      u_min.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->min = u_min.real;
      offset += sizeof(this->min);
      union {
        int8_t real;
        uint8_t base;
      } u_max;
      u_max.base = 0;
      u_max.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->max = u_max.real;
      offset += sizeof(this->max);
      union {
        bool real;
        uint8_t base;
      } u_force_name_matching;
      u_force_name_matching.base = 0;
      u_force_name_matching.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->force_name_matching = u_force_name_matching.real;
      offset += sizeof(this->force_name_matching);
     return offset;
    }

    const char * getType(){ return "concert_msgs/LinkNode"; };
    const char * getMD5(){ return "8cb53e2248706eb448c74babbdffc381"; };

  };

}
#endif