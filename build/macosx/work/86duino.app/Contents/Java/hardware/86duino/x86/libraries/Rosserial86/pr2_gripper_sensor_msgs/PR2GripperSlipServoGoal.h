#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperSlipServoGoal_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperSlipServoGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperSlipServoCommand.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperSlipServoGoal : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperSlipServoCommand command;

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

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperSlipServoGoal"; };
    const char * getMD5(){ return "bf76e656d304158c04ab279db7cefc85"; };

  };

}
#endif