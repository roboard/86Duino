#ifndef _ROS_pr2_plugs_msgs_RechargeGoal_h
#define _ROS_pr2_plugs_msgs_RechargeGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/RechargeCommand.h"

namespace pr2_plugs_msgs
{

  class RechargeGoal : public ros::Msg
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

    const char * getType(){ return "pr2_plugs_msgs/RechargeGoal"; };
    const char * getMD5(){ return "641a24f64aac349324462c0d7dcafe3c"; };

  };

}
#endif