#ifndef _ROS_pr2_precise_trajectory_MoveSequenceActionFeedback_h
#define _ROS_pr2_precise_trajectory_MoveSequenceActionFeedback_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "pr2_precise_trajectory/MoveSequenceFeedback.h"

namespace pr2_precise_trajectory
{

  class MoveSequenceActionFeedback : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalStatus status;
      pr2_precise_trajectory::MoveSequenceFeedback feedback;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->status.serialize(outbuffer + offset);
      offset += this->feedback.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->status.deserialize(inbuffer + offset);
      offset += this->feedback.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_precise_trajectory/MoveSequenceActionFeedback"; };
    const char * getMD5(){ return "9ff19d44120a1ed27ef5716e79f481c6"; };

  };

}
#endif