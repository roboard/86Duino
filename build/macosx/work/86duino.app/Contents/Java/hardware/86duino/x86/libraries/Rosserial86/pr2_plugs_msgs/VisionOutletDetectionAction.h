#ifndef _ROS_pr2_plugs_msgs_VisionOutletDetectionAction_h
#define _ROS_pr2_plugs_msgs_VisionOutletDetectionAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/VisionOutletDetectionActionGoal.h"
#include "pr2_plugs_msgs/VisionOutletDetectionActionResult.h"
#include "pr2_plugs_msgs/VisionOutletDetectionActionFeedback.h"

namespace pr2_plugs_msgs
{

  class VisionOutletDetectionAction : public ros::Msg
  {
    public:
      pr2_plugs_msgs::VisionOutletDetectionActionGoal action_goal;
      pr2_plugs_msgs::VisionOutletDetectionActionResult action_result;
      pr2_plugs_msgs::VisionOutletDetectionActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_plugs_msgs/VisionOutletDetectionAction"; };
    const char * getMD5(){ return "5fbdebe7af5616cd1ccf350ec7449ae9"; };

  };

}
#endif