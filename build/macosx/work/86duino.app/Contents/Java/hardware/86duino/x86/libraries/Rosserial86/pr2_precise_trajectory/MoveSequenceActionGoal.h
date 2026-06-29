#ifndef _ROS_pr2_precise_trajectory_MoveSequenceActionGoal_h
#define _ROS_pr2_precise_trajectory_MoveSequenceActionGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalID.h"
#include "pr2_precise_trajectory/MoveSequenceGoal.h"

namespace pr2_precise_trajectory
{

  class MoveSequenceActionGoal : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalID goal_id;
      pr2_precise_trajectory::MoveSequenceGoal goal;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->goal_id.serialize(outbuffer + offset);
      offset += this->goal.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->goal_id.deserialize(inbuffer + offset);
      offset += this->goal.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_precise_trajectory/MoveSequenceActionGoal"; };
    const char * getMD5(){ return "6c004149dd4a0f1025171a05e3c05221"; };

  };

}
#endif