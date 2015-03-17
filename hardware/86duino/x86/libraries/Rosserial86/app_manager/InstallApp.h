#ifndef _ROS_SERVICE_InstallApp_h
#define _ROS_SERVICE_InstallApp_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace app_manager
{

static const char INSTALLAPP[] = "app_manager/InstallApp";

  class InstallAppRequest : public ros::Msg
  {
    public:
      char * name;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
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
     return offset;
    }

    const char * getType(){ return INSTALLAPP; };
    const char * getMD5(){ return "c1f3d28f1b044c871e6eff2e9fc3c667"; };

  };

  class InstallAppResponse : public ros::Msg
  {
    public:
      bool installed;
      char * message;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_installed;
      u_installed.real = this->installed;
      *(outbuffer + offset + 0) = (u_installed.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->installed);
      uint32_t length_message = strlen( (const char*) this->message);
      memcpy(outbuffer + offset, &length_message, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->message, length_message);
      offset += length_message;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_installed;
      u_installed.base = 0;
      u_installed.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->installed = u_installed.real;
      offset += sizeof(this->installed);
      uint32_t length_message;
      memcpy(&length_message, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_message; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_message-1]=0;
      this->message = (char *)(inbuffer + offset-1);
      offset += length_message;
     return offset;
    }

    const char * getType(){ return INSTALLAPP; };
    const char * getMD5(){ return "08871bc6dbc6813537edf236ff26a1e2"; };

  };

  class InstallApp {
    public:
    typedef InstallAppRequest Request;
    typedef InstallAppResponse Response;
  };

}
#endif
