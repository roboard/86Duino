#ifndef _ROS_industrial_msgs_RobotMode_h
#define _ROS_industrial_msgs_RobotMode_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace industrial_msgs
{

  class RobotMode : public ros::Msg
  {
    public:
      int8_t val;
      enum { UNKNOWN = -1                  };
      enum { MANUAL = 1 			  };
      enum { AUTO = 2                      };

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

    const char * getType(){ return "industrial_msgs/RobotMode"; };
    const char * getMD5(){ return "24ac279e988b6b3db836e437e6ed1ba0"; };

  };

}
#endif