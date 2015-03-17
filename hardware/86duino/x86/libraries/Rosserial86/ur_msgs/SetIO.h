#ifndef _ROS_SERVICE_SetIO_h
#define _ROS_SERVICE_SetIO_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace ur_msgs
{

static const char SETIO[] = "ur_msgs/SetIO";

  class SetIORequest : public ros::Msg
  {
    public:
      int8_t fun;
      int8_t pin;
      float state;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int8_t real;
        uint8_t base;
      } u_fun;
      u_fun.real = this->fun;
      *(outbuffer + offset + 0) = (u_fun.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->fun);
      union {
        int8_t real;
        uint8_t base;
      } u_pin;
      u_pin.real = this->pin;
      *(outbuffer + offset + 0) = (u_pin.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->pin);
      union {
        float real;
        uint32_t base;
      } u_state;
      u_state.real = this->state;
      *(outbuffer + offset + 0) = (u_state.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_state.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_state.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_state.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->state);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int8_t real;
        uint8_t base;
      } u_fun;
      u_fun.base = 0;
      u_fun.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->fun = u_fun.real;
      offset += sizeof(this->fun);
      union {
        int8_t real;
        uint8_t base;
      } u_pin;
      u_pin.base = 0;
      u_pin.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->pin = u_pin.real;
      offset += sizeof(this->pin);
      union {
        float real;
        uint32_t base;
      } u_state;
      u_state.base = 0;
      u_state.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_state.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_state.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_state.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->state = u_state.real;
      offset += sizeof(this->state);
     return offset;
    }

    const char * getType(){ return SETIO; };
    const char * getMD5(){ return "cbb7d56b1142033251cb8df226edd774"; };

  };

  class SetIOResponse : public ros::Msg
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

    const char * getType(){ return SETIO; };
    const char * getMD5(){ return "358e233cde0c8a8bcfea4ce193f8fc15"; };

  };

  class SetIO {
    public:
    typedef SetIORequest Request;
    typedef SetIOResponse Response;
  };

}
#endif
