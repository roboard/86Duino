#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperReleaseCommand_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperReleaseCommand_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gripper_sensor_msgs/PR2GripperEventDetectorCommand.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperReleaseCommand : public ros::Msg
  {
    public:
      pr2_gripper_sensor_msgs::PR2GripperEventDetectorCommand event;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->event.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->event.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperReleaseCommand"; };
    const char * getMD5(){ return "e62b08129864bf301ed0a1335e6158dc"; };

  };

}
#endif