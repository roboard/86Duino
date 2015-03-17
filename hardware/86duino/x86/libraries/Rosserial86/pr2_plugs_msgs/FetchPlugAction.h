#ifndef _ROS_pr2_plugs_msgs_FetchPlugAction_h
#define _ROS_pr2_plugs_msgs_FetchPlugAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/FetchPlugActionGoal.h"
#include "pr2_plugs_msgs/FetchPlugActionResult.h"
#include "pr2_plugs_msgs/FetchPlugActionFeedback.h"

namespace pr2_plugs_msgs
{

  class FetchPlugAction : public ros::Msg
  {
    public:
      pr2_plugs_msgs::FetchPlugActionGoal action_goal;
      pr2_plugs_msgs::FetchPlugActionResult action_result;
      pr2_plugs_msgs::FetchPlugActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_plugs_msgs/FetchPlugAction"; };
    const char * getMD5(){ return "af053731f3da2b41c386a961e9d9a9f1"; };

  };

}
#endif