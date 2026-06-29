#ifndef _ROS_tf2_web_republisher_TFSubscriptionAction_h
#define _ROS_tf2_web_republisher_TFSubscriptionAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "tf2_web_republisher/TFSubscriptionActionGoal.h"
#include "tf2_web_republisher/TFSubscriptionActionResult.h"
#include "tf2_web_republisher/TFSubscriptionActionFeedback.h"

namespace tf2_web_republisher
{

  class TFSubscriptionAction : public ros::Msg
  {
    public:
      tf2_web_republisher::TFSubscriptionActionGoal action_goal;
      tf2_web_republisher::TFSubscriptionActionResult action_result;
      tf2_web_republisher::TFSubscriptionActionFeedback action_feedback;

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

    const char * getType(){ return "tf2_web_republisher/TFSubscriptionAction"; };
    const char * getMD5(){ return "15787ffd6a2492c0022abe990c898794"; };

  };

}
#endif