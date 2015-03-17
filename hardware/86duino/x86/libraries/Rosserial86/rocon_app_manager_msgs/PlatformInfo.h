#ifndef _ROS_rocon_app_manager_msgs_PlatformInfo_h
#define _ROS_rocon_app_manager_msgs_PlatformInfo_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "rocon_app_manager_msgs/Icon.h"

namespace rocon_app_manager_msgs
{

  class PlatformInfo : public ros::Msg
  {
    public:
      char * platform;
      char * system;
      char * robot;
      char * name;
      rocon_app_manager_msgs::Icon icon;
      enum { PLATFORM_ANY = * };
      enum { PLATFORM_LINUX = linux };
      enum { PLATFORM_WINDOZE = windoze };
      enum { PLATFORM_ANDROID = android };
      enum { SYSTEM_CUSTOM = custom };
      enum { SYSTEM_ROS = ros };
      enum { SYSTEM_OPROS = opros };
      enum { ROBOT_ANY = * };
      enum { ROBOT_PC = pc };
      enum { ROBOT_ROBOSEM = robosem };
      enum { ROBOT_KOBUKI = kobuki };
      enum { ROBOT_TURTLEBOT = turtlebot };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
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
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      offset += this->icon.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
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
      uint32_t length_name;
      memcpy(&length_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_name-1]=0;
      this->name = (char *)(inbuffer + offset-1);
      offset += length_name;
      offset += this->icon.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "rocon_app_manager_msgs/PlatformInfo"; };
    const char * getMD5(){ return "18db61d835a2c85545a016989914665e"; };

  };

}
#endif