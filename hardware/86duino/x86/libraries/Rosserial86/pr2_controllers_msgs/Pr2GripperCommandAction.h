#ifndef _ROS_pr2_controllers_msgs_Pr2GripperCommandAction_h
#define _ROS_pr2_controllers_msgs_Pr2GripperCommandAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_controllers_msgs/Pr2GripperCommandActionGoal.h"
#include "pr2_controllers_msgs/Pr2GripperCommandActionResult.h"
#include "pr2_controllers_msgs/Pr2GripperCommandActionFeedback.h"

namespace pr2_controllers_msgs
{

  class Pr2GripperCommandAction : public ros::Msg
  {
    public:
      pr2_controllers_msgs::Pr2GripperCommandActionGoal action_goal;
      pr2_controllers_msgs::Pr2GripperCommandActionResult action_result;
      pr2_controllers_msgs::Pr2GripperCommandActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_controllers_msgs/Pr2GripperCommandAction"; };
    const char * getMD5(){ return "950b2a6ebe831f5d4f4ceaba3d8be01e"; };

  };

}
#endif