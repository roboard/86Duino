#ifndef _ROS_pr2_plugs_msgs_DetectWallNormActionResult_h
#define _ROS_pr2_plugs_msgs_DetectWallNormActionResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "pr2_plugs_msgs/DetectWallNormResult.h"

namespace pr2_plugs_msgs
{

  class DetectWallNormActionResult : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalStatus status;
      pr2_plugs_msgs::DetectWallNormResult result;

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

    const char * getType(){ return "pr2_plugs_msgs/DetectWallNormActionResult"; };
    const char * getMD5(){ return "67c20c5c430c26b5ae56b5ce360b149e"; };

  };

}
#endif