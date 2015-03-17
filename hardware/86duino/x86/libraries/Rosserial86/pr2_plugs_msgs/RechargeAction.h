#ifndef _ROS_pr2_plugs_msgs_RechargeAction_h
#define _ROS_pr2_plugs_msgs_RechargeAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/RechargeActionGoal.h"
#include "pr2_plugs_msgs/RechargeActionResult.h"
#include "pr2_plugs_msgs/RechargeActionFeedback.h"

namespace pr2_plugs_msgs
{

  class RechargeAction : public ros::Msg
  {
    public:
      pr2_plugs_msgs::RechargeActionGoal action_goal;
      pr2_plugs_msgs::RechargeActionResult action_result;
      pr2_plugs_msgs::RechargeActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_plugs_msgs/RechargeAction"; };
    const char * getMD5(){ return "cc1440c89a37d17117ca9820383007a2"; };

  };

}
#endif