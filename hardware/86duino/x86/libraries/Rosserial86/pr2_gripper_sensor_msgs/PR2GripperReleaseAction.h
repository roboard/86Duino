#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperReleaseAction_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperReleaseAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperReleaseActionGoal.h"
#include "pr2_gripper_sensor_msgs/PR2GripperReleaseActionResult.h"
#include "pr2_gripper_sensor_msgs/PR2GripperReleaseActionFeedback.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperReleaseAction : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperReleaseActionGoal action_goal;
      pr2_gripper_sensor_msgs::PR2GripperReleaseActionResult action_result;
      pr2_gripper_sensor_msgs::PR2GripperReleaseActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperReleaseAction"; };
    const char * getMD5(){ return "c3c9b6394f2bb7d0d9e5ed002d9a759a"; };

  };

}
#endif