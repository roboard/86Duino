#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperEventDetectorResult_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperEventDetectorResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperEventDetectorData.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperEventDetectorResult : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperEventDetectorData data;

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

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperEventDetectorResult"; };
    const char * getMD5(){ return "817b45a51c75a067eb5dfb8e18b14aa1"; };

  };

}
#endif