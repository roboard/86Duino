#ifndef _ROS_SERVICE_DetectWall_h
#define _ROS_SERVICE_DetectWall_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/PointStamped.h"
#include "geometry_msgs/Vector3Stamped.h"

namespace stereo_wall_detection
{

static const char DETECTWALL[] = "stereo_wall_detection/DetectWall";

  class DetectWallRequest : public ros::Msg
  {
    public:

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
     return offset;
    }

    const char * getType(){ return DETECTWALL; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class DetectWallResponse : public ros::Msg
  {
    public:
      geometry_msgs::PointStamped wall_point;
      geometry_msgs::Vector3Stamped wall_norm;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->wall_point.serialize(outbuffer + offset);
      offset += this->wall_norm.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->wall_point.deserialize(inbuffer + offset);
      offset += this->wall_norm.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return DETECTWALL; };
    const char * getMD5(){ return "a6870b3c0e483b78cb98aac96d566717"; };

  };

  class DetectWall {
    public:
    typedef DetectWallRequest Request;
    typedef DetectWallResponse Response;
  };

}
#endif
