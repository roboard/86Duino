#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperGrabAction_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperGrabAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperGrabActionGoal.h"
#include "pr2_gripper_sensor_msgs/PR2GripperGrabActionResult.h"
#include "pr2_gripper_sensor_msgs/PR2GripperGrabActionFeedback.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperGrabAction : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperGrabActionGoal action_goal;
      pr2_gripper_sensor_msgs::PR2GripperGrabActionResult action_result;
      pr2_gripper_sensor_msgs::PR2GripperGrabActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperGrabAction"; };
    const char * getMD5(){ return "f467562414aabe5b90666be976b0c379"; };

  };

}
#endif