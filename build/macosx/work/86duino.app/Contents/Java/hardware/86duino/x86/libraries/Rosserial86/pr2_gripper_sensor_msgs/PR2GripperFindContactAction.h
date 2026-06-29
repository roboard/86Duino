#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperFindContactAction_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperFindContactAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperFindContactActionGoal.h"
#include "pr2_gripper_sensor_msgs/PR2GripperFindContactActionResult.h"
#include "pr2_gripper_sensor_msgs/PR2GripperFindContactActionFeedback.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperFindContactAction : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperFindContactActionGoal action_goal;
      pr2_gripper_sensor_msgs::PR2GripperFindContactActionResult action_result;
      pr2_gripper_sensor_msgs::PR2GripperFindContactActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperFindContactAction"; };
    const char * getMD5(){ return "99ab2df1bbde46c447b38f28b7896d16"; };

  };

}
#endif