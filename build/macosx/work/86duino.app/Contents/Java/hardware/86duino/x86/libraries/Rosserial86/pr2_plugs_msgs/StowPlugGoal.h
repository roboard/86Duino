#ifndef _ROS_pr2_plugs_msgs_StowPlugGoal_h
#define _ROS_pr2_plugs_msgs_StowPlugGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Pose.h"

namespace pr2_plugs_msgs
{

  class StowPlugGoal : public ros::Msg
  {
    public:
      geometry_msgs::Pose gripper_to_plug_grasp;
      geometry_msgs::Pose base_to_plug;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->gripper_to_plug_grasp.serialize(outbuffer + offset);
      offset += this->base_to_plug.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->gripper_to_plug_grasp.deserialize(inbuffer + offset);
      offset += this->base_to_plug.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/StowPlugGoal"; };
    const char * getMD5(){ return "3eedcb5ac9e85b1272198d24562908f0"; };

  };

}
#endif