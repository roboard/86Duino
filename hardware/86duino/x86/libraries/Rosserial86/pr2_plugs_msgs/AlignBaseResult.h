#ifndef _ROS_pr2_plugs_msgs_AlignBaseResult_h
#define _ROS_pr2_plugs_msgs_AlignBaseResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/PoseStamped.h"
#include "geometry_msgs/Vector3Stamped.h"

namespace pr2_plugs_msgs
{

  class AlignBaseResult : public ros::Msg
  {
    public:
      geometry_msgs::PoseStamped base_pose;
      geometry_msgs::Vector3Stamped wall_norm;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->base_pose.serialize(outbuffer + offset);
      offset += this->wall_norm.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->base_pose.deserialize(inbuffer + offset);
      offset += this->wall_norm.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/AlignBaseResult"; };
    const char * getMD5(){ return "dda025920ad5961dd3ca17ca87ff6014"; };

  };

}
#endif