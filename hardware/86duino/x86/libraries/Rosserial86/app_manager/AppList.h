#ifndef _ROS_app_manager_AppList_h
#define _ROS_app_manager_AppList_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "app_manager/App.h"

namespace app_manager
{

  class AppList : public ros::Msg
  {
    public:
      uint8_t running_apps_length;
      app_manager::App st_running_apps;
      app_manager::App * running_apps;
      uint8_t available_apps_length;
      app_manager::App st_available_apps;
      app_manager::App * available_apps;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = running_apps_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < running_apps_length; i++){
      offset += this->running_apps[i].serialize(outbuffer + offset);
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
      uint8_t running_apps_lengthT = *(inbuffer + offset++);
      if(running_apps_lengthT > running_apps_length)
        this->running_apps = (app_manager::App*)realloc(this->running_apps, running_apps_lengthT * sizeof(app_manager::App));
      offset += 3;
      running_apps_length = running_apps_lengthT;
      for( uint8_t i = 0; i < running_apps_length; i++){
      offset += this->st_running_apps.deserialize(inbuffer + offset);
        memcpy( &(this->running_apps[i]), &(this->st_running_apps), sizeof(app_manager::App));
      }
      uint8_t available_apps_lengthT = *(inbuffer + offset++);
      if(available_apps_lengthT > available_apps_length)
        this->available_apps = (app_manager::App*)realloc(this->available_apps, available_apps_lengthT * sizeof(app_manager::App));
      offset += 3;
      available_apps_length = available_apps_lengthT;
      for( uint8_t i = 0; i < available_apps_length; i++){
      offset += this->st_available_apps.deserialize(inbuffer + offset);
        memcpy( &(this->available_apps[i]), &(this->st_available_apps), sizeof(app_manager::App));
      }
     return offset;
    }

    const char * getType(){ return "app_manager/AppList"; };
    const char * getMD5(){ return "8a71ede6bf51909653c7c551f462cb30"; };

  };

}
#endif