#ifndef _ROS_pr2_plugs_msgs_DetectPlugOnBaseResult_h
#define _ROS_pr2_plugs_msgs_DetectPlugOnBaseResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/PoseStamped.h"

namespace pr2_plugs_msgs
{

  class DetectPlugOnBaseResult : public ros::Msg
  {
    public:
      geometry_msgs::PoseStamped plug_pose;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->plug_pose.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->plug_pose.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/DetectPlugOnBaseResult"; };
    const char * getMD5(){ return "71af249a01ba6b399781f7e0d4e1fbfd"; };

  };

}
#endif