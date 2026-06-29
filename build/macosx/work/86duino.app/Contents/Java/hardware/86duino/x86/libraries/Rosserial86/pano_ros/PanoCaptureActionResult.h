#ifndef _ROS_pano_ros_PanoCaptureActionResult_h
#define _ROS_pano_ros_PanoCaptureActionResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "pano_ros/PanoCaptureResult.h"

namespace pano_ros
{

  class PanoCaptureActionResult : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalStatus status;
      pano_ros::PanoCaptureResult result;

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

    const char * getType(){ return "pano_ros/PanoCaptureActionResult"; };
    const char * getMD5(){ return "de9a456fe8204e7c814ed7bcdefd6373"; };

  };

}
#endif