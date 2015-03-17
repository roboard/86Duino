#ifndef _ROS_pr2_plugs_msgs_DetectWallNormAction_h
#define _ROS_pr2_plugs_msgs_DetectWallNormAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/DetectWallNormActionGoal.h"
#include "pr2_plugs_msgs/DetectWallNormActionResult.h"
#include "pr2_plugs_msgs/DetectWallNormActionFeedback.h"

namespace pr2_plugs_msgs
{

  class DetectWallNormAction : public ros::Msg
  {
    public:
      pr2_plugs_msgs::DetectWallNormActionGoal action_goal;
      pr2_plugs_msgs::DetectWallNormActionResult action_result;
      pr2_plugs_msgs::DetectWallNormActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_plugs_msgs/DetectWallNormAction"; };
    const char * getMD5(){ return "aed1f64be25979e62454a2eac1ae4789"; };

  };

}
#endif