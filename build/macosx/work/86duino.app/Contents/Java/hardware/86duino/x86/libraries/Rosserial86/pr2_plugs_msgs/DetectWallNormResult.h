#ifndef _ROS_pr2_plugs_msgs_DetectWallNormResult_h
#define _ROS_pr2_plugs_msgs_DetectWallNormResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Vector3Stamped.h"
#include "geometry_msgs/PointStamped.h"

namespace pr2_plugs_msgs
{

  class DetectWallNormResult : public ros::Msg
  {
    public:
      geometry_msgs::Vector3Stamped wall_norm;
      geometry_msgs::PointStamped wall_point;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->wall_norm.serialize(outbuffer + offset);
      offset += this->wall_point.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->wall_norm.deserialize(inbuffer + offset);
      offset += this->wall_point.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/DetectWallNormResult"; };
    const char * getMD5(){ return "5b0db0217832b225255fc76759bc13ac"; };

  };

}
#endif