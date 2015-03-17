#ifndef _ROS_pr2_common_action_msgs_TuckArmsAction_h
#define _ROS_pr2_common_action_msgs_TuckArmsAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_common_action_msgs/TuckArmsActionGoal.h"
#include "pr2_common_action_msgs/TuckArmsActionResult.h"
#include "pr2_common_action_msgs/TuckArmsActionFeedback.h"

namespace pr2_common_action_msgs
{

  class TuckArmsAction : public ros::Msg
  {
    public:
      pr2_common_action_msgs::TuckArmsActionGoal action_goal;
      pr2_common_action_msgs::TuckArmsActionResult action_result;
      pr2_common_action_msgs::TuckArmsActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_common_action_msgs/TuckArmsAction"; };
    const char * getMD5(){ return "4385d436aaa7b0cb11299848e25300f5"; };

  };

}
#endif