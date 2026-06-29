#ifndef _ROS_SERVICE_GetOutput_h
#define _ROS_SERVICE_GetOutput_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "program_queue/Output.h"

namespace program_queue
{

static const char GETOUTPUT[] = "program_queue/GetOutput";

  class GetOutputRequest : public ros::Msg
  {
    public:
      uint64_t token;
      uint64_t program_id;
      uint64_t limit;

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
      union {
        uint64_t real;
        uint32_t base;
      } u_limit;
      u_limit.real = this->limit;
      *(outbuffer + offset + 0) = (u_limit.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_limit.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_limit.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_limit.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->limit);
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
      union {
        uint64_t real;
        uint32_t base;
      } u_limit;
      u_limit.base = 0;
      u_limit.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_limit.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_limit.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_limit.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->limit = u_limit.real;
      offset += sizeof(this->limit);
     return offset;
    }

    const char * getType(){ return GETOUTPUT; };
    const char * getMD5(){ return "f08ecfae10005654998a5ad26daa2eb9"; };

  };

  class GetOutputResponse : public ros::Msg
  {
    public:
      uint8_t output_length;
      program_queue::Output st_output;
      program_queue::Output * output;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = output_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < output_length; i++){
      offset += this->output[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t output_lengthT = *(inbuffer + offset++);
      if(output_lengthT > output_length)
        this->output = (program_queue::Output*)realloc(this->output, output_lengthT * sizeof(program_queue::Output));
      offset += 3;
      output_length = output_lengthT;
      for( uint8_t i = 0; i < output_length; i++){
      offset += this->st_output.deserialize(inbuffer + offset);
        memcpy( &(this->output[i]), &(this->st_output), sizeof(program_queue::Output));
      }
     return offset;
    }

    const char * getType(){ return GETOUTPUT; };
    const char * getMD5(){ return "2f07868317033df679710a7da1873f15"; };

  };

  class GetOutput {
    public:
    typedef GetOutputRequest Request;
    typedef GetOutputResponse Response;
  };

}
#endif
