#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperFindContactFeedback_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperFindContactFeedback_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperFindContactData.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperFindContactFeedback : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperFindContactData data;

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

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperFindContactFeedback"; };
    const char * getMD5(){ return "a1cc8c2fc9268b550e6167f268f97574"; };

  };

}
#endif