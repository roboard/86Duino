#ifndef _ROS_SERVICE_QueueProgram_h
#define _ROS_SERVICE_QueueProgram_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace program_queue
{

static const char QUEUEPROGRAM[] = "program_queue/QueueProgram";

  class QueueProgramRequest : public ros::Msg
  {
    public:
      uint64_t token;
      uint64_t program_id;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        uint64_t real;
        uint32_t base;
      } u_token;
      u_token.real = this->token;
      *(outbuffer + offset + 0) = (u_token.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_token.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_token.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_token.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->token);
      union {
        uint64_t real;
        uint32_t base;
      } u_program_id;
      u_program_id.real = this->program_id;
      *(outbuffer + offset + 0) = (u_program_id.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_program_id.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_program_id.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_program_id.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->program_id);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        uint64_t real;
        uint32_t base;
      } u_token;
      u_token.base = 0;
      u_token.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_token.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_token.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_token.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->token = u_token.real;
      offset += sizeof(this->token);
      union {
        uint64_t real;
        uint32_t base;
      } u_program_id;
      u_program_id.base = 0;
      u_program_id.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_program_id.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_program_id.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_program_id.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->program_id = u_program_id.real;
      offset += sizeof(this->program_id);
     return offset;
    }

    const char * getType(){ return QUEUEPROGRAM; };
    const char * getMD5(){ return "713dc46c9bfd4ddf9bee3500cb0d830c"; };

  };

  class QueueProgramResponse : public ros::Msg
  {
    public:
      uint64_t queue_position;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        uint64_t real;
        uint32_t base;
      } u_queue_position;
      u_queue_position.real = this->queue_position;
      *(outbuffer + offset + 0) = (u_queue_position.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_queue_position.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_queue_position.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_queue_position.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->queue_position);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        uint64_t real;
        uint32_t base;
      } u_queue_position;
      u_queue_position.base = 0;
      u_queue_position.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_queue_position.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_queue_position.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_queue_position.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->queue_position = u_queue_position.real;
      offset += sizeof(this->queue_position);
     return offset;
    }

    const char * getType(){ return QUEUEPROGRAM; };
    const char * getMD5(){ return "28a72135729a2d847325b0cc21963397"; };

  };

  class QueueProgram {
    public:
    typedef QueueProgramRequest Request;
    typedef QueueProgramResponse Response;
  };

}
#endif
