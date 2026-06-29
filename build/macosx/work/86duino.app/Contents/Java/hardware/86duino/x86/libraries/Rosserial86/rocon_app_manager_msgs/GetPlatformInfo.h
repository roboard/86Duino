#ifndef _ROS_SERVICE_GetPlatformInfo_h
#define _ROS_SERVICE_GetPlatformInfo_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "rocon_app_manager_msgs/PlatformInfo.h"

namespace rocon_app_manager_msgs
{

static const char GETPLATFORMINFO[] = "rocon_app_manager_msgs/GetPlatformInfo";

  class GetPlatformInfoRequest : public ros::Msg
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

    const char * getType(){ return GETPLATFORMINFO; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class GetPlatformInfoResponse : public ros::Msg
  {
    public:
      rocon_app_manager_msgs::PlatformInfo platform_info;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->platform_info.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->platform_info.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return GETPLATFORMINFO; };
    const char * getMD5(){ return "d3a75b8e8f07aea96f74c921f13e7d59"; };

  };

  class GetPlatformInfo {
    public:
    typedef GetPlatformInfoRequest Request;
    typedef GetPlatformInfoResponse Response;
  };

}
#endif
