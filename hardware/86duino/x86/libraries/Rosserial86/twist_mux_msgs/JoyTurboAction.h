#ifndef _ROS_twist_mux_msgs_JoyTurboAction_h
#define _ROS_twist_mux_msgs_JoyTurboAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "twist_mux_msgs/JoyTurboActionGoal.h"
#include "twist_mux_msgs/JoyTurboActionResult.h"
#include "twist_mux_msgs/JoyTurboActionFeedback.h"

namespace twist_mux_msgs
{

  class JoyTurboAction : public ros::Msg
  {
    public:
      twist_mux_msgs::JoyTurboActionGoal action_goal;
      twist_mux_msgs::JoyTurboActionResult action_result;
      twist_mux_msgs::JoyTurboActionFeedback action_feedback;

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

    const char * getType(){ return "twist_mux_msgs/JoyTurboAction"; };
    const char * getMD5(){ return "d5a016b49f278075666fbc901debbd08"; };

  };

}
#endif