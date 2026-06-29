#ifndef _ROS_SERVICE_PowerBoardCommand_h
#define _ROS_SERVICE_PowerBoardCommand_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_power_board
{

static const char POWERBOARDCOMMAND[] = "pr2_power_board/PowerBoardCommand";

  class PowerBoardCommandRequest : public ros::Msg
  {
    public:
      uint32_t serial_number;
      int32_t breaker_number;
      char * command;
      uint32_t flags;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->serial_number >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->serial_number >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->serial_number >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->serial_number >> (8 * 3)) & 0xFF;
      offset += sizeof(this->serial_number);
      union {
        int32_t real;
        uint32_t base;
      } u_breaker_number;
      u_breaker_number.real = this->breaker_number;
      *(outbuffer + offset + 0) = (u_breaker_number.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_breaker_number.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_breaker_number.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_breaker_number.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->breaker_number);
      uint32_t length_command = strlen( (const char*) this->command);
      memcpy(outbuffer + offset, &length_command, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->command, length_command);
      offset += length_command;
      *(outbuffer + offset + 0) = (this->flags >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->flags >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->flags >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->flags >> (8 * 3)) & 0xFF;
      offset += sizeof(this->flags);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      this->serial_number =  ((uint32_t) (*(inbuffer + offset)));
      this->serial_number |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->serial_number |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->serial_number |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->serial_number);
      union {
        int32_t real;
        uint32_t base;
      } u_breaker_number;
      u_breaker_number.base = 0;
      u_breaker_number.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_breaker_number.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_breaker_number.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_breaker_number.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->breaker_number = u_breaker_number.real;
      offset += sizeof(this->breaker_number);
      uint32_t length_command;
      memcpy(&length_command, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_command; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_command-1]=0;
      this->command = (char *)(inbuffer + offset-1);
      offset += length_command;
      this->flags =  ((uint32_t) (*(inbuffer + offset)));
      this->flags |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->flags |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->flags |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->flags);
     return offset;
    }

    const char * getType(){ return POWERBOARDCOMMAND; };
    const char * getMD5(){ return "9c621f5309bca0033e8eaef81c31500a"; };

  };

  class PowerBoardCommandResponse : public ros::Msg
  {
    public:
      int32_t retval;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_retval;
      u_retval.real = this->retval;
      *(outbuffer + offset + 0) = (u_retval.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_retval.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_retval.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_retval.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->retval);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_retval;
      u_retval.base = 0;
      u_retval.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_retval.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_retval.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_retval.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->retval = u_retval.real;
      offset += sizeof(this->retval);
     return offset;
    }

    const char * getType(){ return POWERBOARDCOMMAND; };
    const char * getMD5(){ return "f5697a1e05c2a3e1c23cab49a31319ec"; };

  };

  class PowerBoardCommand {
    public:
    typedef PowerBoardCommandRequest Request;
    typedef PowerBoardCommandResponse Response;
  };

}
#endif
