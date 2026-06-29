#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperForceServoAction_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperForceServoAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperForceServoActionGoal.h"
#include "pr2_gripper_sensor_msgs/PR2GripperForceServoActionResult.h"
#include "pr2_gripper_sensor_msgs/PR2GripperForceServoActionFeedback.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperForceServoAction : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperForceServoActionGoal action_goal;
      pr2_gripper_sensor_msgs::PR2GripperForceServoActionResult action_result;
      pr2_gripper_sensor_msgs::PR2GripperForceServoActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperForceServoAction"; };
    const char * getMD5(){ return "0540bb7603e65b3df5c9dc87b150e790"; };

  };

}
#endif