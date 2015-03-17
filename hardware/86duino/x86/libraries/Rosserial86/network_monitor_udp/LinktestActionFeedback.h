#ifndef _ROS_network_monitor_udp_LinktestActionFeedback_h
#define _ROS_network_monitor_udp_LinktestActionFeedback_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "network_monitor_udp/LinktestFeedback.h"

namespace network_monitor_udp
{

  class LinktestActionFeedback : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalStatus status;
      network_monitor_udp::LinktestFeedback feedback;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->status.serialize(outbuffer + offset);
      offset += this->feedback.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->status.deserialize(inbuffer + offset);
      offset += this->feedback.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "network_monitor_udp/LinktestActionFeedback"; };
    const char * getMD5(){ return "967522a816e5535acca726638b979f92"; };

  };

}
#endif