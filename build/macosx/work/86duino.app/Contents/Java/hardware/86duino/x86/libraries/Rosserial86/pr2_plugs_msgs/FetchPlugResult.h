#ifndef _ROS_pr2_plugs_msgs_FetchPlugResult_h
#define _ROS_pr2_plugs_msgs_FetchPlugResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Pose.h"

namespace pr2_plugs_msgs
{

  class FetchPlugResult : public ros::Msg
  {
    public:
      geometry_msgs::Pose plug_on_base_pose;
      geometry_msgs::Pose gripper_plug_grasp_pose;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->plug_on_base_pose.serialize(outbuffer + offset);
      offset += this->gripper_plug_grasp_pose.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->plug_on_base_pose.deserialize(inbuffer + offset);
      offset += this->gripper_plug_grasp_pose.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/FetchPlugResult"; };
    const char * getMD5(){ return "846515236bd59b28c0b365f64b50e7c7"; };

  };

}
#endif