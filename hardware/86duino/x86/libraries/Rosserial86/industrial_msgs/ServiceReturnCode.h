#ifndef _ROS_industrial_msgs_ServiceReturnCode_h
#define _ROS_industrial_msgs_ServiceReturnCode_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace industrial_msgs
{

  class ServiceReturnCode : public ros::Msg
  {
    public:
      int8_t val;
      enum { SUCCESS =  1 };
      enum { FAILURE =  -1 };

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

    const char * getType(){ return "industrial_msgs/ServiceReturnCode"; };
    const char * getMD5(){ return "85a4e241266be66b1e1426b03083a412"; };

  };

}
#endif