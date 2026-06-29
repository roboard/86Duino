#ifndef _ROS_SERVICE_StartMotion_h
#define _ROS_SERVICE_StartMotion_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "industrial_msgs/ServiceReturnCode.h"

namespace industrial_msgs
{

static const char STARTMOTION[] = "industrial_msgs/StartMotion";

  class StartMotionRequest : public ros::Msg
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

    const char * getType(){ return STARTMOTION; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class StartMotionResponse : public ros::Msg
  {
    public:
      industrial_msgs::ServiceReturnCode code;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->code.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->code.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return STARTMOTION; };
    const char * getMD5(){ return "50b1f38f75f5677e5692f3b3e7e1ea48"; };

  };

  class StartMotion {
    public:
    typedef StartMotionRequest Request;
    typedef StartMotionResponse Response;
  };

}
#endif
