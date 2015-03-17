#ifndef _ROS_SERVICE_Connect_h
#define _ROS_SERVICE_Connect_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace shared_serial
{

static const char CONNECT[] = "shared_serial/Connect";

  class ConnectRequest : public ros::Msg
  {
    public:
      float timeout;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
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

    const char * getType(){ return CONNECT; };
    const char * getMD5(){ return "8e929ace7fd80dc265b8b96078f41e82"; };

  };

  class ConnectResponse : public ros::Msg
  {
    public:
      int32_t socket;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_socket;
      u_socket.real = this->socket;
      *(outbuffer + offset + 0) = (u_socket.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_socket.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_socket.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_socket.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->socket);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_socket;
      u_socket.base = 0;
      u_socket.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_socket.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_socket.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_socket.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->socket = u_socket.real;
      offset += sizeof(this->socket);
     return offset;
    }

    const char * getType(){ return CONNECT; };
    const char * getMD5(){ return "89a3ab074876917505d8ad961102ef9f"; };

  };

  class Connect {
    public:
    typedef ConnectRequest Request;
    typedef ConnectResponse Response;
  };

}
#endif
