#ifndef _ROS_manipulation_msgs_GraspPlanningActionResult_h
#define _ROS_manipulation_msgs_GraspPlanningActionResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "manipulation_msgs/GraspPlanningResult.h"

namespace manipulation_msgs
{

  class GraspPlanningActionResult : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalStatus status;
      manipulation_msgs::GraspPlanningResult result;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->status.serialize(outbuffer + offset);
      offset += this->result.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->status.deserialize(inbuffer + offset);
      offset += this->result.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "manipulation_msgs/GraspPlanningActionResult"; };
    const char * getMD5(){ return "3e6fbb82747590d8df5c4c99a3c657e8"; };

  };

}
#endif