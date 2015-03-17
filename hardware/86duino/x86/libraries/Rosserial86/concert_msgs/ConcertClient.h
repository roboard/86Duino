#ifndef _ROS_concert_msgs_ConcertClient_h
#define _ROS_concert_msgs_ConcertClient_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "ros/time.h"
#include "rocon_app_manager_msgs/App.h"

namespace concert_msgs
{

  class ConcertClient : public ros::Msg
  {
    public:
      char * name;
      char * gateway_name;
      char * platform;
      char * system;
      char * robot;
      char * client_status;
      char * app_status;
      char * status;
      ros::Time last_connection_timestamp;
      uint8_t apps_length;
      rocon_app_manager_msgs::App st_apps;
      rocon_app_manager_msgs::App * apps;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      uint32_t length_gateway_name = strlen( (const char*) this->gateway_name);
      memcpy(outbuffer + offset, &length_gateway_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->gateway_name, length_gateway_name);
      offset += length_gateway_name;
      uint32_t length_platform = strlen( (const char*) this->platform);
      memcpy(outbuffer + offset, &length_platform, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->platform, length_platform);
      offset += length_platform;
      uint32_t length_system = strlen( (const char*) this->system);
      memcpy(outbuffer + offset, &length_system, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->system, length_system);
      offset += length_system;
      uint32_t length_robot = strlen( (const char*) this->robot);
      memcpy(outbuffer + offset, &length_robot, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->robot, length_robot);
      offset += length_robot;
      uint32_t length_client_status = strlen( (const char*) this->client_status);
      memcpy(outbuffer + offset, &length_client_status, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->client_status, length_client_status);
      offset += length_client_status;
      uint32_t length_app_status = strlen( (const char*) this->app_status);
      memcpy(outbuffer + offset, &length_app_status, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->app_status, length_app_status);
      offset += length_app_status;
      uint32_t length_status = strlen( (const char*) this->status);
      memcpy(outbuffer + offset, &length_status, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->status, length_status);
      offset += length_status;
      *(outbuffer + offset + 0) = (this->last_connection_timestamp.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->last_connection_timestamp.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->last_connection_timestamp.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->last_connection_timestamp.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->last_connection_timestamp.sec);
      *(outbuffer + offset + 0) = (this->last_connection_timestamp.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->last_connection_timestamp.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->last_connection_timestamp.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->last_connection_timestamp.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->last_connection_timestamp.nsec);
      *(outbuffer + offset++) = apps_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < apps_length; i++){
      offset += this->apps[i].serialize(outbuffer + offset);
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
      uint32_t length_gateway_name;
      memcpy(&length_gateway_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_gateway_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_gateway_name-1]=0;
      this->gateway_name = (char *)(inbuffer + offset-1);
      offset += length_gateway_name;
      uint32_t length_platform;
      memcpy(&length_platform, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_platform; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_platform-1]=0;
      this->platform = (char *)(inbuffer + offset-1);
      offset += length_platform;
      uint32_t length_system;
      memcpy(&length_system, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_system; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_system-1]=0;
      this->system = (char *)(inbuffer + offset-1);
      offset += length_system;
      uint32_t length_robot;
      memcpy(&length_robot, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_robot; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_robot-1]=0;
      this->robot = (char *)(inbuffer + offset-1);
      offset += length_robot;
      uint32_t length_client_status;
      memcpy(&length_client_status, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_client_status; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_client_status-1]=0;
      this->client_status = (char *)(inbuffer + offset-1);
      offset += length_client_status;
      uint32_t length_app_status;
      memcpy(&length_app_status, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_app_status; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_app_status-1]=0;
      this->app_status = (char *)(inbuffer + offset-1);
      offset += length_app_status;
      uint32_t length_status;
      memcpy(&length_status, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_status; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_status-1]=0;
      this->status = (char *)(inbuffer + offset-1);
      offset += length_status;
      this->last_connection_timestamp.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->last_connection_timestamp.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->last_connection_timestamp.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->last_connection_timestamp.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->last_connection_timestamp.sec);
      this->last_connection_timestamp.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->last_connection_timestamp.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->last_connection_timestamp.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->last_connection_timestamp.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->last_connection_timestamp.nsec);
      uint8_t apps_lengthT = *(inbuffer + offset++);
      if(apps_lengthT > apps_length)
        this->apps = (rocon_app_manager_msgs::App*)realloc(this->apps, apps_lengthT * sizeof(rocon_app_manager_msgs::App));
      offset += 3;
      apps_length = apps_lengthT;
      for( uint8_t i = 0; i < apps_length; i++){
      offset += this->st_apps.deserialize(inbuffer + offset);
        memcpy( &(this->apps[i]), &(this->st_apps), sizeof(rocon_app_manager_msgs::App));
      }
     return offset;
    }

    const char * getType(){ return "concert_msgs/ConcertClient"; };
    const char * getMD5(){ return "b3a232588ecbbea02126d208e1faa1a9"; };

  };

}
#endif