#ifndef _ROS_pr2_plugs_msgs_StowPlugAction_h
#define _ROS_pr2_plugs_msgs_StowPlugAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/StowPlugActionGoal.h"
#include "pr2_plugs_msgs/StowPlugActionResult.h"
#include "pr2_plugs_msgs/StowPlugActionFeedback.h"

namespace pr2_plugs_msgs
{

  class StowPlugAction : public ros::Msg
  {
    public:
      pr2_plugs_msgs::StowPlugActionGoal action_goal;
      pr2_plugs_msgs::StowPlugActionResult action_result;
      pr2_plugs_msgs::StowPlugActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_plugs_msgs/StowPlugAction"; };
    const char * getMD5(){ return "9a24836648de65a6be36729bbcbef901"; };

  };

}
#endif