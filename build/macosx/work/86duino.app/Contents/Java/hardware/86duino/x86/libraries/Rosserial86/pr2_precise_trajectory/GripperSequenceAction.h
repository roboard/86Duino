#ifndef _ROS_pr2_precise_trajectory_GripperSequenceAction_h
#define _ROS_pr2_precise_trajectory_GripperSequenceAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_precise_trajectory/GripperSequenceActionGoal.h"
#include "pr2_precise_trajectory/GripperSequenceActionResult.h"
#include "pr2_precise_trajectory/GripperSequenceActionFeedback.h"

namespace pr2_precise_trajectory
{

  class GripperSequenceAction : public ros::Msg
  {
    public:
      pr2_precise_trajectory::GripperSequenceActionGoal action_goal;
      pr2_precise_trajectory::GripperSequenceActionResult action_result;
      pr2_precise_trajectory::GripperSequenceActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_precise_trajectory/GripperSequenceAction"; };
    const char * getMD5(){ return "163d66791fadcbaf1caba7545d9c80a5"; };

  };

}
#endif