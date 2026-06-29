#ifndef _ROS_manipulation_msgs_ClusterBoundingBox_h
#define _ROS_manipulation_msgs_ClusterBoundingBox_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/PoseStamped.h"
#include "geometry_msgs/Vector3.h"

namespace manipulation_msgs
{

  class ClusterBoundingBox : public ros::Msg
  {
    public:
      geometry_msgs::PoseStamped pose_stamped;
      geometry_msgs::Vector3 dimensions;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->pose_stamped.serialize(outbuffer + offset);
      offset += this->dimensions.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->pose_stamped.deserialize(inbuffer + offset);
      offset += this->dimensions.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "manipulation_msgs/ClusterBoundingBox"; };
    const char * getMD5(){ return "9bf2b7a44ad666dc3a6a2bbc21782dad"; };

  };

}
#endif