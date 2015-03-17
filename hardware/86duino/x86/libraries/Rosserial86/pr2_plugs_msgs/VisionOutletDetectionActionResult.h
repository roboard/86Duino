#ifndef _ROS_pr2_plugs_msgs_VisionOutletDetectionActionResult_h
#define _ROS_pr2_plugs_msgs_VisionOutletDetectionActionResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "pr2_plugs_msgs/VisionOutletDetectionResult.h"

namespace pr2_plugs_msgs
{

  class VisionOutletDetectionActionResult : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalStatus status;
      pr2_plugs_msgs::VisionOutletDetectionResult result;

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

    const char * getType(){ return "pr2_plugs_msgs/VisionOutletDetectionActionResult"; };
    const char * getMD5(){ return "d6a49959d6c5f7b3da64795b82f1b524"; };

  };

}
#endif