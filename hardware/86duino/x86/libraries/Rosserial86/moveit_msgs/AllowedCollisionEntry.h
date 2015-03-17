#ifndef _ROS_moveit_msgs_AllowedCollisionEntry_h
#define _ROS_moveit_msgs_AllowedCollisionEntry_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace moveit_msgs
{

  class AllowedCollisionEntry : public ros::Msg
  {
    public:
      uint8_t enabled_length;
      bool st_enabled;
      bool * enabled;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = enabled_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < enabled_length; i++){
      union {
        bool real;
        uint8_t base;
      } u_enabledi;
      u_enabledi.real = this->enabled[i];
      *(outbuffer + offset + 0) = (u_enabledi.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->enabled[i]);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t enabled_lengthT = *(inbuffer + offset++);
      if(enabled_lengthT > enabled_length)
        this->enabled = (bool*)realloc(this->enabled, enabled_lengthT * sizeof(bool));
      offset += 3;
      enabled_length = enabled_lengthT;
      for( uint8_t i = 0; i < enabled_length; i++){
      union {
        bool real;
        uint8_t base;
      } u_st_enabled;
      u_st_enabled.base = 0;
      u_st_enabled.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->st_enabled = u_st_enabled.real;
      offset += sizeof(this->st_enabled);
        memcpy( &(this->enabled[i]), &(this->st_enabled), sizeof(bool));
      }
     return offset;
    }

    const char * getType(){ return "moveit_msgs/AllowedCollisionEntry"; };
    const char * getMD5(){ return "90d1ae1850840724bb043562fe3285fc"; };

  };

}
#endif