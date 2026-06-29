#ifndef _ROS_SERVICE_GetRobotInfo_h
#define _ROS_SERVICE_GetRobotInfo_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "industrial_msgs/ServiceReturnCode.h"
#include "industrial_msgs/DeviceInfo.h"

namespace industrial_msgs
{

static const char GETROBOTINFO[] = "industrial_msgs/GetRobotInfo";

  class GetRobotInfoRequest : public ros::Msg
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

    const char * getType(){ return GETROBOTINFO; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class GetRobotInfoResponse : public ros::Msg
  {
    public:
      industrial_msgs::DeviceInfo controller;
      uint8_t robots_length;
      industrial_msgs::DeviceInfo st_robots;
      industrial_msgs::DeviceInfo * robots;
      industrial_msgs::ServiceReturnCode code;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->controller.serialize(outbuffer + offset);
      *(outbuffer + offset++) = robots_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < robots_length; i++){
      offset += this->robots[i].serialize(outbuffer + offset);
      }
      offset += this->code.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->controller.deserialize(inbuffer + offset);
      uint8_t robots_lengthT = *(inbuffer + offset++);
      if(robots_lengthT > robots_length)
        this->robots = (industrial_msgs::DeviceInfo*)realloc(this->robots, robots_lengthT * sizeof(industrial_msgs::DeviceInfo));
      offset += 3;
      robots_length = robots_lengthT;
      for( uint8_t i = 0; i < robots_length; i++){
      offset += this->st_robots.deserialize(inbuffer + offset);
        memcpy( &(this->robots[i]), &(this->st_robots), sizeof(industrial_msgs::DeviceInfo));
      }
      offset += this->code.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return GETROBOTINFO; };
    const char * getMD5(){ return "5db3230b3e61c85a320b999ffd7f3b3f"; };

  };

  class GetRobotInfo {
    public:
    typedef GetRobotInfoRequest Request;
    typedef GetRobotInfoResponse Response;
  };

}
#endif
