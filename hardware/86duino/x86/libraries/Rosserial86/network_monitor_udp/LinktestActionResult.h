#ifndef _ROS_network_monitor_udp_LinktestActionResult_h
#define _ROS_network_monitor_udp_LinktestActionResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "network_monitor_udp/LinktestResult.h"

namespace network_monitor_udp
{

  class LinktestActionResult : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalStatus status;
      network_monitor_udp::LinktestResult result;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->status.serialize(outbuffer + offset);
      offset += this->result.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->status.deserialize(inbuffer + offset);
      offset += this->result.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "network_monitor_udp/LinktestActionResult"; };
    const char * getMD5(){ return "68fbd44062fcb0a381155a2ce859c30f"; };

  };

}
#endif