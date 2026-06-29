#ifndef _ROS_moveit_simple_grasps_GenerateGraspsActionFeedback_h
#define _ROS_moveit_simple_grasps_GenerateGraspsActionFeedback_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "moveit_simple_grasps/GenerateGraspsFeedback.h"

namespace moveit_simple_grasps
{

  class GenerateGraspsActionFeedback : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalStatus status;
      moveit_simple_grasps::GenerateGraspsFeedback feedback;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->status.serialize(outbuffer + offset);
      offset += this->feedback.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->status.deserialize(inbuffer + offset);
      offset += this->feedback.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "moveit_simple_grasps/GenerateGraspsActionFeedback"; };
    const char * getMD5(){ return "947765100fcb339c2c35349f12eee057"; };

  };

}
#endif