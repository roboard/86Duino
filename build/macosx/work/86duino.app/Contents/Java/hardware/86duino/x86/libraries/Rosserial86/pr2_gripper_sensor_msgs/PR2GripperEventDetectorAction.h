#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperEventDetectorAction_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperEventDetectorAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperEventDetectorActionGoal.h"
#include "pr2_gripper_sensor_msgs/PR2GripperEventDetectorActionResult.h"
#include "pr2_gripper_sensor_msgs/PR2GripperEventDetectorActionFeedback.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperEventDetectorAction : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperEventDetectorActionGoal action_goal;
      pr2_gripper_sensor_msgs::PR2GripperEventDetectorActionResult action_result;
      pr2_gripper_sensor_msgs::PR2GripperEventDetectorActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperEventDetectorAction"; };
    const char * getMD5(){ return "d4e2ee2d04e941280f34bdb366daa9a4"; };

  };

}
#endif