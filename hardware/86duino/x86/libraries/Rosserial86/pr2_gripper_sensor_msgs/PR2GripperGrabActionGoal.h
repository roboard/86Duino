#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperGrabActionGoal_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperGrabActionGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalID.h"
#include "pr2_gripper_sensor_msgs/PR2GripperGrabGoal.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperGrabActionGoal : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalID goal_id;
      pr2_gripper_sensor_msgs::PR2GripperGrabGoal goal;

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

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperGrabActionGoal"; };
    const char * getMD5(){ return "e540762b3e9da0e115b0380658bdbe12"; };

  };

}
#endif