#ifndef _ROS_pr2_plugs_msgs_VisionOutletDetectionResult_h
#define _ROS_pr2_plugs_msgs_VisionOutletDetectionResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/PoseStamped.h"

namespace pr2_plugs_msgs
{

  class VisionOutletDetectionResult : public ros::Msg
  {
    public:
      geometry_msgs::PoseStamped outlet_pose;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->outlet_pose.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->outlet_pose.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/VisionOutletDetectionResult"; };
    const char * getMD5(){ return "e989d3f2014af2ec7bcc3ae5f53a30f5"; };

  };

}
#endif