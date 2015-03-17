#ifndef _ROS_app_manager_ClientApp_h
#define _ROS_app_manager_ClientApp_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "app_manager/KeyValue.h"

namespace app_manager
{

  class ClientApp : public ros::Msg
  {
    public:
      char * client_type;
      uint8_t manager_data_length;
      app_manager::KeyValue st_manager_data;
      app_manager::KeyValue * manager_data;
      uint8_t app_data_length;
      app_manager::KeyValue st_app_data;
      app_manager::KeyValue * app_data;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_client_type = strlen( (const char*) this->client_type);
      memcpy(outbuffer + offset, &length_client_type, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->client_type, length_client_type);
      offset += length_client_type;
      *(outbuffer + offset++) = manager_data_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < manager_data_length; i++){
      offset += this->manager_data[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = app_data_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < app_data_length; i++){
      offset += this->app_data[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_client_type;
      memcpy(&length_client_type, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_client_type; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_client_type-1]=0;
      this->client_type = (char *)(inbuffer + offset-1);
      offset += length_client_type;
      uint8_t manager_data_lengthT = *(inbuffer + offset++);
      if(manager_data_lengthT > manager_data_length)
        this->manager_data = (app_manager::KeyValue*)realloc(this->manager_data, manager_data_lengthT * sizeof(app_manager::KeyValue));
      offset += 3;
      manager_data_length = manager_data_lengthT;
      for( uint8_t i = 0; i < manager_data_length; i++){
      offset += this->st_manager_data.deserialize(inbuffer + offset);
        memcpy( &(this->manager_data[i]), &(this->st_manager_data), sizeof(app_manager::KeyValue));
      }
      uint8_t app_data_lengthT = *(inbuffer + offset++);
      if(app_data_lengthT > app_data_length)
        this->app_data = (app_manager::KeyValue*)realloc(this->app_data, app_data_lengthT * sizeof(app_manager::KeyValue));
      offset += 3;
      app_data_length = app_data_lengthT;
      for( uint8_t i = 0; i < app_data_length; i++){
      offset += this->st_app_data.deserialize(inbuffer + offset);
        memcpy( &(this->app_data[i]), &(this->st_app_data), sizeof(app_manager::KeyValue));
      }
     return offset;
    }

    const char * getType(){ return "app_manager/ClientApp"; };
    const char * getMD5(){ return "0a8112672c3fbf73cb62ec78d67e41bb"; };

  };

}
#endif