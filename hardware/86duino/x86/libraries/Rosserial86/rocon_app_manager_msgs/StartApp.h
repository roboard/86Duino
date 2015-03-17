#ifndef _ROS_SERVICE_StartApp_h
#define _ROS_SERVICE_StartApp_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "rocon_app_manager_msgs/Remapping.h"

namespace rocon_app_manager_msgs
{

static const char STARTAPP[] = "rocon_app_manager_msgs/StartApp";

  class StartAppRequest : public ros::Msg
  {
    public:
      char * name;
      uint8_t remappings_length;
      rocon_app_manager_msgs::Remapping st_remappings;
      rocon_app_manager_msgs::Remapping * remappings;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      *(outbuffer + offset++) = remappings_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < remappings_length; i++){
      offset += this->remappings[i].serialize(outbuffer + offset);
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
      uint8_t remappings_lengthT = *(inbuffer + offset++);
      if(remappings_lengthT > remappings_length)
        this->remappings = (rocon_app_manager_msgs::Remapping*)realloc(this->remappings, remappings_lengthT * sizeof(rocon_app_manager_msgs::Remapping));
      offset += 3;
      remappings_length = remappings_lengthT;
      for( uint8_t i = 0; i < remappings_length; i++){
      offset += this->st_remappings.deserialize(inbuffer + offset);
        memcpy( &(this->remappings[i]), &(this->st_remappings), sizeof(rocon_app_manager_msgs::Remapping));
      }
     return offset;
    }

    const char * getType(){ return STARTAPP; };
    const char * getMD5(){ return "52a705a1e5933de18edbae79e2aafe49"; };

  };

  class StartAppResponse : public ros::Msg
  {
    public:
      bool started;
      int32_t error_code;
      char * message;
      char * app_namespace;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_started;
      u_started.real = this->started;
      *(outbuffer + offset + 0) = (u_started.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->started);
      union {
        int32_t real;
        uint32_t base;
      } u_error_code;
      u_error_code.real = this->error_code;
      *(outbuffer + offset + 0) = (u_error_code.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_error_code.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_error_code.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_error_code.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->error_code);
      uint32_t length_message = strlen( (const char*) this->message);
      memcpy(outbuffer + offset, &length_message, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->message, length_message);
      offset += length_message;
      uint32_t length_app_namespace = strlen( (const char*) this->app_namespace);
      memcpy(outbuffer + offset, &length_app_namespace, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->app_namespace, length_app_namespace);
      offset += length_app_namespace;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_started;
      u_started.base = 0;
      u_started.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->started = u_started.real;
      offset += sizeof(this->started);
      union {
        int32_t real;
        uint32_t base;
      } u_error_code;
      u_error_code.base = 0;
      u_error_code.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_error_code.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_error_code.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_error_code.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->error_code = u_error_code.real;
      offset += sizeof(this->error_code);
      uint32_t length_message;
      memcpy(&length_message, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_message; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_message-1]=0;
      this->message = (char *)(inbuffer + offset-1);
      offset += length_message;
      uint32_t length_app_namespace;
      memcpy(&length_app_namespace, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_app_namespace; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_app_namespace-1]=0;
      this->app_namespace = (char *)(inbuffer + offset-1);
      offset += length_app_namespace;
     return offset;
    }

    const char * getType(){ return STARTAPP; };
    const char * getMD5(){ return "d47dedd876ba5df1daf4654018d1329b"; };

  };

  class StartApp {
    public:
    typedef StartAppRequest Request;
    typedef StartAppResponse Response;
  };

}
#endif
