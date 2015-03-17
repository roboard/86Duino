#ifndef _ROS_app_manager_App_h
#define _ROS_app_manager_App_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "app_manager/Icon.h"
#include "app_manager/ClientApp.h"

namespace app_manager
{

  class App : public ros::Msg
  {
    public:
      char * name;
      char * display_name;
      app_manager::Icon icon;
      uint8_t client_apps_length;
      app_manager::ClientApp st_client_apps;
      app_manager::ClientApp * client_apps;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      uint32_t length_display_name = strlen( (const char*) this->display_name);
      memcpy(outbuffer + offset, &length_display_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->display_name, length_display_name);
      offset += length_display_name;
      offset += this->icon.serialize(outbuffer + offset);
      *(outbuffer + offset++) = client_apps_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < client_apps_length; i++){
      offset += this->client_apps[i].serialize(outbuffer + offset);
      }
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
      uint32_t length_display_name;
      memcpy(&length_display_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_display_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_display_name-1]=0;
      this->display_name = (char *)(inbuffer + offset-1);
      offset += length_display_name;
      offset += this->icon.deserialize(inbuffer + offset);
      uint8_t client_apps_lengthT = *(inbuffer + offset++);
      if(client_apps_lengthT > client_apps_length)
        this->client_apps = (app_manager::ClientApp*)realloc(this->client_apps, client_apps_lengthT * sizeof(app_manager::ClientApp));
      offset += 3;
      client_apps_length = client_apps_lengthT;
      for( uint8_t i = 0; i < client_apps_length; i++){
      offset += this->st_client_apps.deserialize(inbuffer + offset);
        memcpy( &(this->client_apps[i]), &(this->st_client_apps), sizeof(app_manager::ClientApp));
      }
     return offset;
    }

    const char * getType(){ return "app_manager/App"; };
    const char * getMD5(){ return "643c1db5f71b615a47789ff5e190811e"; };

  };

}
#endif