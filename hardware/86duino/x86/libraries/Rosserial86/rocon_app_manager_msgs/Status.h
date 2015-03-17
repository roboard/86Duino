#ifndef _ROS_SERVICE_Status_h
#define _ROS_SERVICE_Status_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "rocon_app_manager_msgs/App.h"

namespace rocon_app_manager_msgs
{

static const char STATUS[] = "rocon_app_manager_msgs/Status";

  class StatusRequest : public ros::Msg
  {
    public:

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
     return offset;
    }

    const char * getType(){ return STATUS; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class StatusResponse : public ros::Msg
  {
    public:
      char * application_namespace;
      char * remote_controller;
      char * application_status;
      rocon_app_manager_msgs::App application;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_application_namespace = strlen( (const char*) this->application_namespace);
      memcpy(outbuffer + offset, &length_application_namespace, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->application_namespace, length_application_namespace);
      offset += length_application_namespace;
      uint32_t length_remote_controller = strlen( (const char*) this->remote_controller);
      memcpy(outbuffer + offset, &length_remote_controller, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->remote_controller, length_remote_controller);
      offset += length_remote_controller;
      uint32_t length_application_status = strlen( (const char*) this->application_status);
      memcpy(outbuffer + offset, &length_application_status, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->application_status, length_application_status);
      offset += length_application_status;
      offset += this->application.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_application_namespace;
      memcpy(&length_application_namespace, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_application_namespace; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_application_namespace-1]=0;
      this->application_namespace = (char *)(inbuffer + offset-1);
      offset += length_application_namespace;
      uint32_t length_remote_controller;
      memcpy(&length_remote_controller, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_remote_controller; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_remote_controller-1]=0;
      this->remote_controller = (char *)(inbuffer + offset-1);
      offset += length_remote_controller;
      uint32_t length_application_status;
      memcpy(&length_application_status, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_application_status; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_application_status-1]=0;
      this->application_status = (char *)(inbuffer + offset-1);
      offset += length_application_status;
      offset += this->application.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return STATUS; };
    const char * getMD5(){ return "549cefdd5ebd893b04e084dde8f31530"; };

  };

  class Status {
    public:
    typedef StatusRequest Request;
    typedef StatusResponse Response;
  };

}
#endif
