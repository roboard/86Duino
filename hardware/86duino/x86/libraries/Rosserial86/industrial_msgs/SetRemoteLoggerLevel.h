#ifndef _ROS_SERVICE_SetRemoteLoggerLevel_h
#define _ROS_SERVICE_SetRemoteLoggerLevel_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "industrial_msgs/ServiceReturnCode.h"
#include "industrial_msgs/DebugLevel.h"

namespace industrial_msgs
{

static const char SETREMOTELOGGERLEVEL[] = "industrial_msgs/SetRemoteLoggerLevel";

  class SetRemoteLoggerLevelRequest : public ros::Msg
  {
    public:
      industrial_msgs::DebugLevel level;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->level.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->level.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return SETREMOTELOGGERLEVEL; };
    const char * getMD5(){ return "3a0089a136293a269fa1a44788ce9777"; };

  };

  class SetRemoteLoggerLevelResponse : public ros::Msg
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

    const char * getType(){ return SETREMOTELOGGERLEVEL; };
    const char * getMD5(){ return "50b1f38f75f5677e5692f3b3e7e1ea48"; };

  };

  class SetRemoteLoggerLevel {
    public:
    typedef SetRemoteLoggerLevelRequest Request;
    typedef SetRemoteLoggerLevelResponse Response;
  };

}
#endif
