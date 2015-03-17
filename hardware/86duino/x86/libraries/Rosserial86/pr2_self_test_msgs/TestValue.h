#ifndef _ROS_pr2_self_test_msgs_TestValue_h
#define _ROS_pr2_self_test_msgs_TestValue_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_self_test_msgs
{

  class TestValue : public ros::Msg
  {
    public:
      char * key;
      char * value;
      char * min;
      char * max;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_key = strlen( (const char*) this->key);
      memcpy(outbuffer + offset, &length_key, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->key, length_key);
      offset += length_key;
      uint32_t length_value = strlen( (const char*) this->value);
      memcpy(outbuffer + offset, &length_value, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->value, length_value);
      offset += length_value;
      uint32_t length_min = strlen( (const char*) this->min);
      memcpy(outbuffer + offset, &length_min, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->min, length_min);
      offset += length_min;
      uint32_t length_max = strlen( (const char*) this->max);
      memcpy(outbuffer + offset, &length_max, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->max, length_max);
      offset += length_max;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_key;
      memcpy(&length_key, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_key; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_key-1]=0;
      this->key = (char *)(inbuffer + offset-1);
      offset += length_key;
      uint32_t length_value;
      memcpy(&length_value, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_value; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_value-1]=0;
      this->value = (char *)(inbuffer + offset-1);
      offset += length_value;
      uint32_t length_min;
      memcpy(&length_min, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_min; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_min-1]=0;
      this->min = (char *)(inbuffer + offset-1);
      offset += length_min;
      uint32_t length_max;
      memcpy(&length_max, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_max; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_max-1]=0;
      this->max = (char *)(inbuffer + offset-1);
      offset += length_max;
     return offset;
    }

    const char * getType(){ return "pr2_self_test_msgs/TestValue"; };
    const char * getMD5(){ return "8fffe15af3a2ec4c24d3cf323fdfe721"; };

  };

}
#endif