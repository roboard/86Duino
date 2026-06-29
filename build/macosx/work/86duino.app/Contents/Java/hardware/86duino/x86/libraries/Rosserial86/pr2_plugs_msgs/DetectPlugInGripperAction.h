#ifndef _ROS_pr2_plugs_msgs_DetectPlugInGripperAction_h
#define _ROS_pr2_plugs_msgs_DetectPlugInGripperAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/DetectPlugInGripperActionGoal.h"
#include "pr2_plugs_msgs/DetectPlugInGripperActionResult.h"
#include "pr2_plugs_msgs/DetectPlugInGripperActionFeedback.h"

namespace pr2_plugs_msgs
{

  class DetectPlugInGripperAction : public ros::Msg
  {
    public:
      pr2_plugs_msgs::DetectPlugInGripperActionGoal action_goal;
      pr2_plugs_msgs::DetectPlugInGripperActionResult action_result;
      pr2_plugs_msgs::DetectPlugInGripperActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_plugs_msgs/DetectPlugInGripperAction"; };
    const char * getMD5(){ return "91a0fab00f0755b5347a45e330e82c56"; };

  };

}
#endif