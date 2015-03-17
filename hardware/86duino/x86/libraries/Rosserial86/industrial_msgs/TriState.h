#ifndef _ROS_industrial_msgs_TriState_h
#define _ROS_industrial_msgs_TriState_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace industrial_msgs
{

  class TriState : public ros::Msg
  {
    public:
      int8_t val;
      enum { UNKNOWN = -1 };
      enum { TRUE = 1 };
      enum { ON = 1 };
      enum { ENABLED = 1 };
      enum { HIGH = 1 };
      enum { CLOSED = 1 };
      enum { FALSE = 0 };
      enum { OFF = 0 };
      enum { DISABLED = 0 };
      enum { LOW = 0 };
      enum { OPEN = 0 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int8_t real;
        uint8_t base;
      } u_val;
      u_val.real = this->val;
      *(outbuffer + offset + 0) = (u_val.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->val);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int8_t real;
        uint8_t base;
      } u_val;
      u_val.base = 0;
      u_val.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->val = u_val.real;
      offset += sizeof(this->val);
     return offset;
    }

    const char * getType(){ return "industrial_msgs/TriState"; };
    const char * getMD5(){ return "deb03829f3c2d0f1b647fa38d7087952"; };

  };

}
#endif