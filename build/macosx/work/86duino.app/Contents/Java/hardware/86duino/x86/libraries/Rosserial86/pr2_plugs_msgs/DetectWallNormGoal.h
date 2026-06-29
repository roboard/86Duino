#ifndef _ROS_pr2_plugs_msgs_DetectWallNormGoal_h
#define _ROS_pr2_plugs_msgs_DetectWallNormGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/PointStamped.h"

namespace pr2_plugs_msgs
{

  class DetectWallNormGoal : public ros::Msg
  {
    public:
      geometry_msgs::PointStamped look_point;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->look_point.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->look_point.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/DetectWallNormGoal"; };
    const char * getMD5(){ return "e584b4d17af4c9d4ab4eb40db4b41013"; };

  };

}
#endif