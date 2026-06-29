#ifndef _ROS_pr2_plugs_msgs_PlugInGoal_h
#define _ROS_pr2_plugs_msgs_PlugInGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Pose.h"

namespace pr2_plugs_msgs
{

  class PlugInGoal : public ros::Msg
  {
    public:
      geometry_msgs::Pose base_to_outlet;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->base_to_outlet.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->base_to_outlet.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/PlugInGoal"; };
    const char * getMD5(){ return "99078b8d3b9adbf3bc663c4d68d01efd"; };

  };

}
#endif