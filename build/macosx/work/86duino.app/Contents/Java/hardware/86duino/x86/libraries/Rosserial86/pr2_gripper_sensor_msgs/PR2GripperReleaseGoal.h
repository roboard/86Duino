#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperReleaseGoal_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperReleaseGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperReleaseCommand.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperReleaseGoal : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperReleaseCommand command;

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

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperReleaseGoal"; };
    const char * getMD5(){ return "f92a4c7c03d33b62ef7f6041bec6a43d"; };

  };

}
#endif