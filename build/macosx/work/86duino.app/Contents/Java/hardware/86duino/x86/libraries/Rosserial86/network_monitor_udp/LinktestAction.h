#ifndef _ROS_network_monitor_udp_LinktestAction_h
#define _ROS_network_monitor_udp_LinktestAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "network_monitor_udp/LinktestActionGoal.h"
#include "network_monitor_udp/LinktestActionResult.h"
#include "network_monitor_udp/LinktestActionFeedback.h"

namespace network_monitor_udp
{

  class LinktestAction : public ros::Msg
  {
    public:
      network_monitor_udp::LinktestActionGoal action_goal;
      network_monitor_udp::LinktestActionResult action_result;
      network_monitor_udp::LinktestActionFeedback action_feedback;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->action_goal.serialize(outbuffer + offset);
      offset += this->action_result.serialize(outbuffer + offset);
      offset += this->action_feedback.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->action_goal.deserialize(inbuffer + offset);
      offset += this->action_result.deserialize(inbuffer + offset);
      offset += this->action_feedback.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "network_monitor_udp/LinktestAction"; };
    const char * getMD5(){ return "87d4de3ce9cd4ec54de5ab95fc70d3ac"; };

  };

}
#endif