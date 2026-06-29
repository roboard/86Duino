#ifndef _ROS_pr2_plugs_msgs_DetectPlugOnBaseAction_h
#define _ROS_pr2_plugs_msgs_DetectPlugOnBaseAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/DetectPlugOnBaseActionGoal.h"
#include "pr2_plugs_msgs/DetectPlugOnBaseActionResult.h"
#include "pr2_plugs_msgs/DetectPlugOnBaseActionFeedback.h"

namespace pr2_plugs_msgs
{

  class DetectPlugOnBaseAction : public ros::Msg
  {
    public:
      pr2_plugs_msgs::DetectPlugOnBaseActionGoal action_goal;
      pr2_plugs_msgs::DetectPlugOnBaseActionResult action_result;
      pr2_plugs_msgs::DetectPlugOnBaseActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_plugs_msgs/DetectPlugOnBaseAction"; };
    const char * getMD5(){ return "b30c12afa71a80f92f75bb09c002917d"; };

  };

}
#endif