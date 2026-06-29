#ifndef _ROS_pr2_plugs_msgs_PlugInResult_h
#define _ROS_pr2_plugs_msgs_PlugInResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Pose.h"

namespace pr2_plugs_msgs
{

  class PlugInResult : public ros::Msg
  {
    public:
      geometry_msgs::Pose gripper_to_plug;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->gripper_to_plug.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->gripper_to_plug.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/PlugInResult"; };
    const char * getMD5(){ return "673ccf1c09212980cb6a232d9870f8f7"; };

  };

}
#endif