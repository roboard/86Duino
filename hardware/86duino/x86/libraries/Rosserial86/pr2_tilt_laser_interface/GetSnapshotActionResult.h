#ifndef _ROS_pr2_tilt_laser_interface_GetSnapshotActionResult_h
#define _ROS_pr2_tilt_laser_interface_GetSnapshotActionResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "pr2_tilt_laser_interface/GetSnapshotResult.h"

namespace pr2_tilt_laser_interface
{

  class GetSnapshotActionResult : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalStatus status;
      pr2_tilt_laser_interface::GetSnapshotResult result;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->status.serialize(outbuffer + offset);
      offset += this->result.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->status.deserialize(inbuffer + offset);
      offset += this->result.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_tilt_laser_interface/GetSnapshotActionResult"; };
    const char * getMD5(){ return "56d477df5ecfc314886aed9a9ec75b8c"; };

  };

}
#endif