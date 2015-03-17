#ifndef _ROS_SERVICE_SetPayload_h
#define _ROS_SERVICE_SetPayload_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace ur_msgs
{

static const char SETPAYLOAD[] = "ur_msgs/SetPayload";

  class SetPayloadRequest : public ros::Msg
  {
    public:
      float payload;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_payload;
      u_payload.real = this->payload;
      *(outbuffer + offset + 0) = (u_payload.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_payload.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_payload.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_payload.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->payload);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_payload;
      u_payload.base = 0;
      u_payload.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_payload.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_payload.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_payload.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->payload = u_payload.real;
      offset += sizeof(this->payload);
     return offset;
    }

    const char * getType(){ return SETPAYLOAD; };
    const char * getMD5(){ return "d12269f931817591aa52047629ca66ca"; };

  };

  class SetPayloadResponse : public ros::Msg
  {
    public:
      bool success;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_success;
      u_success.real = this->success;
      *(outbuffer + offset + 0) = (u_success.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->success);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_success;
      u_success.base = 0;
      u_success.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->success = u_success.real;
      offset += sizeof(this->success);
     return offset;
    }

    const char * getType(){ return SETPAYLOAD; };
    const char * getMD5(){ return "358e233cde0c8a8bcfea4ce193f8fc15"; };

  };

  class SetPayload {
    public:
    typedef SetPayloadRequest Request;
    typedef SetPayloadResponse Response;
  };

}
#endif
