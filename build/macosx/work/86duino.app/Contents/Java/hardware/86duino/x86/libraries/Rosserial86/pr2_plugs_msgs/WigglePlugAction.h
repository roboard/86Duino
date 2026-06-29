#ifndef _ROS_pr2_plugs_msgs_WigglePlugAction_h
#define _ROS_pr2_plugs_msgs_WigglePlugAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/WigglePlugActionGoal.h"
#include "pr2_plugs_msgs/WigglePlugActionResult.h"
#include "pr2_plugs_msgs/WigglePlugActionFeedback.h"

namespace pr2_plugs_msgs
{

  class WigglePlugAction : public ros::Msg
  {
    public:
      pr2_plugs_msgs::WigglePlugActionGoal action_goal;
      pr2_plugs_msgs::WigglePlugActionResult action_result;
      pr2_plugs_msgs::WigglePlugActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_plugs_msgs/WigglePlugAction"; };
    const char * getMD5(){ return "549447bb3ee7b6a5c11024de5884acea"; };

  };

}
#endif