#ifndef _ROS_pr2_plugs_msgs_DetectOutletResult_h
#define _ROS_pr2_plugs_msgs_DetectOutletResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Pose.h"

namespace pr2_plugs_msgs
{

  class DetectOutletResult : public ros::Msg
  {
    public:
      geometry_msgs::Pose base_to_outlet_pose;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->base_to_outlet_pose.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->base_to_outlet_pose.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/DetectOutletResult"; };
    const char * getMD5(){ return "1d071981939746debd9292384b39dba4"; };

  };

}
#endif