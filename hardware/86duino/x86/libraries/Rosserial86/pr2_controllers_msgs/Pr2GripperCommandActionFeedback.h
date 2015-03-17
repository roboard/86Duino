#ifndef _ROS_pr2_controllers_msgs_Pr2GripperCommandActionFeedback_h
#define _ROS_pr2_controllers_msgs_Pr2GripperCommandActionFeedback_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "pr2_controllers_msgs/Pr2GripperCommandFeedback.h"

namespace pr2_controllers_msgs
{

  class Pr2GripperCommandActionFeedback : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalStatus status;
      pr2_controllers_msgs::Pr2GripperCommandFeedback feedback;

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

    const char * getType(){ return "pr2_controllers_msgs/Pr2GripperCommandActionFeedback"; };
    const char * getMD5(){ return "653dff30c045f5e6ff3feb3409f4558d"; };

  };

}
#endif