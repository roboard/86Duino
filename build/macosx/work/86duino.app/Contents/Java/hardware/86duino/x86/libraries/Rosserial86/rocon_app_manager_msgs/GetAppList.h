#ifndef _ROS_SERVICE_GetAppList_h
#define _ROS_SERVICE_GetAppList_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "rocon_app_manager_msgs/App.h"

namespace rocon_app_manager_msgs
{

static const char GETAPPLIST[] = "rocon_app_manager_msgs/GetAppList";

  class GetAppListRequest : public ros::Msg
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

    const char * getType(){ return GETAPPLIST; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class GetAppListResponse : public ros::Msg
  {
    public:
      uint8_t available_apps_length;
      rocon_app_manager_msgs::App st_available_apps;
      rocon_app_manager_msgs::App * available_apps;
      uint8_t running_apps_length;
      rocon_app_manager_msgs::App st_running_apps;
      rocon_app_manager_msgs::App * running_apps;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = available_apps_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < available_apps_length; i++){
      offset += this->available_apps[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = running_apps_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < running_apps_length; i++){
      offset += this->running_apps[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t available_apps_lengthT = *(inbuffer + offset++);
      if(available_apps_lengthT > available_apps_length)
        this->available_apps = (rocon_app_manager_msgs::App*)realloc(this->available_apps, available_apps_lengthT * sizeof(rocon_app_manager_msgs::App));
      offset += 3;
      available_apps_length = available_apps_lengthT;
      for( uint8_t i = 0; i < available_apps_length; i++){
      offset += this->st_available_apps.deserialize(inbuffer + offset);
        memcpy( &(this->available_apps[i]), &(this->st_available_apps), sizeof(rocon_app_manager_msgs::App));
      }
      uint8_t running_apps_lengthT = *(inbuffer + offset++);
      if(running_apps_lengthT > running_apps_length)
        this->running_apps = (rocon_app_manager_msgs::App*)realloc(this->running_apps, running_apps_lengthT * sizeof(rocon_app_manager_msgs::App));
      offset += 3;
      running_apps_length = running_apps_lengthT;
      for( uint8_t i = 0; i < running_apps_length; i++){
      offset += this->st_running_apps.deserialize(inbuffer + offset);
        memcpy( &(this->running_apps[i]), &(this->st_running_apps), sizeof(rocon_app_manager_msgs::App));
      }
     return offset;
    }

    const char * getType(){ return GETAPPLIST; };
    const char * getMD5(){ return "dc2c4094a09bf136a3e1a0c1ee817df8"; };

  };

  class GetAppList {
    public:
    typedef GetAppListRequest Request;
    typedef GetAppListResponse Response;
  };

}
#endif
