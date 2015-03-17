#ifndef _ROS_pr2_precise_trajectory_MoveSequenceAction_h
#define _ROS_pr2_precise_trajectory_MoveSequenceAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_precise_trajectory/MoveSequenceActionGoal.h"
#include "pr2_precise_trajectory/MoveSequenceActionResult.h"
#include "pr2_precise_trajectory/MoveSequenceActionFeedback.h"

namespace pr2_precise_trajectory
{

  class MoveSequenceAction : public ros::Msg
  {
    public:
      pr2_precise_trajectory::MoveSequenceActionGoal action_goal;
      pr2_precise_trajectory::MoveSequenceActionResult action_result;
      pr2_precise_trajectory::MoveSequenceActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_precise_trajectory/MoveSequenceAction"; };
    const char * getMD5(){ return "c742bcf9e07a8bdb468f9b6eacec4bfa"; };

  };

}
#endif