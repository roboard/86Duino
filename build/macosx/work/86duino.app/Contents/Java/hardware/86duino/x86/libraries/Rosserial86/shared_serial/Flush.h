#ifndef _ROS_shared_serial_Flush_h
#define _ROS_shared_serial_Flush_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace shared_serial
{

  class Flush : public ros::Msg
  {
    public:
      uint32_t socket;
      float timeout;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->socket >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->socket >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->socket >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->socket >> (8 * 3)) & 0xFF;
      offset += sizeof(this->socket);
      union {
        float real;
        uint32_t base;
      } u_timeout;
      u_timeout.real = this->timeout;
      *(outbuffer + offset + 0) = (u_timeout.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_timeout.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_timeout.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_timeout.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->timeout);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      this->socket =  ((uint32_t) (*(inbuffer + offset)));
      this->socket |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->socket |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->socket |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->socket);
      union {
        float real;
        uint32_t base;
      } u_timeout;
      u_timeout.base = 0;
      u_timeout.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_timeout.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_timeout.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_timeout.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->timeout = u_timeout.real;
      offset += sizeof(this->timeout);
     return offset;
    }

    const char * getType(){ return "shared_serial/Flush"; };
    const char * getMD5(){ return "998caebd95b1820bc38f9812484a09de"; };

  };

}
#endif