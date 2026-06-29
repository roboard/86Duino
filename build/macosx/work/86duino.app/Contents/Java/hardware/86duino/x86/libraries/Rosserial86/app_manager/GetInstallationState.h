#ifndef _ROS_SERVICE_GetInstallationState_h
#define _ROS_SERVICE_GetInstallationState_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "app_manager/ExchangeApp.h"

namespace app_manager
{

static const char GETINSTALLATIONSTATE[] = "app_manager/GetInstallationState";

  class GetInstallationStateRequest : public ros::Msg
  {
    public:
      bool remote_update;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_remote_update;
      u_remote_update.real = this->remote_update;
      *(outbuffer + offset + 0) = (u_remote_update.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->remote_update);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_remote_update;
      u_remote_update.base = 0;
      u_remote_update.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->remote_update = u_remote_update.real;
      offset += sizeof(this->remote_update);
     return offset;
    }

    const char * getType(){ return GETINSTALLATIONSTATE; };
    const char * getMD5(){ return "f7e64723808960ca985ba81f45f1b8a7"; };

  };

  class GetInstallationStateResponse : public ros::Msg
  {
    public:
      uint8_t installed_apps_length;
      app_manager::ExchangeApp st_installed_apps;
      app_manager::ExchangeApp * installed_apps;
      uint8_t available_apps_length;
      app_manager::ExchangeApp st_available_apps;
      app_manager::ExchangeApp * available_apps;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = installed_apps_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < installed_apps_length; i++){
      offset += this->installed_apps[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = available_apps_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < available_apps_length; i++){
      offset += this->available_apps[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t installed_apps_lengthT = *(inbuffer + offset++);
      if(installed_apps_lengthT > installed_apps_length)
        this->installed_apps = (app_manager::ExchangeApp*)realloc(this->installed_apps, installed_apps_lengthT * sizeof(app_manager::ExchangeApp));
      offset += 3;
      installed_apps_length = installed_apps_lengthT;
      for( uint8_t i = 0; i < installed_apps_length; i++){
      offset += this->st_installed_apps.deserialize(inbuffer + offset);
        memcpy( &(this->installed_apps[i]), &(this->st_installed_apps), sizeof(app_manager::ExchangeApp));
      }
      uint8_t available_apps_lengthT = *(inbuffer + offset++);
      if(available_apps_lengthT > available_apps_length)
        this->available_apps = (app_manager::ExchangeApp*)realloc(this->available_apps, available_apps_lengthT * sizeof(app_manager::ExchangeApp));
      offset += 3;
      available_apps_length = available_apps_lengthT;
      for( uint8_t i = 0; i < available_apps_length; i++){
      offset += this->st_available_apps.deserialize(inbuffer + offset);
        memcpy( &(this->available_apps[i]), &(this->st_available_apps), sizeof(app_manager::ExchangeApp));
      }
     return offset;
    }

    const char * getType(){ return GETINSTALLATIONSTATE; };
    const char * getMD5(){ return "46d45bbda08250199267aff8c0ee8c41"; };

  };

  class GetInstallationState {
    public:
    typedef GetInstallationStateRequest Request;
    typedef GetInstallationStateResponse Response;
  };

}
#endif
