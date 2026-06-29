#ifndef _ROS_pano_ros_PanoCaptureActionGoal_h
#define _ROS_pano_ros_PanoCaptureActionGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalID.h"
#include "pano_ros/PanoCaptureGoal.h"

namespace pano_ros
{

  class PanoCaptureActionGoal : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalID goal_id;
      pano_ros::PanoCaptureGoal goal;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->goal_id.serialize(outbuffer + offset);
      offset += this->goal.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->goal_id.deserialize(inbuffer + offset);
      offset += this->goal.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pano_ros/PanoCaptureActionGoal"; };
    const char * getMD5(){ return "89c6aeb4dc23614f0f2fdd00ff6a29c8"; };

  };

}
#endif