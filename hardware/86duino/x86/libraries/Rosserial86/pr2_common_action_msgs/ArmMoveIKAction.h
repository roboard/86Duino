#ifndef _ROS_pr2_common_action_msgs_ArmMoveIKAction_h
#define _ROS_pr2_common_action_msgs_ArmMoveIKAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_common_action_msgs/ArmMoveIKActionGoal.h"
#include "pr2_common_action_msgs/ArmMoveIKActionResult.h"
#include "pr2_common_action_msgs/ArmMoveIKActionFeedback.h"

namespace pr2_common_action_msgs
{

  class ArmMoveIKAction : public ros::Msg
  {
    public:
      pr2_common_action_msgs::ArmMoveIKActionGoal action_goal;
      pr2_common_action_msgs::ArmMoveIKActionResult action_result;
      pr2_common_action_msgs::ArmMoveIKActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_common_action_msgs/ArmMoveIKAction"; };
    const char * getMD5(){ return "f9df8066f1c6f2acf0247564f92e5ff2"; };

  };

}
#endif