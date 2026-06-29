#ifndef _ROS_SERVICE_RechargeRequest_h
#define _ROS_SERVICE_RechargeRequest_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/RechargeCommand.h"

namespace pr2_plugs_msgs
{

static const char RECHARGEREQUEST[] = "pr2_plugs_msgs/RechargeRequest";

  class RechargeRequestRequest : public ros::Msg
  {
    public:
      pr2_plugs_msgs::RechargeCommand command;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->command.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->command.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return RECHARGEREQUEST; };
    const char * getMD5(){ return "641a24f64aac349324462c0d7dcafe3c"; };

  };

  class RechargeRequestResponse : public ros::Msg
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

    const char * getType(){ return RECHARGEREQUEST; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class RechargeRequest {
    public:
    typedef RechargeRequestRequest Request;
    typedef RechargeRequestResponse Response;
  };

}
#endif
