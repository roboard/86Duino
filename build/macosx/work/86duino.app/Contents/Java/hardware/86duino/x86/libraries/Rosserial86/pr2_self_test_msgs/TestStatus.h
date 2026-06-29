#ifndef _ROS_pr2_self_test_msgs_TestStatus_h
#define _ROS_pr2_self_test_msgs_TestStatus_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_self_test_msgs
{

  class TestStatus : public ros::Msg
  {
    public:
      int8_t test_ok;
      char * message;
      enum { TEST_RUNNING =  0 };
      enum { TEST_WARNING =  1 };
      enum { TEST_ERROR =  2 };
      enum { TEST_STALE =  3 };
      enum { RUNNING =  0 };
      enum { WARNING =  1 };
      enum { ERROR =  2 };
      enum { STALE =  3 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int8_t real;
        uint8_t base;
      } u_test_ok;
      u_test_ok.real = this->test_ok;
      *(outbuffer + offset + 0) = (u_test_ok.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->test_ok);
      uint32_t length_message = strlen( (const char*) this->message);
      memcpy(outbuffer + offset, &length_message, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->message, length_message);
      offset += length_message;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int8_t real;
        uint8_t base;
      } u_test_ok;
      u_test_ok.base = 0;
      u_test_ok.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->test_ok = u_test_ok.real;
      offset += sizeof(this->test_ok);
      uint32_t length_message;
      memcpy(&length_message, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_message; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_message-1]=0;
      this->message = (char *)(inbuffer + offset-1);
      offset += length_message;
     return offset;
    }

    const char * getType(){ return "pr2_self_test_msgs/TestStatus"; };
    const char * getMD5(){ return "b3baedd2ad2ef574d577434a0165fb15"; };

  };

}
#endif