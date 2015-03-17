#ifndef _ROS_interval_intersection_ConfigAction_h
#define _ROS_interval_intersection_ConfigAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "interval_intersection/ConfigActionGoal.h"
#include "interval_intersection/ConfigActionResult.h"
#include "interval_intersection/ConfigActionFeedback.h"

namespace interval_intersection
{

  class ConfigAction : public ros::Msg
  {
    public:
      interval_intersection::ConfigActionGoal action_goal;
      interval_intersection::ConfigActionResult action_result;
      interval_intersection::ConfigActionFeedback action_feedback;

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

    const char * getType(){ return "interval_intersection/ConfigAction"; };
    const char * getMD5(){ return "daf4f3168538603bba58f6ea42d51314"; };

  };

}
#endif