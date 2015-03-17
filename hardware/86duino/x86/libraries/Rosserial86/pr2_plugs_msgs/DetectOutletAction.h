#ifndef _ROS_pr2_plugs_msgs_DetectOutletAction_h
#define _ROS_pr2_plugs_msgs_DetectOutletAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/DetectOutletActionGoal.h"
#include "pr2_plugs_msgs/DetectOutletActionResult.h"
#include "pr2_plugs_msgs/DetectOutletActionFeedback.h"

namespace pr2_plugs_msgs
{

  class DetectOutletAction : public ros::Msg
  {
    public:
      pr2_plugs_msgs::DetectOutletActionGoal action_goal;
      pr2_plugs_msgs::DetectOutletActionResult action_result;
      pr2_plugs_msgs::DetectOutletActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_plugs_msgs/DetectOutletAction"; };
    const char * getMD5(){ return "7da230c88b96273a4028dcec81f23fbb"; };

  };

}
#endif