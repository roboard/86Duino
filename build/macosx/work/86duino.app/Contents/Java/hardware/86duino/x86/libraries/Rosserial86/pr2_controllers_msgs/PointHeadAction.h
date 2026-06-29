#ifndef _ROS_pr2_controllers_msgs_PointHeadAction_h
#define _ROS_pr2_controllers_msgs_PointHeadAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_controllers_msgs/PointHeadActionGoal.h"
#include "pr2_controllers_msgs/PointHeadActionResult.h"
#include "pr2_controllers_msgs/PointHeadActionFeedback.h"

namespace pr2_controllers_msgs
{

  class PointHeadAction : public ros::Msg
  {
    public:
      pr2_controllers_msgs::PointHeadActionGoal action_goal;
      pr2_controllers_msgs::PointHeadActionResult action_result;
      pr2_controllers_msgs::PointHeadActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_controllers_msgs/PointHeadAction"; };
    const char * getMD5(){ return "7252920f1243de1b741f14f214125371"; };

  };

}
#endif