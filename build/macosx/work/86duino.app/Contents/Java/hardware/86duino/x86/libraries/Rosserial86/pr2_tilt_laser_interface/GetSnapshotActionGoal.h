#ifndef _ROS_pr2_tilt_laser_interface_GetSnapshotActionGoal_h
#define _ROS_pr2_tilt_laser_interface_GetSnapshotActionGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalID.h"
#include "pr2_tilt_laser_interface/GetSnapshotGoal.h"

namespace pr2_tilt_laser_interface
{

  class GetSnapshotActionGoal : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalID goal_id;
      pr2_tilt_laser_interface::GetSnapshotGoal goal;

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

    const char * getType(){ return "pr2_tilt_laser_interface/GetSnapshotActionGoal"; };
    const char * getMD5(){ return "b7d8dfaa64edc35cd62f781639e44a0a"; };

  };

}
#endif