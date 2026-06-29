#ifndef _ROS_pr2_mechanism_msgs_SwitchControllerAction_h
#define _ROS_pr2_mechanism_msgs_SwitchControllerAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_mechanism_msgs/SwitchControllerActionGoal.h"
#include "pr2_mechanism_msgs/SwitchControllerActionResult.h"
#include "pr2_mechanism_msgs/SwitchControllerActionFeedback.h"

namespace pr2_mechanism_msgs
{

  class SwitchControllerAction : public ros::Msg
  {
    public:
      pr2_mechanism_msgs::SwitchControllerActionGoal action_goal;
      pr2_mechanism_msgs::SwitchControllerActionResult action_result;
      pr2_mechanism_msgs::SwitchControllerActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_mechanism_msgs/SwitchControllerAction"; };
    const char * getMD5(){ return "c7b048ee44f1abe19d1dfae77332d13a"; };

  };

}
#endif