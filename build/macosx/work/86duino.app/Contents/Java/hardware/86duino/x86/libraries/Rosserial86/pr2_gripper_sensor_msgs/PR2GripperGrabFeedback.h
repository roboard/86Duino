#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperGrabFeedback_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperGrabFeedback_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperGrabData.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperGrabFeedback : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperGrabData data;

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

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperGrabFeedback"; };
    const char * getMD5(){ return "b4b68d48ac7d07bdb11b7f3badfa9266"; };

  };

}
#endif