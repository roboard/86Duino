#ifndef _ROS_SERVICE_Recv_h
#define _ROS_SERVICE_Recv_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace shared_serial
{

static const char RECV[] = "shared_serial/Recv";

  class RecvRequest : public ros::Msg
  {
    public:
      uint32_t socket;
      uint32_t length;
      float recv_timeout;
      float sock_timeout;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->socket >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->socket >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->socket >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->socket >> (8 * 3)) & 0xFF;
      offset += sizeof(this->socket);
      *(outbuffer + offset + 0) = (this->length >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->length >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->length >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->length >> (8 * 3)) & 0xFF;
      offset += sizeof(this->length);
      union {
        float real;
        uint32_t base;
      } u_recv_timeout;
      u_recv_timeout.real = this->recv_timeout;
      *(outbuffer + offset + 0) = (u_recv_timeout.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_recv_timeout.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_recv_timeout.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_recv_timeout.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->recv_timeout);
      union {
        float real;
        uint32_t base;
      } u_sock_timeout;
      u_sock_timeout.real = this->sock_timeout;
      *(outbuffer + offset + 0) = (u_sock_timeout.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_sock_timeout.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_sock_timeout.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_sock_timeout.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->sock_timeout);
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
      this->length =  ((uint32_t) (*(inbuffer + offset)));
      this->length |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->length |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->length |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->length);
      union {
        float real;
        uint32_t base;
      } u_recv_timeout;
      u_recv_timeout.base = 0;
      u_recv_timeout.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_recv_timeout.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_recv_timeout.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_recv_timeout.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->recv_timeout = u_recv_timeout.real;
      offset += sizeof(this->recv_timeout);
      union {
        float real;
        uint32_t base;
      } u_sock_timeout;
      u_sock_timeout.base = 0;
      u_sock_timeout.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_sock_timeout.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_sock_timeout.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_sock_timeout.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->sock_timeout = u_sock_timeout.real;
      offset += sizeof(this->sock_timeout);
     return offset;
    }

    const char * getType(){ return RECV; };
    const char * getMD5(){ return "c05d3c907840837e419db15be31d15b6"; };

  };

  class RecvResponse : public ros::Msg
  {
    public:
      uint32_t socket;
      uint8_t data_length;
      uint8_t st_data;
      uint8_t * data;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->socket >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->socket >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->socket >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->socket >> (8 * 3)) & 0xFF;
      offset += sizeof(this->socket);
      *(outbuffer + offset++) = data_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < data_length; i++){
      *(outbuffer + offset + 0) = (this->data[i] >> (8 * 0)) & 0xFF;
      offset += sizeof(this->data[i]);
      }
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
      uint8_t data_lengthT = *(inbuffer + offset++);
      if(data_lengthT > data_length)
        this->data = (uint8_t*)realloc(this->data, data_lengthT * sizeof(uint8_t));
      offset += 3;
      data_length = data_lengthT;
      for( uint8_t i = 0; i < data_length; i++){
      this->st_data =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->st_data);
        memcpy( &(this->data[i]), &(this->st_data), sizeof(uint8_t));
      }
     return offset;
    }

    const char * getType(){ return RECV; };
    const char * getMD5(){ return "9aa8c2746a0876552ecc2a81ad0d58a5"; };

  };

  class Recv {
    public:
    typedef RecvRequest Request;
    typedef RecvResponse Response;
  };

}
#endif
