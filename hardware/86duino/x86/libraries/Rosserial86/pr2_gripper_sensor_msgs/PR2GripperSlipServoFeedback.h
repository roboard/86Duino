#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperSlipServoFeedback_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperSlipServoFeedback_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperSlipServoData.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperSlipServoFeedback : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperSlipServoData data;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->data.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->data.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperSlipServoFeedback"; };
    const char * getMD5(){ return "1b10af616c7e94f609790b12cde04c6d"; };

  };

}
#endif