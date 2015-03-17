#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperEventDetectorGoal_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperEventDetectorGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperEventDetectorCommand.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperEventDetectorGoal : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperEventDetectorCommand command;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->command.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->command.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperEventDetectorGoal"; };
    const char * getMD5(){ return "88b98e578eece7bef53cd48d37d3253b"; };

  };

}
#endif