#ifndef _ROS_SERVICE_Invite_h
#define _ROS_SERVICE_Invite_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace concert_msgs
{

static const char INVITE[] = "concert_msgs/Invite";

  class InviteRequest : public ros::Msg
  {
    public:
      char * mastername;
      uint8_t clientnames_length;
      char* st_clientnames;
      char* * clientnames;
      bool ok_flag;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_mastername = strlen( (const char*) this->mastername);
      memcpy(outbuffer + offset, &length_mastername, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->mastername, length_mastername);
      offset += length_mastername;
      *(outbuffer + offset++) = clientnames_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < clientnames_length; i++){
      uint32_t length_clientnamesi = strlen( (const char*) this->clientnames[i]);
      memcpy(outbuffer + offset, &length_clientnamesi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->clientnames[i], length_clientnamesi);
      offset += length_clientnamesi;
      }
      union {
        bool real;
        uint8_t base;
      } u_ok_flag;
      u_ok_flag.real = this->ok_flag;
      *(outbuffer + offset + 0) = (u_ok_flag.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->ok_flag);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_mastername;
      memcpy(&length_mastername, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_mastername; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_mastername-1]=0;
      this->mastername = (char *)(inbuffer + offset-1);
      offset += length_mastername;
      uint8_t clientnames_lengthT = *(inbuffer + offset++);
      if(clientnames_lengthT > clientnames_length)
        this->clientnames = (char**)realloc(this->clientnames, clientnames_lengthT * sizeof(char*));
      offset += 3;
      clientnames_length = clientnames_lengthT;
      for( uint8_t i = 0; i < clientnames_length; i++){
      uint32_t length_st_clientnames;
      memcpy(&length_st_clientnames, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_clientnames; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_clientnames-1]=0;
      this->st_clientnames = (char *)(inbuffer + offset-1);
      offset += length_st_clientnames;
        memcpy( &(this->clientnames[i]), &(this->st_clientnames), sizeof(char*));
      }
      union {
        bool real;
        uint8_t base;
      } u_ok_flag;
      u_ok_flag.base = 0;
      u_ok_flag.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->ok_flag = u_ok_flag.real;
      offset += sizeof(this->ok_flag);
     return offset;
    }

    const char * getType(){ return INVITE; };
    const char * getMD5(){ return "082cfd7cce8a75992c90b91803aba9c4"; };

  };

  class InviteResponse : public ros::Msg
  {
    public:
      char * result;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_result = strlen( (const char*) this->result);
      memcpy(outbuffer + offset, &length_result, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->result, length_result);
      offset += length_result;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_result;
      memcpy(&length_result, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_result; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_result-1]=0;
      this->result = (char *)(inbuffer + offset-1);
      offset += length_result;
     return offset;
    }

    const char * getType(){ return INVITE; };
    const char * getMD5(){ return "c22f2a1ed8654a0b365f1bb3f7ff2c0f"; };

  };

  class Invite {
    public:
    typedef InviteRequest Request;
    typedef InviteResponse Response;
  };

}
#endif
