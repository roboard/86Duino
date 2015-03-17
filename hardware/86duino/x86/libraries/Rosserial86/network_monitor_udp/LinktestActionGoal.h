#ifndef _ROS_network_monitor_udp_LinktestActionGoal_h
#define _ROS_network_monitor_udp_LinktestActionGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalID.h"
#include "network_monitor_udp/LinktestGoal.h"

namespace network_monitor_udp
{

  class LinktestActionGoal : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalID goal_id;
      network_monitor_udp::LinktestGoal goal;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->goal_id.serialize(outbuffer + offset);
      offset += this->goal.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->goal_id.deserialize(inbuffer + offset);
      offset += this->goal.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "network_monitor_udp/LinktestActionGoal"; };
    const char * getMD5(){ return "63df721c3245a9bd7725838e690d5f4a"; };

  };

}
#endif