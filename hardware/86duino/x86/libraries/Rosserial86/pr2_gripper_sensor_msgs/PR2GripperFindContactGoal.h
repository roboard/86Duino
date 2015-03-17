#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperFindContactGoal_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperFindContactGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperFindContactCommand.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperFindContactGoal : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperFindContactCommand command;

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

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperFindContactGoal"; };
    const char * getMD5(){ return "f0ae570e217e7429eba0f205341933a0"; };

  };

}
#endif