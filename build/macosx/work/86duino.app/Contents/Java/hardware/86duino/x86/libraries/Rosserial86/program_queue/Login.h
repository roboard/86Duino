#ifndef _ROS_SERVICE_Login_h
#define _ROS_SERVICE_Login_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace program_queue
{

static const char LOGIN[] = "program_queue/Login";

  class LoginRequest : public ros::Msg
  {
    public:
      char * name;
      char * password;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      uint32_t length_password = strlen( (const char*) this->password);
      memcpy(outbuffer + offset, &length_password, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->password, length_password);
      offset += length_password;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_name;
      memcpy(&length_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_name-1]=0;
      this->name = (char *)(inbuffer + offset-1);
      offset += length_name;
      uint32_t length_password;
      memcpy(&length_password, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_password; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_password-1]=0;
      this->password = (char *)(inbuffer + offset-1);
      offset += length_password;
     return offset;
    }

    const char * getType(){ return LOGIN; };
    const char * getMD5(){ return "b597dd6c204454f497f0b80cc3001171"; };

  };

  class LoginResponse : public ros::Msg
  {
    public:
      uint64_t token;
      bool is_admin;

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
        bool real;
        uint8_t base;
      } u_is_admin;
      u_is_admin.real = this->is_admin;
      *(outbuffer + offset + 0) = (u_is_admin.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->is_admin);
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
        bool real;
        uint8_t base;
      } u_is_admin;
      u_is_admin.base = 0;
      u_is_admin.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->is_admin = u_is_admin.real;
      offset += sizeof(this->is_admin);
     return offset;
    }

    const char * getType(){ return LOGIN; };
    const char * getMD5(){ return "0b18fed3f6284cfe6e5d6d685d16d5a6"; };

  };

  class Login {
    public:
    typedef LoginRequest Request;
    typedef LoginResponse Response;
  };

}
#endif
