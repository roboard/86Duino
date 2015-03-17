#ifndef _ROS_pr2_tilt_laser_interface_GetSnapshotResult_h
#define _ROS_pr2_tilt_laser_interface_GetSnapshotResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "sensor_msgs/PointCloud2.h"

namespace pr2_tilt_laser_interface
{

  class GetSnapshotResult : public ros::Msg
  {
    public:
      sensor_msgs::PointCloud2 cloud;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->cloud.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->cloud.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_tilt_laser_interface/GetSnapshotResult"; };
    const char * getMD5(){ return "96cec5374164b3b3d1d7ef5d7628a7ed"; };

  };

}
#endif