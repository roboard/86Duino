#ifndef _ROS_pr2_plugs_msgs_VisionPlugDetectionAction_h
#define _ROS_pr2_plugs_msgs_VisionPlugDetectionAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/VisionPlugDetectionActionGoal.h"
#include "pr2_plugs_msgs/VisionPlugDetectionActionResult.h"
#include "pr2_plugs_msgs/VisionPlugDetectionActionFeedback.h"

namespace pr2_plugs_msgs
{

  class VisionPlugDetectionAction : public ros::Msg
  {
    public:
      pr2_plugs_msgs::VisionPlugDetectionActionGoal action_goal;
      pr2_plugs_msgs::VisionPlugDetectionActionResult action_result;
      pr2_plugs_msgs::VisionPlugDetectionActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_plugs_msgs/VisionPlugDetectionAction"; };
    const char * getMD5(){ return "2fabd1f35635b6fd6cb05419d68414d7"; };

  };

}
#endif