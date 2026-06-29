#ifndef _ROS_SERVICE_Invite_h
#define _ROS_SERVICE_Invite_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace rocon_app_manager_msgs
{

static const char INVITE[] = "rocon_app_manager_msgs/Invite";

  class InviteRequest : public ros::Msg
  {
    public:
      char * remote_target_name;
      char * application_namespace;
      bool cancel;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_remote_target_name = strlen( (const char*) this->remote_target_name);
      memcpy(outbuffer + offset, &length_remote_target_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->remote_target_name, length_remote_target_name);
      offset += length_remote_target_name;
      uint32_t length_application_namespace = strlen( (const char*) this->application_namespace);
      memcpy(outbuffer + offset, &length_application_namespace, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->application_namespace, length_application_namespace);
      offset += length_application_namespace;
      union {
        bool real;
        uint8_t base;
      } u_cancel;
      u_cancel.real = this->cancel;
      *(outbuffer + offset + 0) = (u_cancel.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->cancel);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_remote_target_name;
      memcpy(&length_remote_target_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_remote_target_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_remote_target_name-1]=0;
      this->remote_target_name = (char *)(inbuffer + offset-1);
      offset += length_remote_target_name;
      uint32_t length_application_namespace;
      memcpy(&length_application_namespace, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_application_namespace; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_application_namespace-1]=0;
      this->application_namespace = (char *)(inbuffer + offset-1);
      offset += length_application_namespace;
      union {
        bool real;
        uint8_t base;
      } u_cancel;
      u_cancel.base = 0;
      u_cancel.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->cancel = u_cancel.real;
      offset += sizeof(this->cancel);
     return offset;
    }

    const char * getType(){ return INVITE; };
    const char * getMD5(){ return "4bed4f247c648d28c7c34a7504917911"; };

  };

  class InviteResponse : public ros::Msg
  {
    public:
      bool result;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_result;
      u_result.real = this->result;
      *(outbuffer + offset + 0) = (u_result.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->result);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_result;
      u_result.base = 0;
      u_result.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->result = u_result.real;
      offset += sizeof(this->result);
     return offset;
    }

    const char * getType(){ return INVITE; };
    const char * getMD5(){ return "eb13ac1f1354ccecb7941ee8fa2192e8"; };

  };

  class Invite {
    public:
    typedef InviteRequest Request;
    typedef InviteResponse Response;
  };

}
#endif
