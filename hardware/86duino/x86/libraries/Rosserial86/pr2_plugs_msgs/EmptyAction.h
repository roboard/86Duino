#ifndef _ROS_pr2_plugs_msgs_EmptyAction_h
#define _ROS_pr2_plugs_msgs_EmptyAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/EmptyActionGoal.h"
#include "pr2_plugs_msgs/EmptyActionResult.h"
#include "pr2_plugs_msgs/EmptyActionFeedback.h"

namespace pr2_plugs_msgs
{

  class EmptyAction : public ros::Msg
  {
    public:
      pr2_plugs_msgs::EmptyActionGoal action_goal;
      pr2_plugs_msgs::EmptyActionResult action_result;
      pr2_plugs_msgs::EmptyActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_plugs_msgs/EmptyAction"; };
    const char * getMD5(){ return "d5a016b49f278075666fbc901debbd08"; };

  };

}
#endif