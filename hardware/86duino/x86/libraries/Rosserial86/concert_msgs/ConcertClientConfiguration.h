#ifndef _ROS_concert_msgs_ConcertClientConfiguration_h
#define _ROS_concert_msgs_ConcertClientConfiguration_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace concert_msgs
{

  class ConcertClientConfiguration : public ros::Msg
  {
    public:
      char * platform;
      char * system;
      char * robot;
      char * app;

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
      uint32_t length_app = strlen( (const char*) this->app);
      memcpy(outbuffer + offset, &length_app, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->app, length_app);
      offset += length_app;
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
      uint32_t length_app;
      memcpy(&length_app, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_app; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_app-1]=0;
      this->app = (char *)(inbuffer + offset-1);
      offset += length_app;
     return offset;
    }

    const char * getType(){ return "concert_msgs/ConcertClientConfiguration"; };
    const char * getMD5(){ return "271180ccd464930e6cd371441f8f7bf5"; };

  };

}
#endif