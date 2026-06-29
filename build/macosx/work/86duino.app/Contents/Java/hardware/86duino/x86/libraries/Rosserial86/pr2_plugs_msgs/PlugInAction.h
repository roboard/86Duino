#ifndef _ROS_pr2_plugs_msgs_PlugInAction_h
#define _ROS_pr2_plugs_msgs_PlugInAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/PlugInActionGoal.h"
#include "pr2_plugs_msgs/PlugInActionResult.h"
#include "pr2_plugs_msgs/PlugInActionFeedback.h"

namespace pr2_plugs_msgs
{

  class PlugInAction : public ros::Msg
  {
    public:
      pr2_plugs_msgs::PlugInActionGoal action_goal;
      pr2_plugs_msgs::PlugInActionResult action_result;
      pr2_plugs_msgs::PlugInActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_plugs_msgs/PlugInAction"; };
    const char * getMD5(){ return "233098d4d7615b96f7d1fb69d583f6fd"; };

  };

}
#endif