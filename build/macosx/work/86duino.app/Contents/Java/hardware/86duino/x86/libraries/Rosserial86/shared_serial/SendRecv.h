#ifndef _ROS_SERVICE_SendRecv_h
#define _ROS_SERVICE_SendRecv_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace shared_serial
{

static const char SENDRECV[] = "shared_serial/SendRecv";

  class SendRecvRequest : public ros::Msg
  {
    public:
      uint32_t socket;
      uint8_t send_data_length;
      uint8_t st_send_data;
      uint8_t * send_data;
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
      *(outbuffer + offset++) = send_data_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < send_data_length; i++){
      *(outbuffer + offset + 0) = (this->send_data[i] >> (8 * 0)) & 0xFF;
      offset += sizeof(this->send_data[i]);
      }
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
      uint8_t send_data_lengthT = *(inbuffer + offset++);
      if(send_data_lengthT > send_data_length)
        this->send_data = (uint8_t*)realloc(this->send_data, send_data_lengthT * sizeof(uint8_t));
      offset += 3;
      send_data_length = send_data_lengthT;
      for( uint8_t i = 0; i < send_data_length; i++){
      this->st_send_data =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->st_send_data);
        memcpy( &(this->send_data[i]), &(this->st_send_data), sizeof(uint8_t));
      }
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

    const char * getType(){ return SENDRECV; };
    const char * getMD5(){ return "3fa8cb9d9b17970f1ef94f8b72f437be"; };

  };

  class SendRecvResponse : public ros::Msg
  {
    public:
      uint32_t socket;
      uint8_t recv_data_length;
      uint8_t st_recv_data;
      uint8_t * recv_data;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->socket >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->socket >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->socket >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->socket >> (8 * 3)) & 0xFF;
      offset += sizeof(this->socket);
      *(outbuffer + offset++) = recv_data_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < recv_data_length; i++){
      *(outbuffer + offset + 0) = (this->recv_data[i] >> (8 * 0)) & 0xFF;
      offset += sizeof(this->recv_data[i]);
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
      uint8_t recv_data_lengthT = *(inbuffer + offset++);
      if(recv_data_lengthT > recv_data_length)
        this->recv_data = (uint8_t*)realloc(this->recv_data, recv_data_lengthT * sizeof(uint8_t));
      offset += 3;
      recv_data_length = recv_data_lengthT;
      for( uint8_t i = 0; i < recv_data_length; i++){
      this->st_recv_data =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->st_recv_data);
        memcpy( &(this->recv_data[i]), &(this->st_recv_data), sizeof(uint8_t));
      }
     return offset;
    }

    const char * getType(){ return SENDRECV; };
    const char * getMD5(){ return "85c9ca63788ec2863d436bf470432137"; };

  };

  class SendRecv {
    public:
    typedef SendRecvRequest Request;
    typedef SendRecvResponse Response;
  };

}
#endif
