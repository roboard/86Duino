#ifndef _ROS_app_manager_ExchangeApp_h
#define _ROS_app_manager_ExchangeApp_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "app_manager/Icon.h"

namespace app_manager
{

  class ExchangeApp : public ros::Msg
  {
    public:
      char * name;
      char * display_name;
      char * version;
      char * latest_version;
      char * description;
      app_manager::Icon icon;
      bool hidden;

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
      uint32_t length_version = strlen( (const char*) this->version);
      memcpy(outbuffer + offset, &length_version, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->version, length_version);
      offset += length_version;
      uint32_t length_latest_version = strlen( (const char*) this->latest_version);
      memcpy(outbuffer + offset, &length_latest_version, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->latest_version, length_latest_version);
      offset += length_latest_version;
      uint32_t length_description = strlen( (const char*) this->description);
      memcpy(outbuffer + offset, &length_description, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->description, length_description);
      offset += length_description;
      offset += this->icon.serialize(outbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_hidden;
      u_hidden.real = this->hidden;
      *(outbuffer + offset + 0) = (u_hidden.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->hidden);
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
      uint32_t length_version;
      memcpy(&length_version, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_version; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_version-1]=0;
      this->version = (char *)(inbuffer + offset-1);
      offset += length_version;
      uint32_t length_latest_version;
      memcpy(&length_latest_version, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_latest_version; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_latest_version-1]=0;
      this->latest_version = (char *)(inbuffer + offset-1);
      offset += length_latest_version;
      uint32_t length_description;
      memcpy(&length_description, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_description; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_description-1]=0;
      this->description = (char *)(inbuffer + offset-1);
      offset += length_description;
      offset += this->icon.deserialize(inbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_hidden;
      u_hidden.base = 0;
      u_hidden.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->hidden = u_hidden.real;
      offset += sizeof(this->hidden);
     return offset;
    }

    const char * getType(){ return "app_manager/ExchangeApp"; };
    const char * getMD5(){ return "ccad20aa9f390121e44c61d218038d78"; };

  };

}
#endif