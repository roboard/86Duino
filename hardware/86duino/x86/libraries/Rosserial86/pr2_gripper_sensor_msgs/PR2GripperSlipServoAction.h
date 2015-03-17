#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperSlipServoAction_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperSlipServoAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperSlipServoActionGoal.h"
#include "pr2_gripper_sensor_msgs/PR2GripperSlipServoActionResult.h"
#include "pr2_gripper_sensor_msgs/PR2GripperSlipServoActionFeedback.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperSlipServoAction : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperSlipServoActionGoal action_goal;
      pr2_gripper_sensor_msgs::PR2GripperSlipServoActionResult action_result;
      pr2_gripper_sensor_msgs::PR2GripperSlipServoActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperSlipServoAction"; };
    const char * getMD5(){ return "d1abef6e5d417a62bf67570de0fcd426"; };

  };

}
#endif