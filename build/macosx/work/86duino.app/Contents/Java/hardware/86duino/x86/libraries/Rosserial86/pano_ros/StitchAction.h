#ifndef _ROS_pano_ros_StitchAction_h
#define _ROS_pano_ros_StitchAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pano_ros/StitchActionGoal.h"
#include "pano_ros/StitchActionResult.h"
#include "pano_ros/StitchActionFeedback.h"

namespace pano_ros
{

  class StitchAction : public ros::Msg
  {
    public:
      pano_ros::StitchActionGoal action_goal;
      pano_ros::StitchActionResult action_result;
      pano_ros::StitchActionFeedback action_feedback;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->action_goal.serialize(outbuffer + offset);
      offset += this->action_result.serialize(outbuffer + offset);
      offset += this->action_feedback.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->action_goal.deserialize(inbuffer + offset);
      offset += this->action_result.deserialize(inbuffer + offset);
      offset += this->action_feedback.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pano_ros/StitchAction"; };
    const char * getMD5(){ return "ab1d70bbe0a37cea2298db1374a78cfa"; };

  };

}
#endif